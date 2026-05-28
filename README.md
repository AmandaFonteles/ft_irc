*This project has been created as part of the 42 curriculum by afontele, aibonade, ndarouec.*

# ft_irc

## Description

`ft_irc` is a simple IRC server written in C++98. It allows multiple IRC clients to connect, authenticate, set nicknames and usernames, join channels, and exchange public and private messages.

The main challenge of the project is to handle multiple clients simultaneously without forking and without blocking I/O. To achieve this, the server is built around a single event loop using `poll()`, which monitors all sockets (the listening socket and every connected client) at once. The loop runs until a quit signal is received.

The server implements user authentication via a password, channel management, and the operator-specific commands required by the subject (`KICK`, `INVITE`, `TOPIC`, and `MODE` with the `i`, `t`, `k`, `o`, and `l` flags).

## Instructions

### Compilation

A standard `Makefile` is provided at the root of the repository. To build the server:

```
make
```

The `Makefile` provides the rules required by the subject: `all`, `clean`, `fclean`, and `re`, all declared as `.PHONY`. Compilation uses `c++` with the flags `-Wall -Wextra -Werror -std=c++98`.

### Running the server

```
./ircserv <port> <password>
```

- **port**: the TCP port the server will listen on for incoming IRC connections.
- **password**: the connection password. Every client must send it (via the `PASS` command) before it can register and use the server.

### Connecting with a client

Our reference client is **irssi**. Once the server is running, you can connect with:

```
irssi
/connect <server_ip> <port> <password>
```

From there you can register a nickname, join channels, and send messages as you would on any IRC server.

## Features

The server supports the following commands:

- **Registration**: `PASS`, `NICK`, `USER`
- **Messaging**: `PRIVMSG` (to users and channels)
- **Channels**: `JOIN`, `PART`, `QUIT`
- **Channel operators**:
  - `KICK` — eject a client from a channel
  - `INVITE` — invite a client to a channel
  - `TOPIC` — view or change the channel topic
  - `MODE` — change channel modes:
    - `i` — invite-only channel
    - `t` — restrict `TOPIC` to channel operators
    - `k` — channel key (password)
    - `o` — give/take operator privilege
    - `l` — set/remove user limit

Partial data is handled correctly: the server buffers incoming bytes per client and only processes a command once a full line (terminated by `\r\n`) has been received.

## Resources

Classic references we used while working on the project:

- The C library manual pages (`man 2 socket`, `man 2 poll`, `man 2 recv`, etc.) for the system calls allowed by the subject.
- **Beej's Guide to Network Programming** — https://beej.us/guide/bgnet/html/ — for the overall network architecture and socket programming patterns.
- The IRC protocol RFCs (**RFC 1459**, **RFC 2810**, **RFC 2811** and **RFC 2812**) for command syntax, numeric replies, and expected server behavior.
- The **Modern IRC documentation** — https://modern.ircdocs.horse/ — as a more readable complement to the RFCs.

### How AI was used

We used AI in a limited, supporting role. Specifically:

- **Planning and task division**: to draft a project plan and help us split the work between team members.
- **Understanding concepts**: to clarify parts of the IRC protocol and networking concepts we were less familiar with.
- **Debugging**: to help interpret unexpected behavior and narrow down the cause of bugs.
- **Testing**: to complement our own tests with additional cases and edge scenarios.
