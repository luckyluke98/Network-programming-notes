# Network Programming Notes

Out of personal interest, I wanted to deepen my understanding of network programming in C. I've always been fascinated by the low-level control C offers, so I decided to explore a topic I had previously only touched on superficially.

To study, I followed the fantastic Beej’s guides ([this one in particular](https://beej.us/guide/bgnet/)).

As a small project, I created a simple multi-user chat. The server binds a socket to a port and listens for incoming connections. The client opens a socket and connects to the server on that port.

For each accepted connection, the server manages the socket file descriptors using an array of `struct pollfd`.

You’ll find everything in the source code.

> **Disclaimer**  
> Comments and notes in the code are written in a mix of Italian and English.  
> This is intentional and part of the personal learning workflow — they won’t be translated.

# TCP Chat Project

This is a simple client-server application written in C, designed to simulate a multi-user chat over TCP sockets. It’s useful during penetration testing or for practicing low-level networking concepts.

## Files

- `TCP_chat.c` – Implements the TCP multi-client server using `poll()` for concurrent management.
- `TCP_chat_client.c` – Implements an interactive terminal client using the `ncurses` library.

## Compile the files

You need `gcc`, and the `ncurses` and `pthread` libraries installed.

```bash
gcc -o TCP_chat TCP_chat.c
gcc -o TCP_chat_client TCP_chat_client.c -lncurses -lpthread
```

## Execution

### Start the server

```bash
./TCP_chat
```

### Start one or more clients (in separate terminals)

```bash
./TCP_chat_client <nickname>
```

Example:

```bash
./TCP_chat_client Alice
./TCP_chat_client Bob
```

## How it works

### Server (`TCP_chat.c`)

- Listens on port `3490` on `localhost`.
- Supports up to `MAX_FDS` simultaneous connections (default: 5).
- Uses `poll()` to handle multiple clients without threading.
- Each client sends their nickname as the first message.
- Messages are forwarded to all connected clients.
- When a client disconnects, others are notified.

### Client (`TCP_chat_client.c`)

- Requires a nickname as a command-line argument.
- Connects to the server and sends the nickname as the first message.
- Uses `ncurses` for the GUI.
- Creates a background thread to listen for messages from the server.
- Allows the user to write and send messages.
- Typing `/quit` closes the connection and exits the client.

## TODO / Future Improvements

- [ ] Server currently only accepts connections from `localhost`. Modify to allow external connections.
- [ ] **Basic encryption support (e.g., TLS or SSL)**
- [ ] When the connection limit is reached, new clients are immediately disconnected.
- [ ] **Unique nicknames**, with rejection or notification in case of duplicates.
- [ ] Allow more connections by reallocating the file descriptor array.

## Example Images

Server Logs:

![image](https://github.com/user-attachments/assets/3a9e9c8a-2e47-4e74-9449-115c77391ffb)

User perspective:

![image](https://github.com/user-attachments/assets/e09ec0de-bc93-441b-8d8b-eb9b17d84f0b)

---

Developed as a personal project for testing and educational purposes.
