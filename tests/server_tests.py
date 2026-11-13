"""채팅 서버 시나리오 테스트. CTest가 시나리오마다 따로 부름.

사용법: server_tests.py --server <server 실행 파일> --base-config <server.json>
    [--loadtest <loadtest 실행 파일>] [--client <client 실행 파일> --client-config <client.json>] <시나리오>

시나리오마다 기본 설정에 바꿀 값만 덮어 임시 설정 파일을 만들고, 빈 포트로 서버를 띄움.
끝나면 서버를 정상 종료시키고(SIGINT, Windows는 강제 종료) 종료 코드를 확인함.
실패하면 종료 코드 1과 함께 이유와 서버 출력을 남김.
"""
import argparse
import copy
import json
import os
import signal
import socket
import struct
import subprocess
import sys
import tempfile
import threading
import time

CHAT, CHAT_NOTIFY, SET_NICKNAME, SET_NICKNAME_RESULT = 1, 2, 3, 4
CONTROL_PACKET_ID, PING, CLIENT_HELLO, DISCONNECT, LOGOUT = 0, 2, 8, 9, 10
ENGINE_PROTOCOL_VERSION, APP_PROTOCOL_VERSION = 1, 1
# 엔진 DisconnectReason
REASON_SERVER_SHUTDOWN, REASON_LOGIN_TIMEOUT, REASON_VERSION_MISMATCH, REASON_SERVER_FULL = 1, 3, 4, 6


class TestFailure(Exception):
    pass


def check(condition, message):
    if not condition:
        raise TestFailure(message)


def wait_until(predicate, timeout, message):
    deadline = time.time() + timeout
    while time.time() < deadline:
        if predicate():
            return
        time.sleep(0.02)
    raise TestFailure(message)


def string_field(text):
    body = text.encode()
    return struct.pack("<H", len(body)) + body


def client_hello(engine_version=ENGINE_PROTOCOL_VERSION, app_version=APP_PROTOCOL_VERSION):
    return packet(CONTROL_PACKET_ID, struct.pack("<HHH", CLIENT_HELLO, engine_version, app_version))


def packet(packet_id, body=b""):
    return struct.pack("<HH", 4 + len(body), packet_id) + body


class Client:
    """채팅 프로토콜만 아는 테스트용 클라이언트. Ping은 보내지 않음."""

    def __init__(self, port, name=None, hello=True, close_on_eof=True):
        self.sock = socket.create_connection(("127.0.0.1", port))
        self.notifies = []
        self.logged_in = False
        self.closed_reason = None
        self.disconnect_reason = None
        # 실제 클라이언트처럼 서버가 끊으면 소켓을 닫음. False면 닫지 않고 버팀
        self.close_on_eof = close_on_eof
        # 서버가 바로 끊는 경우 읽기 스레드가 소켓을 먼저 닫을 수 있어 hello는 읽기 전에 보냄
        if hello:
            try:
                self.send(client_hello())
            except OSError:
                pass
        threading.Thread(target=self._read, daemon=True).start()
        if name:
            self.send(packet(SET_NICKNAME, string_field(name)))
            wait_until(lambda: self.logged_in, 3, f"{name} did not log in")

    def send(self, data):
        self.sock.sendall(data)

    def chat(self, text):
        self.send(packet(CHAT, string_field(text)))

    def messages_from(self, sender):
        return [message for who, message in self.notifies if who == sender]

    def close(self):
        # 읽기 스레드가 recv에서 기다리는 소켓은 close()만으로는 연결이 끊기지 않을 수 있어 먼저 shutdown함
        try:
            self.sock.shutdown(socket.SHUT_RDWR)
        except OSError:
            pass
        self.sock.close()

    def _read(self):
        buffer = b""
        try:
            while True:
                data = self.sock.recv(65536)
                if not data:
                    self.closed_reason = "closed"
                    if self.close_on_eof:
                        self.sock.close()
                    return
                buffer += data
                while len(buffer) >= 4:
                    size, packet_id = struct.unpack_from("<HH", buffer)
                    if len(buffer) < size:
                        break
                    body, buffer = buffer[4:size], buffer[size:]
                    self._handle(packet_id, body)
        except OSError as error:
            self.closed_reason = type(error).__name__

    def _handle(self, packet_id, body):
        if packet_id == CONTROL_PACKET_ID:
            control_type = struct.unpack_from("<H", body)[0]
            if control_type == DISCONNECT:
                self.disconnect_reason = struct.unpack_from("<H", body, 2)[0]
        elif packet_id == SET_NICKNAME_RESULT:
            self.logged_in = struct.unpack_from("<H", body)[0] == 0
        elif packet_id == CHAT_NOTIFY:
            length = struct.unpack_from("<H", body)[0]
            sender = body[2:2 + length].decode()
            message_length = struct.unpack_from("<H", body, 2 + length)[0]
            message = body[4 + length:4 + length + message_length].decode()
            self.notifies.append((sender, message))


class TcpProxy:
    """클라이언트와 서버 사이에서 바이트를 그대로 넘김. drop_all()로 연결을 끊어 네트워크 단절을 흉내 냄."""

    def __init__(self, server_port):
        self.server_port = server_port
        self.listener = socket.create_server(("127.0.0.1", 0))
        self.port = self.listener.getsockname()[1]
        self.sockets = []
        self.lock = threading.Lock()
        threading.Thread(target=self._accept, daemon=True).start()

    def __enter__(self):
        return self

    def __exit__(self, *exc):
        self.drop_all()
        self.listener.close()

    def drop_all(self):
        with self.lock:
            sockets, self.sockets = self.sockets, []
        for sock in sockets:
            try:
                sock.shutdown(socket.SHUT_RDWR)
            except OSError:
                pass
            sock.close()

    def _accept(self):
        while True:
            try:
                downstream, _ = self.listener.accept()
                upstream = socket.create_connection(("127.0.0.1", self.server_port))
            except OSError:
                return
            with self.lock:
                self.sockets += [downstream, upstream]
            threading.Thread(target=self._pump, args=(downstream, upstream), daemon=True).start()
            threading.Thread(target=self._pump, args=(upstream, downstream), daemon=True).start()

    @staticmethod
    def _pump(source, target):
        try:
            while data := source.recv(65536):
                target.sendall(data)
        except OSError:
            pass
        for sock in (source, target):
            try:
                sock.shutdown(socket.SHUT_RDWR)
            except OSError:
                pass


class ChatClientProcess:
    """client 실행 파일을 띄워 표준 입력으로 조작하고 표준 출력을 모음."""

    def __init__(self, context, port):
        with open(context.client_config, encoding="utf-8") as file:
            config = merge(json.load(file), {"engine": {"port": port, "reconnectMaxDelaySeconds": 1}})
        self.config_path = os.path.join(context.directory, "client.json")
        with open(self.config_path, "w", encoding="utf-8") as file:
            json.dump(config, file)
        self.process = subprocess.Popen([context.client, self.config_path], stdin=subprocess.PIPE,
                                        stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        self.output = ""
        threading.Thread(target=self._read, daemon=True).start()

    def _read(self):
        for line in self.process.stdout:
            self.output += line

    def type(self, line):
        self.process.stdin.write(line + "\n")
        self.process.stdin.flush()

    def wait_for(self, text, timeout=3):
        wait_until(lambda: text in self.output, timeout, f"client did not print '{text}':\n{self.output}")

    def wait_exit(self, timeout=10):
        return self.process.wait(timeout=timeout)

    def kill(self):
        if self.process.poll() is None:
            self.process.kill()
            self.process.wait()


# 시나리오: (설정에 덮어쓸 값, 실행 함수)

def chat(port, context):
    alice = Client(port, "alice")
    bob = Client(port, "bob")
    wait_until(lambda: ("Server", "bob joined") in alice.notifies, 2, "alice did not see bob join")

    alice.chat("hello from alice")
    bob.chat("hi alice")
    for client in (alice, bob):
        wait_until(lambda: client.messages_from("alice") == ["hello from alice"]
                   and client.messages_from("bob") == ["hi alice"], 2, "chat was not broadcast to everyone")


def chat_limit(port, context):
    observer = Client(port, "observer")
    talker = Client(port, "talker")

    # 3초에 5개. 한 번에 10개를 보내면 5개만 퍼지고 나머지는 본인에게 경고
    talker.send(b"".join(packet(CHAT, string_field(f"burst {i}")) for i in range(10)))
    wait_until(lambda: len(talker.messages_from("Server")) >= 5, 2, "talker was not warned")
    time.sleep(0.3)
    check(len(observer.messages_from("talker")) == 5,
          f"expected 5 chats to pass, got {len(observer.messages_from('talker'))}")

    time.sleep(3.2)
    before = len(observer.messages_from("talker"))
    talker.send(b"".join(packet(CHAT, string_field(f"later {i}")) for i in range(5)))
    wait_until(lambda: len(observer.messages_from("talker")) - before == 5, 2, "tokens did not refill")


def receive_limit(port, context):
    observer = Client(port, "observer")
    flooder = Client(port)
    ping = packet(CONTROL_PACKET_ID, struct.pack("<H", PING))
    try:
        flooder.send(ping * 300)
    except OSError:
        pass
    wait_until(lambda: flooder.closed_reason is not None, 2, "flooder was not disconnected")
    time.sleep(0.2)
    check(observer.closed_reason is None, "observer was disconnected too")


def heartbeat(port, context):
    silent = Client(port, "silent")
    start = time.time()
    wait_until(lambda: silent.closed_reason is not None, 6, "silent client was not disconnected")
    elapsed = time.time() - start
    check(2.0 <= elapsed <= 5.0, f"disconnected after {elapsed:.1f}s, expected about 3s")


def send_queue_limit(port, context):
    sleeper_socket = socket.create_connection(("127.0.0.1", port))
    sleeper_socket.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 4096)
    sleeper_socket.sendall(client_hello() + packet(SET_NICKNAME, string_field("sleeper")))
    spammer = Client(port, "spammer")

    message = packet(CHAT, string_field("x" * 1000)) * 50
    deadline = time.time() + 10
    reset = False
    while time.time() < deadline and not reset:
        spammer.send(message)
        time.sleep(0.01)
        reset = "Send queue over" in context.server_output()
    check(reset, "server did not reset the client that stopped reading")
    check(spammer.closed_reason is None, "spammer was disconnected too")
    sleeper_socket.close()


def shutdown(port, context):
    # 재접속을 기다리는 세션 둘과 연결된 사용자 하나가 남은 상태에서 정상 종료
    for name in ("first", "second"):
        Client(port, name).close()
    wait_until(lambda: context.server_output().count("waiting for reconnect") == 2, 2, "sessions were not detached")
    online = Client(port, "online")
    guest = Client(port)

    if not context.can_signal():
        return
    elapsed, code = context.stop_server()
    check(code == 0, f"server exited with {code}")
    check(elapsed <= 2, f"server took {elapsed:.1f}s to shut down")
    for client in (online, guest):
        check(client.disconnect_reason == REASON_SERVER_SHUTDOWN,
              f"client was not told about the shutdown (reason {client.disconnect_reason})")


def force_shutdown(port, context):
    # 상대가 끊어 주지 않아 정상 종료가 closeTimeout(5초)까지 걸리는 상황에서 두 번째 신호로 바로 끝냄
    if not context.can_signal():
        return
    stubborn = Client(port, "stubborn", close_on_eof=False)
    context.server.process.send_signal(signal.SIGINT)
    wait_until(lambda: stubborn.disconnect_reason == REASON_SERVER_SHUTDOWN, 2, "first signal did not start shutdown")
    check(context.server.process.poll() is None, "server stopped before the peer closed")
    start = time.time()
    elapsed, code = context.stop_server()
    check(code == 0, f"server exited with {code}")
    check(time.time() - start <= 1.5, "second signal did not stop the server at once")


def login_timeout(port, context):
    idle = Client(port)
    start = time.time()
    wait_until(lambda: idle.closed_reason is not None, 4, "idle connection was not closed")
    check(idle.disconnect_reason == REASON_LOGIN_TIMEOUT, f"unexpected reason {idle.disconnect_reason}")
    elapsed = time.time() - start
    check(0.5 <= elapsed <= 3, f"closed after {elapsed:.1f}s, expected about 1s")
    logged_in = Client(port, "member")
    time.sleep(1.5)
    check(logged_in.closed_reason is None, "logged-in client was closed by the login timeout")


def max_connections(port, context):
    first = Client(port, "first")
    second = Client(port, "second")
    third = Client(port)
    wait_until(lambda: third.closed_reason is not None, 2, "connection over the limit was accepted")
    check(third.disconnect_reason == REASON_SERVER_FULL, f"unexpected reason {third.disconnect_reason}")
    check(first.closed_reason is None and second.closed_reason is None, "existing clients were closed")
    wait_until(lambda: "refused" in context.server_output(), 2, "refusal was not logged")


def version_mismatch(port, context):
    old = Client(port, hello=False)
    old.send(client_hello(app_version=APP_PROTOCOL_VERSION + 1))
    wait_until(lambda: old.closed_reason is not None, 2, "client with another version was not closed")
    check(old.disconnect_reason == REASON_VERSION_MISMATCH, f"unexpected reason {old.disconnect_reason}")

    # ClientHello 없이 보낸 응용 패킷은 받지 않음
    rude = Client(port, hello=False)
    rude.send(packet(SET_NICKNAME, string_field("rude")))
    wait_until(lambda: rude.closed_reason is not None, 2, "packet before ClientHello was accepted")


def logout(port, context):
    observer = Client(port, "observer")
    leaver = Client(port, "leaver")
    leaver.send(packet(CONTROL_PACKET_ID, struct.pack("<H", LOGOUT)))
    # 재접속 대기(30초) 없이 바로 나감
    wait_until(lambda: ("Server", "leaver left") in observer.notifies, 1, "logout did not end the session at once")


def reconnect(port, context):
    # 실제 클라이언트를 프록시 너머로 붙이고, 프록시가 연결을 끊었을 때 세션을 이어 가는지 확인
    check(context.client is not None, "--client is required")
    observer = Client(port, "observer")
    with TcpProxy(port) as proxy:
        chat_client = ChatClientProcess(context, proxy.port)
        try:
            chat_client.type("alice")
            chat_client.wait_for("Entered the chat as alice")

            proxy.drop_all()
            chat_client.wait_for("reconnecting")
            chat_client.wait_for("Reconnected", timeout=5)
            wait_until(lambda: "alice reconnected" in context.server_output(), 2, "server did not resume the session")

            chat_client.type("back again")
            wait_until(lambda: observer.messages_from("alice") == ["back again"], 2, "chat after reconnect was lost")

            # 빈 줄로 나가면 Logout을 보내 재접속 대기 없이 바로 "left"가 나감
            chat_client.type("")
            wait_until(lambda: ("Server", "alice left") in observer.notifies, 2, "quitting did not log out")
            check(chat_client.wait_exit() == 0, "client exited with an error")
        finally:
            chat_client.kill()


def load_test(port, context):
    check(context.loadtest is not None, "--loadtest is required")
    result = subprocess.run([context.loadtest, "--port", str(port), "--clients", "50", "--messages", "20",
                             "--interval-ms", "50", "--timeout-s", "30"], capture_output=True, text=True)
    check(result.returncode == 0, "load test failed:\n" + result.stdout + result.stderr)


LOAD_LIMITS = {"engine": {"receiveLimit": {"packetsPerSecond": 100000}},
               "app": {"chatLimit": {"messages": 100000, "perSeconds": 1}}}

SCENARIOS = {
    "chat": ({}, chat),
    "chat_limit": ({}, chat_limit),
    "receive_limit": ({}, receive_limit),
    "heartbeat": ({"engine": {"heartbeat": {"interval": 1, "timeout": 3}}}, heartbeat),
    "send_queue_limit": (LOAD_LIMITS, send_queue_limit),
    "shutdown": ({}, shutdown),
    "force_shutdown": ({}, force_shutdown),
    "login_timeout": ({"engine": {"loginTimeoutSeconds": 1}}, login_timeout),
    "max_connections": ({"engine": {"maxConnections": 2}}, max_connections),
    "version_mismatch": ({}, version_mismatch),
    "logout": ({}, logout),
    "reconnect": ({}, reconnect),
    "load": (LOAD_LIMITS, load_test),
}


def merge(base, overrides):
    result = copy.deepcopy(base)
    for key, value in overrides.items():
        if isinstance(value, dict) and isinstance(result.get(key), dict):
            result[key] = merge(result[key], value)
        else:
            result[key] = value
    return result


def free_port():
    with socket.socket() as probe:
        probe.bind(("127.0.0.1", 0))
        return probe.getsockname()[1]


class ServerProcess:
    def __init__(self, server, config):
        self.output_file = tempfile.TemporaryFile(mode="w+")
        flags = subprocess.CREATE_NEW_PROCESS_GROUP if os.name == "nt" else 0
        self.process = subprocess.Popen([server, config], stdout=self.output_file, stderr=subprocess.STDOUT,
                                        creationflags=flags)

    def output(self):
        self.output_file.flush()
        self.output_file.seek(0)
        return self.output_file.read()

    def stop(self):
        """정상 종료시키고 걸린 시간과 종료 코드를 반환. 이미 끝났으면 그 종료 코드를 반환
        Windows는 신호를 보낼 수 없어 강제 종료하고 (None, None)을 반환함"""
        if self.process.poll() is not None:
            return 0.0, self.process.returncode
        start = time.time()
        if os.name == "nt":
            self.process.kill()
            self.process.wait()
            return None, None
        self.process.send_signal(signal.SIGINT)
        try:
            code = self.process.wait(timeout=10)
        except subprocess.TimeoutExpired:
            self.process.kill()
            self.process.wait()
            return time.time() - start, None
        return time.time() - start, code


class Context:
    def __init__(self, server, args, directory):
        self.server = server
        self.loadtest = args.loadtest
        self.client = args.client
        self.client_config = args.client_config
        self.directory = directory

    def server_output(self):
        return self.server.output()

    @staticmethod
    def can_signal():
        """정상 종료 신호를 보낼 수 있는지. Windows에서는 신호로 확인하는 부분을 건너뜀"""
        return os.name != "nt"

    def stop_server(self):
        return self.server.stop()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--server", required=True)
    parser.add_argument("--base-config", required=True)
    parser.add_argument("--loadtest")
    parser.add_argument("--client")
    parser.add_argument("--client-config")
    parser.add_argument("scenario", choices=sorted(SCENARIOS))
    args = parser.parse_args()

    overrides, run = SCENARIOS[args.scenario]
    with open(args.base_config, encoding="utf-8") as file:
        config = merge(json.load(file), overrides)
    port = free_port()
    config["engine"]["port"] = port

    with tempfile.TemporaryDirectory() as directory:
        config_path = os.path.join(directory, "server.json")
        with open(config_path, "w", encoding="utf-8") as file:
            json.dump(config, file)

        server = ServerProcess(args.server, config_path)
        context = Context(server, args, directory)
        failure = None
        try:
            wait_until(lambda: "listening" in server.output() or server.process.poll() is not None, 5,
                       "server did not start")
            check(server.process.poll() is None, "server exited at startup")
            run(port, context)
        except (TestFailure, OSError, subprocess.TimeoutExpired) as error:
            failure = str(error)

        # 시나리오가 서버를 이미 멈췄으면 그 결과를 그대로 씀
        _, code = server.stop()
        if failure is None and code is not None and code != 0:
            failure = f"server exited with {code}"

        if failure:
            print(f"FAIL {args.scenario}: {failure}")
            print("----- server output -----")
            print(server.output())
            return 1
        print(f"PASS {args.scenario}")
        return 0


if __name__ == "__main__":
    sys.exit(main())
