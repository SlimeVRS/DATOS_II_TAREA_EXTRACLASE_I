//C++ SERVER with Floyd/Warshall algorithm
#include <iostream>
#include <sstream>
#include <list>
#include <WS2tcpip.h>
#pragma comment (lib, "ws2_32.lib")

using namespace std;

/** Define INFINITE as a large enough value.
This value will be used for vertices not
connected to each other */
#define INF 99999

/** A function to print the solution matrix*/
void printSolution(int *dist, int x, int y);
/** A function to find the shortest distance between every paor of vertices*/
void floydWarshall(int *graph, int x, int y);
/** A function to convert the string received from the client to a graph*/
void convertToGraph(std::string);

int main(){
    // Initialize winsock
    WSADATA wsData;
    WORD ver = MAKEWORD(2, 2);

    int Succesful = WSAStartup(ver, &wsData);

    if(Succesful != 0) {
        cerr << "Can't initialize winsock! Quitting" << endl;
        return 0;
    }

    // Create a socket
    SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
    if(listening == INVALID_SOCKET){
        cerr << "Can't create a socket! Quitting!" << endl;
        return 0;
    }
    // Bind the ip addres and port to a socket
    SOCKADDR_IN hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(8080);
    hint.sin_addr.S_un.S_addr = INADDR_ANY;

    bind(listening, (sockaddr*)&hint, sizeof(hint));

    // Tell winsock the socket is for listening
    listen(listening, SOMAXCONN);

    // Wait for a connection
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

    // Close listening socket
    closesocket(listening);

    // While loop: accept and echo message back to client
    char buf[4096];

    while(true){
        ZeroMemory(buf, 4096);

        // Wait for the client to send data
        int bytesReceived = recv(clientSocket, buf, 4096, 0);
        if(bytesReceived == SOCKET_ERROR){
            cerr << "Error in recv(). Quitting" << endl;
            break;
        }
        if(bytesReceived == 0) {
            cout << " Client disconnected " << endl;
            break;
        }

        // Data from clients
        // cout << string(buf, 0, bytesReceived) << endl;
        string message = string(buf, 0, bytesReceived);
        convertToGraph(message);

        // Echo message back to client
        send(clientSocket, buf, bytesReceived + 1, 0);
    }

    // Close the socket
    closesocket(clientSocket);

    // Shutdown winsock
    WSACleanup();

    return 0;
}
// A function to convert the string received from the client to a graph
void convertToGraph(string message) {

    //Loop to establish the x an y of the graph
    string param = "";
    int x = 0;
    int y = 0;

    cout << "Looking for parameters in: " << message << endl;
    
    for(int i = 0; i < message.size(); i++) {
        //cout << message[i] << endl;
        if(message.compare(i,1,")") == 0) {
            //cout << "case: )" << endl;
            stringstream value(param);
            value >> y;
            param = "";
            message.erase(0, i+1);
            i = message.size();
        }
        else if(message.compare(i, 1, ",") == 0){
            //cout << "case: ," << endl;
            stringstream value(param);
            value >> x;
            param = "";
        }
        else if(message.compare(i, 1, "(") == 0) {
            //cout << "case: (" << endl;
            x = 0;
            y = 0;
            param = "";
        }
        else {
            param.append(message, i, 1);
            //cout << "current param: " << param << endl;
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
                //cout << "case: }}" << endl;
                a = 0;
                b = 0;
                param = "";
                message.erase(0,j+1);
                j = message.size();
            }
            else {
                //cout << "case: }" << endl;
                if(param == "INF") {
                    //cout << "case INF" << endl;
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
                //cout << "case: },{" << endl;
                param = "";
                a++;
                b = 0;
            }
            else {
                //cout << "case: ," << endl;
                if(param == "INF") {
                    //cout << "case INF" << endl;
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
                //cout << "case: {{" << endl;
                a = 0;
                b = 0;
                param = "";
            }
            else {
                //cout << "case: {" << endl;
                param = "";
            }
        }
        else {
            param.append(message, j, 1);
            //cout << "current param: " << param << endl;
        }
    }
    //printSolution(mat,x,y);
    floydWarshall(mat, x, y);

}

// Solves the all-pairs shortest path
// problem using Floyd Warshall algorithm
void floydWarshall(int *graph, int x, int y){

    /* dist[][] will be the output matrix
    that will finally have the shortest
    distances between every pair of vertices */
    int *dist = (int *)malloc(x*y*sizeof(int));
    int i, j, k;

    /* Initialize the solution matrix same
    as input graph matrix. Or we can say
    the initial values of shortest distances
    are based on shortest paths considering
    no intermediate vertex. */
    for(i = 0; i < x; i++) {
        for(j = 0; j < y; j++) {
            dist[i*y+j] = graph[i*y+j];
        }
    }

        /* Add all vertices one by one to
    the set of intermediate vertices.
    ---> Before start of an iteration,
    we have shortest distances between all
    pairs of vertices such that the
    shortest distances consider only the
    vertices in set {0, 1, 2, .. k-1} as
    intermediate vertices.
    ----> After the end of an iteration,
    vertex no. k is added to the set of
    intermediate vertices and the set becomes {0, 1, 2, .. k} */
    for(k = 0; k < x; k++) {
        // Pick all vertices as source one by one
        for(i = 0; i < x; i++) {
            /** Pick all vertices as destination for the
             * above picked source
            */
            for(j = 0; j < y; j++) {
                // If vertex k is on the shortest path from
                // i to j, then update the value of dist[i][j]
                if(dist[i*x + k] + dist[x*y+j] < dist[i*y+j]){
                    dist[i*x + j] = dist[i*x+k] + dist[k*x+j];
                }
            }
        }
    }

    // Print the shortest distance matrix
    printSolution(dist, x, y);
}

/* A utility function to print solution */
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