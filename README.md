# ChatProject
This is a real-time chat program with server and client written in *C++*. It allows users to send and receive messages instantly.

## Features
- Real-time chat.
- User registration and authentication.

## Requirements
You need to have at least version 11 of *CMake* and *C++* installed.

## Installation
To configure and run the project locally, follow these steps:
1. Clone the repository:
	```bash
	git clone https://github.com/KevinJGard/ChatProject.git
	```
2. Navigate to the project directory:
	```bash
	cd ChatProject/
	```
3. Create a **build/** directory:
	```bash
	mkdir build/
	```
4. Navigate to the build/ directory:
	```bash
	cd build/
	```
5. Execute the following command:
	```bash
	cmake ..
	```
6. Compile the project with:
	```bash
	make
	```

## Usage
To start the server and the client, run the following commands in two separate terminals:
1. Start the server.
	```bash
	./server <IP> <port>
	```
2. Start the client.
	```bash
	./client <IP> <port>
	```
**<IP>** is the IP address where you want it to run.
**<port>** is the port on which you want it to run.

After running correctly, in the client you can put the command /help to display the following:
-Type /status_ACTIVATE to set your status to activated.
-Type /status_AWAY to set your status to away.
-Type /status_BUSY to set your status to busy.
-Type /users to see the list of users in the chat.
-Type /pvtmsg_<username> <message> to send a private message to another user.
-Type /exit to disconnect from the chat.