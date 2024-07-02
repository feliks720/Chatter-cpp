# Chatter-cpp
This project is a full-stack C++ chat application practice, including QT interface programming for the PC side, designing an asynchronous server with asio, setting up an HTTP gateway with the Beast networking library, building a verification service with Node.js, and using grpc for communication between services, with asio for communication between the server and client. It also includes user information entry and other features. The project achieves cross-platform design, initially developing the server on Windows and later considering migration to other platforms.

## Architecture Design
A brief structural design is shown in the following diagram.
![https://cdn.llfc.club/1709009717000.jpg](https://cdn.llfc.club/1709009717000.jpg)
1.GateServer acts as the gateway service, primarily handling client connections and registration requests. Since the server is distributed, GateServer queries the status service to select a server address with a lower load for the client upon receiving a connection request. The client then uses this address to establish a persistent connection directly with the server.

2.When a user registers, the registration request is sent to GateServer. GateServer calls VerifyServer to validate the registration and sends a verification code to the client. The client uses this verification code to register with GateServer.

3.StatusServer, ServerA, and ServerB can directly access Redis and MySQL services.

## Creating the Application
First, we will create the client's login interface. We'll start by using Qt to create Qt application widgets. We'll use the QLineEdit widget to create the username and password fields, and the QPushButton widget to create the login button. We'll also use the QMessageBox widget to display error messages.