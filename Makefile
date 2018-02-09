all: server client
server: server.cpp
	g++ --std=c++11 -o server_20172062 server.cpp
client: client.cpp
	g++ --std=c++11 -o client_20172062 client.cpp
