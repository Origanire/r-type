# Running

Start one server, then one client per player (up to four).

## On one machine

```sh
./build/bin/r-type_server
./build/bin/r-type_client
```

Run each command in its own terminal. On Windows, use `out\bin\Release\r-type_server.exe` and
`out\bin\Release\r-type_client.exe`.

## On a local network

On the server machine, find its IPv4 address (`ip addr` on Linux, `ipconfig` on Windows) and allow
UDP port 4242 in the firewall. On each player machine:

```sh
./build/bin/r-type_client -h 192.168.1.42 -p 4242
```

Players on the Internet need the server's UDP port to be forwarded by its router.

## Command-line options

| Program | Option | Default | Meaning |
|---|---|---|---|
| server | `-p <port>` | 4242 | UDP port to listen on |
| client | `-h <IPv4>` | 127.0.0.1 | Server address (numeric IPv4 only) |
| client | `-p <port>` | 4242 | Server port |
| both | `--help` | — | Print usage |

Invalid arguments make the program exit with code 84.

## Controls

| Key | Action |
|---|---|
| Arrow keys | Move |
| Space | Fire |
| Escape | Quit the client |

## What to expect

- Each player has a ship of a different color, which leans when moving up or down.
- Bydos enter from the right, flying straight, in a sine wave or in a zigzag.
- A missile destroys a Bydo and gives 100 points to the shooter.
- Touching a Bydo costs a life; the ship respawns after 2 s and is invulnerable for 2 s.
- When no player has lives left, the server prints `Game over` and starts a new round 5 s later.
- `Ctrl+C` stops the server cleanly; connected clients are notified and exit.
