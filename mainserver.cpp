#include <bits/stdc++.h>
#include <cstddef>
#include <iostream>
#include <cstring>
#include <strings.h>
#include <thread>
#include <vector>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
using namespace std;

const int MSG_LEN = 1024;
char msg[MSG_LEN];
map<int, pair<const char *, int>> ClientSockets, mp;
void ClientHandling(int clientSocket)
{

	cout << "\x1B[32m"
		 << "Client " << clientSocket << " connected"
		 << "\x1B[0m"
		 << "\n";
	while (true)
	{
		// Receive data from the client
		memset((char *)&msg, 0, sizeof(msg));
		int bytes = recv(clientSocket, msg, sizeof(msg), 0);
		if (strcmp(msg, "/exit") == 0 || strlen(msg) == 0)
		{
			cout << "\x1B[31m"
				 << "Client " << clientSocket << " disconnected"
				 << "\x1B[0m" << endl;
			string msgg = "ClientDisconnected " + to_string(clientSocket);
			const char *disconMsg = msgg.data();
			// cout << msgg << endl;
			ClientSockets.erase(clientSocket);
			for (auto x : ClientSockets)
			{
				// cout << msgg << endl;
				send(x.first, disconMsg, strlen(disconMsg), 0);
			}
			break;
		}
	}
	close(clientSocket);
}
void Handle_Input()
{
	while (true)
	{
		memset((char *)&msg, 0, sizeof(msg));
		cin.getline(msg, MSG_LEN);
		if (strcmp(msg, "/exit") == 0 || strlen(msg) == 0)
		{
			const char *disconMsg = "Server Disconnected";
			for (auto x : ClientSockets)
			{
				// cout << msgg << endl;
				send(x.first, disconMsg, strlen(disconMsg), 0);
			}
			exit(1);
		}
	}
}
int main(int argc, char *argv[])
{

	if (argc != 2)
	{
		cout << "Usage: " << argv[0] << " <serverPort>" << endl;
		return 1;
	}
	uint PORT = std::stoi(argv[1]);
	const char *Server_IP = "127.0.0.1";
	uint MSG_LEN = 1024;
	int max_clients = 30;
	int opt = 1;
	int master_socket, addrlen, new_socket, activity, i, valread, socket_descriptor_id;
	int max_socket_descriptor_id;
	struct sockaddr_in address;

	char message_buffer[MSG_LEN];
	fd_set readfds;
	const char *message = "Welcome Client! \r\n";

	if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		cout << "socket System Call Error: Failed to create Socket" << endl;
		exit(EXIT_FAILURE);
	}

	if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
				   sizeof(opt)) < 0)
	{
		cout << "setsockopt System Call Error" << endl;
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(Server_IP);
	cout << "ip: " << Server_IP << endl;
	address.sin_port = htons(PORT);

	if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		cout << "bind System Call Error: Failed to bind socket with Server_ID and Port" << endl;
		exit(EXIT_FAILURE);
	}
	cout << "Server on Port: " << PORT << std::endl;
	// cout << master_socket << endl;
	if (listen(master_socket, 3) < 0)
	{
		cout << "listen System Call Error" << endl;
		exit(EXIT_FAILURE);
	}

	cout << "Waiting for connections...." << std::endl;

	thread(Handle_Input).detach();
	while (true)
	{
		addrlen = sizeof(address);
		new_socket = accept(master_socket, (sockaddr *)&address, (socklen_t *)&addrlen);
		if (new_socket == -1)
			continue;
		char message[MSG_LEN];
		int bytes = recv(new_socket, message, sizeof(message), 0);
		char *msgid = to_string(new_socket).data();
		send(new_socket, msgid, sizeof(msgid), 0);
		int port = atoi(message);
		for (auto x : ClientSockets)
		{
			string Msg = "ClientConnected " + to_string(new_socket) + " " + to_string(port) + " " + to_string(x.first);
			const char *connectMsg = Msg.data();
			send(x.first, connectMsg, strlen(connectMsg), 0);
		}
		const char *IP = "127.0.0.1";
		ClientSockets[new_socket] = {IP, port};

		thread(ClientHandling, new_socket).detach();
	}
	// Close server socket
	close(master_socket);
	return 0;
}