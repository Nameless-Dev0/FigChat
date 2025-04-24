<h1 align="center" id="title">FigChat</h1>

<p align="center">
  <img src="images/fig.png" alt="FigChat Logo" width="300">
</p>

<p align="center" id="description">
A real-time chat application built using TCP/IP Networking, C++, and the Qt Framework.<br>
FigChat uses a centralized server model and client-server architecture, offering a seamless messaging experience with support for private messaging, broadcasts, file sharing, and more.
</p>

---

## Features

- Real-time Messaging: Instant communication between clients over TCP/IP.
- Private & Broadcast Messaging: Send messages directly to individuals or to all connected users.
- File Transfer Support: Upload and share files to server.
- Centralized Server Architecture: Manages client connections, message routing, and data flow.
- User-friendly GUI: Built using the Qt framework for a smooth user experience.
- Robust Error Handling: Ensures stable client-server communication.

---

## Tech Stack

- Programming Language: C++
- Framework: Qt (for GUI and networking)
- Networking: TCP/IP (using QTcpSocket, QTcpServer)
- Architecture: Client-Server (Centralized)

---

## Installation

### Prerequisites

- Qt Framework (Qt version 6)
- C++17 compatible compiler
- QMake (using Qt Creator)

### Clone the Repository

```bash
git clone https://github.com/Nameless-Dev0/FigChat.git
cd FigChat

Client:
Open qmake file from the client called client.pro from the client folder using QtCreator
Select and Configure the Qt Kit 6.9 
Build release and run

Server:
Open qmake file from the client called server.pro from the server folder using QtCreator
Select and Configure the Qt Kit 6.9 
Build release and run

