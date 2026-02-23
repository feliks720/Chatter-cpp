# Chatter-cpp
This project is a full-stack C++ chat application practice, including QT interface programming for the PC side, designing an asynchronous server with asio, setting up an HTTP gateway with the Beast networking library, building a verification service with Node.js, and using grpc for communication between services, with asio for communication between the server and client. It also includes user information entry and other features. The project achieves cross-platform design, initially developing the server on Windows and later considering migration to other platforms.

## Architecture Design
A brief structural design is shown in the following diagram.
![https://cdn.llfc.club/1709009717000.jpg](https://cdn.llfc.club/1709009717000.jpg)
1.GateServer acts as the gateway service, primarily handling client connections and registration requests. Since the server is distributed, GateServer queries the status service to select a server address with a lower load for the client upon receiving a connection request. The client then uses this address to establish a persistent connection directly with the server.

2.When a user registers, the registration request is sent to GateServer. GateServer calls VerifyServer to validate the registration and sends a verification code to the client. The client uses this verification code to register with GateServer.

3.StatusServer, ServerA, and ServerB can directly access Redis and MySQL services.

## Current Functionality (Implemented)

### GateServer (`server/GateServer`)

- Async HTTP gateway (Boost.Asio + Beast)
- Verification code request endpoint: `POST /get_varifycode` (and alias `POST /get_verifycode`)
- User registration endpoint: `POST /register`
- User login endpoint: `POST /login`
- Realtime chat endpoint (WebSocket): `GET /ws` upgrade
- In-memory user store + verification-code expiry handling (3 minutes)

### VerifyServer (`server/VarifyServer`)

- gRPC service `GetVarifyCode`
- Sends email verification code (Node.js + nodemailer)
- Returns `email`, `error`, and `code`

### Qt Client (`client/llfcchat`)

- Register UI:
  - request verification code
  - submit registration
  - input validation and user feedback
- Login UI:
  - submit login request
  - success/failure feedback

### Console Realtime Client (`client/console`)

- C++ WebSocket terminal client for realtime chat testing
- Supports:
  - sending chat messages
  - `/nick your_name` nickname command
  - `/quit` to exit

## API Overview

### `POST /get_varifycode`
Request:
```json
{
  "email": "user@example.com"
}
```

Response:
```json
{
  "error": 0,
  "email": "user@example.com",
  "code": "123456"
}
```

### `POST /register`
Request:
```json
{
  "user": "alice",
  "email": "user@example.com",
  "password": "secret123",
  "code": "123456"
}
```

### `POST /login`
Request:
```json
{
  "user": "alice",
  "password": "secret123"
}
```

### `GET /ws` (WebSocket)
- Connect to `ws://<gate-host>:<gate-port>/ws`
- Messages are broadcast to all connected clients

## Console Chat Client Build (Linux)

```bash
cd client/console
cmake -S . -B build
cmake --build build -j
./build/chat_cli 127.0.0.1 8080 /ws
```