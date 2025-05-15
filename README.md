## Chat Rooms | CSC-284 Final Project

### Introduction
---
This project consists of both a Server and Client application. Both applications when ran from the command line will take IP and Port arguments or can be ran without arguments which will allow for IP and Port to be given during run-time. The server-side application will allow many client connections concurrently. When a client connects they are prompted for a username which will be unique to that client until they disconnect. Clients have the opportunity to create rooms within the server which will allow chat to only be shared between other clients within the same room.

![Example of the client application](https://i.imgur.com/917d4UB.png)

### Features
---
- Client Textual UI
- Multithreaded Server & Client
- Networking
- Multi-room Chat

### Dependencies
---
- Windows
- C++17 combatible compiler
- [Ncurses](https://invisible-island.net/ncurses/)

### Client Usage
---
When starting the client an IP and Port may be given as command line arguments or during run-time in the terminal, these arguments will determine where to connect to the server.

Upon connection, the user is prompted for a username which will identify them to every other client on the server when they send messages. Clients also have access to a number of commands.

#### Client Commands
- `/help` : Lists all commands
- `/create_room <name>` : Creates a new room and moves the client to that room
- `/join_room <name>` : Moves the client to the given room as long as it exists
- `/list_users <room = Lobby>` : Returns a list of users in a given room or all users in the server if a room is not provided.
- `/exit` : Leaves the server intentionally.

When using a command it must be preceeded by a '/' else it will not work, commands are also case insensitive.

### Server Usage
---
When starting the server an IP and Port may be provided as command line arguments or in the terminal during run-time. This will open the server to accepting connections on that IP and Port.

When a client connects the server prompts for a username and will create User objects based on the connected client's socket, given username, and the room they reside in. The server handles all message formatting, when a client sends a message the time at which it's received is captured and the client message is concated with the time-stamp and that client's username before being broadcast to other clients on the server.

If one has access to the terminal the server is running on, they have the ability to send server-wide messages to all connected clients.