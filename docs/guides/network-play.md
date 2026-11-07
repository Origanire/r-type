# Playing on a local network

## 1. Start the server

On the machine that hosts the game:

```sh
./r-type_server -p 4242
```

It prints the address to give to the other players:

```text
Server listening on UDP port 4242
Players on your network can join with: -h 192.168.1.67 -p 4242
```

If the line is missing (no network found), find the address yourself: `ipconfig` on Windows (IPv4 address),
`ip addr` on Linux.

## 2. Start the clients

On every player's machine, including the host if it plays:

```sh
./r-type_client -h 192.168.1.67 -p 4242
```

`-p` can be left out when the server uses the default port 4242. Without `-h`, the client uses
`serverAddress` from `rtype_settings.json`.

## 3. Firewall

The server receives UDP on its port. If clients stay on "CONNECTING...":

| System | What to do |
|---|---|
| Windows | Accept the firewall popup the first time the server starts, for **private networks**. Otherwise: Windows Defender Firewall, Allow an app, add `r-type_server.exe` |
| Linux (ufw) | `sudo ufw allow 4242/udp` |
| School or public Wi-Fi | Client isolation may block traffic between machines: use a phone hotspot or a cable switch |

## 4. WSL

A server started inside WSL 2 is behind a virtual network by default. Either run the Windows build of the
server, or enable mirrored networking (`networkingMode=mirrored` in `%UserProfile%\.wslconfig`) so that WSL uses
the Windows address.

## Checks

| Symptom | Cause |
|---|---|
| "CONNECTING..." then "SERVER UNREACHABLE" | Wrong address, firewall, or server not started |
| "SERVER UNREACHABLE OR CONNECTION LOST" right away, server running | Client and server built from different versions (the server answers `REJECT` version mismatch): rebuild both from the same commit |
| Lag spikes | Wi-Fi interference: prefer a cable for the host |
| Room list empty on one machine only | It is connected to another server (check `-h`) |
