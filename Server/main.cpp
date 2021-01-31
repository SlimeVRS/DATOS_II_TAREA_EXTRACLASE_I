#include <iostream>
#include <sstream>
#include <list>
#include <WS2tcpip.h>
#pragma comment (lib, "ws2_32.lib")

using namespace std;

#define INF 99999

void printSolution(int *dist, int x, int y);
void floydWarshall(int *graph, int x, int y);
void convertToGraph(std::string);

int main(){
    WSADATA wsData;
    WORD ver = MAKEWORD(2, 2);

    int Succesful = WSAStartup(ver, &wsData);

    if(Succesful != 0) {
        cerr << "Can't initialize winsock! Quitting" << endl;
        return 0;
    }

    SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
    if(listening == INVALID_SOCKET){
        cerr << "Can't create a socket! Quitting!" << endl;
        return 0;
    }
    SOCKADDR_IN hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(8080);
    hint.sin_addr.S_un.S_addr = INADDR_ANY;

    bind(listening, (sockaddr*)&hint, sizeof(hint));

    listen(listening, SOMAXCONN);

    sockaddr_in client;
    int clientSize = sizeof(client);

    SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
    if(clientSocket == INVALID_SOCKET){
    //connection invalid
    }
    char host[NI_MAXHOST];      // Client's remote name
    char service[NI_MAXHOST];    // Service (i.e port) the client is connected on

    ZeroMemory(host, NI_MAXHOST);
    ZeroMemory(service, NI_MAXHOST);

    if(getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXHOST, 0) == 0) {
        cout << host << "connected on port" << service << endl;
    }
    else {
        inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
        cout << host << " connect on port " << ntohs(client.sin_port) << endl;
    }
    closesocket(listening);
    char buf[4096];
    while(true){
        ZeroMemory(buf, 4096);
        int bytesReceived = recv(clientSocket, buf, 4096, 0);
        if(bytesReceived == SOCKET_ERROR){
            cerr << "Error in recv(). Quitting" << endl;
            break;
        }

        if(bytesReceived == 0) {
            cout << "Client disconnected " << endl;
            break;
        }
        string message = string(buf, 0, bytesReceived);
        convertToGraph(message);
        send(clientSocket, buf, bytesReceived + 1, 0);
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}

void convertToGraph(string message) {
    string param = "";
    int x = 0;
    int y = 0;

    cout << "Looking for parameters in: " << message << endl;
    for(int i = 0; i < message.size(); i++) {
        if(message.compare(i,1,")") == 0) {
            stringstream value(param);
            value >> y;
            param = "";
            message.erase(0, i+1);
            i = message.size();
        }
        else if(message.compare(i, 1, ",") == 0){
            stringstream value(param);
            value >> x;
            param = "";
        }
        else if(message.compare(i, 1, "(") == 0) {
            x = 0;
            y = 0;
            param = "";
        }
        else {
            param.append(message, i, 1);
        }
    }
    cout << "X is: " << x << " Y is: " << y << endl;
    int a = x;
    int b = y;
    int temp = 0;

    int *mat = (int *) malloc(a*b*sizeof(int));
    cout << "Looking for graph parameters in: " << message << endl;
    for(int j = 1; j < message.size(); j++) {
        if(message.compare(j,1,"}") == 0) {
            if(message.compare(j-1,1,"}") == 0) {
                a = 0;
                b = 0;
                param = "";
                message.erase(0,j+1);
                j = message.size();
            }
            else {
                if(param == "INF") {
                    temp = INF;
                }
                else {
                    stringstream value(param);
                    value >> temp;
                }
                mat[a*y+b] = temp;
                param = "";
            }
        }
        else if(message.compare(j, 1, ",") == 0) {
            if((message.compare(j-1, 1, "}") == 0)&&(message.compare(j+1,1,"{") == 0)) {
                param = "";
                a++;
                b = 0;
            }
            else {
                if(param == "INF") {
                    temp = INF;
                }
                else {
                    stringstream value(param);
                    value >> temp;
                }
                mat[a*y + b] = temp;
                param = "";
                b++;
            }
        }
        else if(message.compare(j, 1, "{") == 0) {
            if(message.compare(j - 1, 1, "{") == 0) {
                a = 0;
                b = 0;
                param = "";
            }
            else {
                param = "";
            }
        }
        else {
            param.append(message, j, 1);
        }
    }
    floydWarshall(mat, x, y);
}

void floydWarshall(int *graph, int x, int y){
    int *dist = (int *)malloc(x*y*sizeof(int));
    int i, j, k;
    for(i = 0; i < x; i++) {
        for(j = 0; j < y; j++) {
            dist[i*y+j] = graph[i*y+j];
        }
    }
    for(k = 0; k < x; k++) {
        for(i = 0; i < x; i++) {
            for(j = 0; j < y; j++) {
                if(dist[i*x + k] + dist[x*y+j] < dist[i*y+j]){
                    dist[i*x + j] = dist[i*x+k] + dist[k*x+j];
                }
            }
        }
    }
    printSolution(dist, x, y);
}

void printSolution(int *dist, int x, int y) {
    cout << "The following matrix shows the shortest distances between every pair of vertices \n";
    for(int i = 0; i < x; i++) {
        for(int j = 0; j < x; j++) {
            if(dist[i*y+j] == INF) {
                cout << "INF" << "      ";
            }
            else {
                cout << dist[i*y+j]<<"      ";
            }
        }
        cout << endl;
    }
}