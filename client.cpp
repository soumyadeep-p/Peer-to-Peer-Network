#include <bits/stdc++.h>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>
using namespace std;

const int MSG_LEN = 1024;
char msg[MSG_LEN];
int client_server_socket;
map<int, int> client_to_client_sockets;
const char *Server_IP;
void ClientHandling(int clientSocket, int org)
{
    int org_socket = org;
    while (true)
    {
        // Receive message from server
        memset((char *)&msg, 0, sizeof(msg));
        recv(clientSocket, msg, sizeof(msg), 0);
        int idx = 0;
        if (strlen(msg) == 0)
        {
            close(clientSocket);
            break;
        }
        if (msg[idx] == '0')
        {
            idx += 2;
            char recvmsg[10];
            int x = 0;
            while (idx < sizeof(msg) && msg[idx] != '\0')
            {
                recvmsg[x] = msg[idx];
                idx++;
                x++;
            }
            org_socket = atoi(recvmsg);
            // cout << "org_socket: " << org_socket << endl;
            client_to_client_sockets[org_socket] = clientSocket;
            cout << "\x1B[32m"
                 << "Connected to Client " << org_socket << "\x1B[0m" << endl;
            continue;
        }
        cout << "Client " << org_socket << ": "
             << "\x1B[33m" << msg << "\x1B[0m"
             << "\n";
    }
}

void InputHandling()
{
    while (true)
    {
        // take input
        int flag = 0;
        memset((char *)&msg, 0, sizeof(msg));
        cin.getline(msg, MSG_LEN);
        if (strcmp(msg, "/exit") == 0 || strlen(msg) == 0)
        {
            // cout << "bye" << endl;
            send(client_server_socket, msg, strlen(msg), 0);
            break;
        }
        int idx = 0;
        string clientSoc_str;
        int clientSoc;
        if (msg[idx] == ' ')
            flag = 1;
        while (msg[idx] != ' ' && idx < sizeof(msg))
        {
            clientSoc_str += msg[idx];
            if (msg[idx] < '0' || msg[idx] > '9')
                flag = 1;
            idx++;
        }
        idx++;
        if (flag != 1)
            clientSoc = stoi(clientSoc_str);
        // cout << clientSoc << endl;
        if (clientSoc == 1)
        {
            char actualmsg[1024];
            int x = 0;
            while (msg[idx] != '\n' && msg[idx] != '\0' && idx < sizeof(msg))
            {
                actualmsg[x] = msg[idx];
                idx++;
                x++;
            }
            string s = " [Broadcasted Message]";
            for (int y = 0; y < s.length(); y++)
            {
                actualmsg[x] = s[y];
                x++;
            }
            actualmsg[x] = '\0';
            for (auto x : client_to_client_sockets)
            {
                send(x.second, actualmsg, strlen(actualmsg), 0);
            }
        }
        else
        {
            char actualmsg[1024];
            int x = 0;
            while (msg[idx] != '\n' && msg[idx] != '\0' && idx < sizeof(msg))
            {
                actualmsg[x] = msg[idx];
                idx++;
                x++;
            }
            actualmsg[x] = '\0';
            // cout << actualmsg << endl;
            if (client_to_client_sockets.count(clientSoc) == 0)
                flag = 1;
            if (flag == 0)
                send(client_to_client_sockets[clientSoc], actualmsg, strlen(actualmsg), 0);
            else
                cout << "Please use correct format : <clientNo.> <Msg>" << endl;
        }
    }
    close(client_server_socket);
    exit(1);
}
void MessageRec_Server()
{
    while (true)
    {
        // Receive message from server
        memset((char *)&msg, 0, sizeof(msg));
        recv(client_server_socket, msg, sizeof(msg), 0);
        if (strcmp(msg, "/exit") == 0 || strlen(msg) == 0)
        {
            close(client_server_socket);
            break;
        }
        int idx = 0;
        string signal;
        while (msg[idx] != ' ')
        {
            signal += msg[idx];
            idx++;
        }
        idx++;
        // newclient connected
        if (signal == "ClientConnected")
        {
            string newClient, port_str, to_send_Socketstr;
            struct sockaddr_in clientAddr;
            while (msg[idx] != ' ')
            {
                newClient += msg[idx];
                idx++;
            }
            idx++;
            while (msg[idx] != ' ')
            {
                port_str += msg[idx];
                idx++;
            }
            idx++;
            while (msg[idx] != '\0' && idx < sizeof(msg))
            {
                to_send_Socketstr += msg[idx];
                idx++;
            }
            int port = stoi(port_str);
            int newClientSocketid = stoi(newClient);
            // int to_send_sockid = stoi(to_send_Sockerstr);
            int client_socket = socket(AF_INET, SOCK_STREAM, 0);
            // cout<<port<<" "<<Server_IP<<endl;
            clientAddr.sin_family = AF_INET;
            clientAddr.sin_port = htons(port);
            clientAddr.sin_addr.s_addr = inet_addr(Server_IP);
            // cout << msg << endl;
            if (connect(client_socket, (struct sockaddr *)&clientAddr, sizeof(clientAddr)) < 0)
            {
                cout << "\x1B[31m"
                     << "connect System Call Error : Failed to connect to the client"
                     << "\x1B[0m" << endl;
                continue;
            }
            cout << "\x1B[32m"
                 << "Connected to Client " << newClientSocketid << "\x1B[0m" << endl;
            to_send_Socketstr = "0 " + to_send_Socketstr;

            const char *confmsg = to_send_Socketstr.data();
            send(client_socket, confmsg, strlen(confmsg), 0);
            client_to_client_sockets[newClientSocketid] = client_socket;
            thread(ClientHandling, client_socket, newClientSocketid).detach();
        }
        else if (signal == "ClientDisconnected")
        {
            string client_str;
            while (msg[idx] != '\0' && idx < sizeof(msg))
            {
                client_str += msg[idx];
                idx++;
            }
            // cout << client_str << endl;
            int client_socket = stoi(client_str);
            close(client_to_client_sockets[client_socket]);
            cout << "\x1B[31m"
                 << "Client " << client_socket << " Disconnected."
                 << "\x1B[0m" << endl;
        }
        else
        {
            cout << "\x1B[31m"
                 << "Server Disconnected "
                 << "\x1B[0m" << endl;
        }
    }
    exit(1);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <Server_IP_Address> <Server_Port_Number>\n", argv[0]);
        return 1;
    }
    Server_IP = argv[1];
    const char *Client_IP = argv[1];
    int Server_Port = atoi(argv[2]);
    int Client_Port;
    int opt = 1;
    // listening client socket
    int listen_client_socket;
    struct sockaddr_in address, serverAddr;
    listen_client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_client_socket == -1)
    {
        cout << "socket System Call Error : Failed to create Client Socket" << endl;
        return 1;
    }
    // if ((listen_client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    // {
    //     cout << "socket System Call Error: Failed to create Socket" << endl;
    //     exit(EXIT_FAILURE);
    // }

    if (setsockopt(listen_client_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
    {
        cout << "setsockopt System Call Error" << endl;
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(Client_IP);
    // cout << "ip: " << Server_IP << endl;
    vector<int> ports;
    for (int x = 0; x < 20; x++)
    {
        ports.push_back(3000 + x);
    }
    for (int x = 0; x < 20; x++)
    {
        Client_Port = ports[x];
        address.sin_port = htons(Client_Port);
        if (bind(listen_client_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
        {
            continue;
        }
        else
            break;
    }
    // cout << "Server on Port: " << PORT << std::endl;
    // cout << listen_client_socket << endl;
    if (listen(listen_client_socket, 3) < 0)
    {
        cout << "listen System Call Error" << endl;
        exit(EXIT_FAILURE);
    }

    // client connecting to server
    client_server_socket = socket(AF_INET, SOCK_STREAM, 0);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(Server_Port);
    serverAddr.sin_addr.s_addr = inet_addr(Server_IP);

    if (connect(client_server_socket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        cout << "\x1B[31m"
             << "connect System Call Error : Failed to connect to the server"
             << "\x1B[0m" << endl;
        return 1;
    }
    const char *msgg = to_string(Client_Port).data();
    cout << "\x1B[34m"
         << "Connected to server\n"
         << "\x1B[0m" << endl;
    send(client_server_socket, msgg, strlen(msgg), 0);
    memset((char *)&msg, 0, sizeof(msg));
    recv(client_server_socket, msg, sizeof(msg), 0);
    cout << "Client ID: " << msg << endl;
    thread(MessageRec_Server).detach();
    thread(InputHandling).detach();
    while (true)
    {
        int addrlen = sizeof(address);
        // cout << "waiting..." << endl;
        int new_socket = accept(listen_client_socket, (sockaddr *)&address, (socklen_t *)&addrlen);
        if (new_socket == -1)
            continue;
        thread(ClientHandling, new_socket, new_socket).detach();
    }

    return 0;
}
