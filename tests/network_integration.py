"""Exercise the real server and C++ client over loopback, with an independent codec."""
import signal
import socket
import struct
import subprocess
import sys
import time


def packet(kind, sequence=0, payload=b'', version=1):
    return struct.pack('!HBBIH', 0x5254, version, kind, sequence, len(payload)) + payload


def receive(peer, kind, timeout=2):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        peer.settimeout(max(.001, deadline - time.monotonic()))
        data = peer.recv(2048)
        magic, version, message, sequence, size = struct.unpack('!HBBIH', data[:10])
        assert magic == 0x5254 and version == 1 and size == len(data) - 10
        if message == kind:
            return data[10:]
    raise AssertionError(f'Missing message {kind:x}')


def snapshot(peer):
    data = receive(peer, 0x83)
    count = data[37]
    assert len(data) == 38 + count * 7
    entities = [struct.unpack_from('!HBhh', data, 38 + i * 7) for i in range(count)]
    return data, entities


def make_peer(port):
    peer = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    peer.connect(('127.0.0.1', port))
    return peer


def main():
    reservation = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    reservation.bind(('127.0.0.1', 0))
    port = reservation.getsockname()[1]
    reservation.close()
    process = subprocess.Popen([sys.argv[1], '-p', str(port)], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    peers = []
    try:
        assert 'listening' in process.stdout.readline(), 'Server failed to start'
        a = make_peer(port); peers.append(a)
        a.send(packet(1))
        accepted = receive(a, 0x81)
        token, slot = struct.unpack('!IB', accepted)
        assert token and slot == 0
        a.settimeout(.15)
        try:
            a.recv(2048)
            raise AssertionError('Unconfirmed endpoint received a snapshot')
        except socket.timeout:
            pass
        time.sleep(.06)
        a.send(packet(1, 1))
        assert receive(a, 0x81) == accepted, 'CONNECT retry changed session'
        a.send(packet(2, 2, struct.pack('!IB', token, 0)))
        first, entities = snapshot(a)
        ship = entities[0][0]
        assert entities[0][2] == 200
        b = make_peer(port); peers.append(b)
        b.send(packet(1))
        token_b, slot_b = struct.unpack('!IB', receive(b, 0x81))
        assert slot_b == 1 and token_b != token
        b.send(packet(2, 1, struct.pack('!IB', token_b, 0)))
        deadline = time.monotonic() + 2
        while time.monotonic() < deadline:
            _, entities = snapshot(a)
            if len(entities) == 2: break
        assert len(entities) == 2, 'Second client not replicated'
        a.send(packet(2, 10, struct.pack('!IB', token, 8)))
        a.send(packet(2, 9, struct.pack('!IB', token, 4)))  # stale input must not win
        deadline = time.monotonic() + 1
        while time.monotonic() < deadline:
            _, entities = snapshot(b)
            if next(e[2] for e in entities if e[0] == ship) >= 240: break
        assert next(e[2] for e in entities if e[0] == ship) >= 240
        time.sleep(.65)
        _, entities = snapshot(a)
        # Drain snapshots queued during the wait, then compare fresh positions.
        while True:
            a.settimeout(.01)
            try: a.recv(2048)
            except socket.timeout: break
        _, entities = snapshot(a)
        stopped_x = next(e[2] for e in entities if e[0] == ship)
        a.send(packet(2, 100, struct.pack('!IB', token ^ 0xFFFFFFFF, 8)))
        b.send(packet(2, 100, struct.pack('!IB', token, 8)))  # right token, wrong endpoint
        for bad in [b'bad', packet(2, 101, struct.pack('!IB', token, 0x80)), b'x' * 1300,
                    packet(0x83), packet(0x7F), packet(2, 102, struct.pack('!IB', token, 8))[:-1]]:
            a.send(bad)
        time.sleep(.1)
        _, entities = snapshot(a)
        assert next(e[2] for e in entities if e[0] == ship) == stopped_x, 'Invalid packet changed position'
        a.send(packet(3, 11, struct.pack('!II', token, 123456)))
        assert receive(a, 0x84) == struct.pack('!I', 123456)
        wrong_version = make_peer(port); peers.append(wrong_version)
        wrong_version.send(packet(1, version=2))
        assert receive(wrong_version, 0x82) == b'\x02'
        for _ in range(2):
            peer = make_peer(port); peers.append(peer); peer.send(packet(1))
            t, _ = struct.unpack('!IB', receive(peer, 0x81))
            peer.send(packet(2, 1, struct.pack('!IB', t, 0)))
        full = make_peer(port); peers.append(full); full.send(packet(1))
        assert receive(full, 0x82) == b'\x01'
        b.send(packet(4, 2, struct.pack('!I', token_b)))
        assert receive(a, 0x88) == bytes([slot_b, 1])
        # Keep one client alive; the other two must expire after 5 seconds.
        deadline = time.monotonic() + 5.5
        sequence = 20
        while time.monotonic() < deadline:
            a.send(packet(3, sequence, struct.pack('!II', token, sequence)))
            sequence += 1
            time.sleep(.2)
        while True:
            a.settimeout(.005)
            try: a.recv(2048)
            except socket.timeout: break
        _, entities = snapshot(a)
        assert len(entities) == 1, 'Timed-out clients still present'
        a.send(packet(4, sequence, struct.pack('!I', token)))
        time.sleep(.1)
        result = subprocess.run([sys.argv[2], str(port)], capture_output=True, text=True, timeout=5)
        assert result.returncode == 0, result.stdout + result.stderr
        print('UDP integration passed: sessions, replication, movement, stale/invalid input, heartbeat, capacity, disconnect, timeout, C++ NetworkClient')
        process.send_signal(signal.SIGINT)
        assert process.wait(timeout=3) == 0
    finally:
        for peer in peers: peer.close()
        if process.poll() is None:
            process.terminate()
            try: process.wait(timeout=3)
            except subprocess.TimeoutExpired: process.kill(); process.wait()
        if process.returncode not in (0, None):
            print(process.stdout.read(), process.stderr.read(), file=sys.stderr)


if __name__ == '__main__':
    main()
