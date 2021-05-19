
#include "server_funcs.h"


typedef enum enStatusList {
	EMPTY = 0,
	LISTEN,
	RECEIVE,
	IDLE,
	SEND,
};

typedef enum enCommandList {
	GET = 1,
	HEAD,
	PUT,
	POST,
	D_DELETE,
	TRACE,
	OPTIONS 
};


struct stSocketState
{
	SOCKET id;
	enStatusList recv;
	enStatusList send;
	enCommandList HTTP_command;
	time_t tLastActivity;
	char sSocketData[BUFF_SIZE];
	int iSocketDataLen;
};


bool addSocket(SOCKET id, enStatusList what);
void removeSocket(int index);
void acceptConnection(int index);
void receiveMessage(int index);
bool sendMessage(int index);
int WritePut(int index, char* filename);


struct stSocketState sockets[MAX_SOCKETS] = { 0 };
int socketsCount = 0;


void main()
{
	time_t tCurrentTime;

	//set timeout for Select function
	struct timeval tvTimeOut;
	tvTimeOut.tv_sec = 2;
	tvTimeOut.tv_usec = 0;


	// Initialize Winsock (Windows Sockets).

	// Create a WSADATA object called wsaData.
	// The WSADATA structure contains information about the Windows 
	// Sockets implementation.
	WSAData wsaData;

	// Call WSAStartup and return its value as an integer and check for errors.
	// The WSAStartup function initiates the use of WS2_32.DLL by a process.
	// First parameter is the version number 2.2.
	// The WSACleanup function destructs the use of WS2_32.DLL by a process.
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		cout << "HTTP Server: Error at WSAStartup()\n";
		return;
	}
	// Server side:
	// Create and bind a socket to an internet address.
	// Listen through the socket for incoming connections.

	// After initialization, a SOCKET object is ready to be instantiated.

	// Create a SOCKET object called listenSocket. 
	// For this application:	use the Internet address family (AF_INET), 
	//							streaming sockets (SOCK_STREAM), 
	//							and the TCP/IP protocol (IPPROTO_TCP).
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Check for errors to ensure that the socket is a valid socket.
	// Error detection is a key part of successful networking code. 
	// If the socket call fails, it returns INVALID_SOCKET. 
	// The if statement in the previous code is used to catch any errors that
	// may have occurred while creating the socket. WSAGetLastError returns an 
	// error number associated with the last error that occurred.
	if (INVALID_SOCKET == listenSocket)
	{
		cout << "HTTP Server: Error at socket(): " << WSAGetLastError() << endl;
		WSACleanup();
		return;
	}
	// For a server to communicate on a network, it must bind the socket to 
	// a network address.

	// Need to assemble the required data for connection in sockaddr structure.

	// Create a sockaddr_in object called serverService. 

	sockaddr_in serverService;
	// Address family (must be AF_INET - Internet address family).
	serverService.sin_family = AF_INET;
	// IP address. The sin_addr is a union (s_addr is a unsigned long 
	// (4 bytes) data type).
	// inet_addr (Iternet address) is used to convert a string (char *) 
	// into unsigned long.
	// The IP address is INADDR_ANY to accept connections on all interfaces.
	serverService.sin_addr.s_addr = INADDR_ANY;
	// IP Port. The htons (host to network - short) function converts an
	// unsigned short from host to TCP/IP network byte order 
	// (which is big-endian). 
	serverService.sin_port = htons(PORT);

	// Bind the socket for client's requests.

	// The bind function establishes a connection to a specified socket.
	// The function uses the socket handler, the sockaddr structure (which
	// defines properties of the desired connection) and the length of the
	// sockaddr structure (in bytes).
	if (SOCKET_ERROR == bind(listenSocket, (SOCKADDR*)&serverService, sizeof(serverService)))
	{
		cout << "HTTP Server: Error at bind(): " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}

	// Listen on the Socket for incoming connections.
	// This socket accepts only one connection (no pending connections 
	// from other clients). This sets the backlog parameter.
	if (SOCKET_ERROR == listen(listenSocket, 50))
	{
		cout << "HTTP Server: Error at LISTEN(): " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}

	addSocket(listenSocket, LISTEN);
	// Accept connections and handles them one by one.

	cout << "Waiting for client connections..." << endl;

	while (true)
	{
		// The select function determines the status of one or more sockets,
		// waiting if necessary, to perform asynchronous I/O. Use fd_sets for
		// sets of handles for reading, writing and exceptions. select gets "timeout" for waiting
		// and still performing other operations (Use NULL for blocking). Finally,
		// select returns the number of descriptors which are ready for use (use FD_ISSET
		// macro to check which descriptor in each set is ready to be used).
		for (int i = 1; i < MAX_SOCKETS; i++)
		{
			tCurrentTime = time(0);
			if ((tCurrentTime - sockets[i].tLastActivity > 120) && (sockets[i].tLastActivity != 0))
			{
				removeSocket(i);
			}
		}

		fd_set waitRecv;
		FD_ZERO(&waitRecv);
		for (int i = 0; i < MAX_SOCKETS; i++)
		{
			if ((sockets[i].recv == LISTEN) || (sockets[i].recv == RECEIVE))
				FD_SET(sockets[i].id, &waitRecv);
		}

		fd_set waitSend;
		FD_ZERO(&waitSend);
		for (int i = 0; i < MAX_SOCKETS; i++)
		{
			if (sockets[i].send == SEND)
				FD_SET(sockets[i].id, &waitSend);
		}

		//
		// Wait for interesting event.
		// Note: First argument is ignored. The fourth is for exceptions.
		// And as written above the last is a timeout, hence we are blocked if nothing happens.
		//
		int nfd;
		nfd = select(0, &waitRecv, &waitSend, NULL, NULL);
		if (nfd == SOCKET_ERROR)
		{
			cout << "HTTP Server: Error at select(): " << WSAGetLastError() << endl;
			WSACleanup();
			return;
		}

		for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
		{
			if (FD_ISSET(sockets[i].id, &waitRecv))
			{
				nfd--;
				switch (sockets[i].recv)
				{
				case LISTEN:
					acceptConnection(i);
					break;

				case RECEIVE:
					receiveMessage(i);
					break;
				}
			}
		}

		for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
		{
			if (FD_ISSET(sockets[i].id, &waitSend))
			{
				nfd--;
				switch (sockets[i].send)
				{
				case SEND:
					if (!sendMessage(i))
						continue;
					break;
				}
			}
		}
	}

	// Closing connections and Winsock.

	cout << "HTTP Server: Closing Connection.\n";
	closesocket(listenSocket);
	WSACleanup();
}

bool addSocket(SOCKET id, enStatusList what)
{
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if (sockets[i].recv == EMPTY)
		{
			sockets[i].id = id;
			sockets[i].recv = what;
			sockets[i].send = IDLE;
			sockets[i].iSocketDataLen = 0;
			sockets[i].tLastActivity = time(0);
			socketsCount++;
			return (true);
		}
	}
	return (false);
}

void removeSocket(int index)
{
	cout << "The socket number " << index << " has been removed" << endl;
	sockets[index].recv = EMPTY;
	sockets[index].send = EMPTY;
	sockets[index].tLastActivity = 0;
	socketsCount--;
}

void acceptConnection(int index)
{
	SOCKET id = sockets[index].id;
	sockets[index].tLastActivity = time(0);
	struct sockaddr_in from;		// Address of sending partner
	int fromLen = sizeof(from);

	SOCKET msgSocket = accept(id, (struct sockaddr*)&from, &fromLen);
	if (INVALID_SOCKET == msgSocket)
	{
		cout << "HTTP Server: Error at accept(): " << WSAGetLastError() << endl;
		return;
	}
	cout << "HTTP Server: Client " << inet_ntoa(from.sin_addr) << ":" << ntohs(from.sin_port) << " is connected." << endl;


	//
	// Set the socket to be in non-blocking mode.
	//

	unsigned long flag = 1;
	if (ioctlsocket(msgSocket, FIONBIO, &flag) != 0)
	{
		cout << "HTTP Server: Error at ioctlsocket(): " << WSAGetLastError() << endl;
	}

	if (addSocket(msgSocket, RECEIVE) == false)
	{
		cout << "\t\tToo many connections, dropped!\n";
		closesocket(id);
	}
	return;
}

void receiveMessage(int index)
{
	SOCKET msgSocket = sockets[index].id;
	char* tmp = NULL;

	int len = sockets[index].iSocketDataLen;
	int bytesRecv = recv(msgSocket, &sockets[index].sSocketData[len], sizeof(sockets[index].sSocketData) - len, 0);

	if (SOCKET_ERROR == bytesRecv)
	{
		cout << "HTTP Server: Error at recv(): " << WSAGetLastError() << endl;
		closesocket(msgSocket);
		removeSocket(index);
		return;
	}
	sockets[index].tLastActivity = time(0);
	if (bytesRecv == 0)
	{
		closesocket(msgSocket);
		removeSocket(index);
		return;
	}
	else
	{
		sockets[index].sSocketData[len + bytesRecv] = '\0';
		cout << "HTTP Server: Recieved: " << bytesRecv << " bytes of \"" << &sockets[index].sSocketData[len] << "\n---message---\n";
		sockets[index].iSocketDataLen += bytesRecv;
		if (sockets[index].iSocketDataLen > 0)
		{
			if (strncmp(sockets[index].sSocketData, "HEAD", 4) == 0)
			{
				sockets[index].send = SEND;
				sockets[index].HTTP_command = HEAD;
				strcpy(sockets[index].sSocketData, &sockets[index].sSocketData[6]);
				sockets[index].iSocketDataLen = strlen(sockets[index].sSocketData);
				sockets[index].sSocketData[sockets[index].iSocketDataLen] = NULL;
				return;
			}
			else if (strncmp(sockets[index].sSocketData, "GET", 3) == 0)
			{
				sockets[index].HTTP_command = GET;
				strcpy(sockets[index].sSocketData, &sockets[index].sSocketData[5]);
				sockets[index].iSocketDataLen = strlen(sockets[index].sSocketData);
				sockets[index].sSocketData[sockets[index].iSocketDataLen] = NULL;	//did you mean \0			
			}
			else if (strncmp(sockets[index].sSocketData, "PUT", 3) == 0)
			{				
				sockets[index].HTTP_command = PUT;	
				strcpy(sockets[index].sSocketData, &sockets[index].sSocketData[5]);
				sockets[index].iSocketDataLen = strlen(sockets[index].sSocketData);
				sockets[index].sSocketData[sockets[index].iSocketDataLen] = NULL;
			}
			else if (strncmp(sockets[index].sSocketData, "DELETE", 6) == 0)
			{
				strcpy(sockets[index].sSocketData, &sockets[index].sSocketData[8]);				
				sockets[index].HTTP_command = D_DELETE;				
			}
			else if (strncmp(sockets[index].sSocketData, "TRACE", 5) == 0)
			{				
				sockets[index].HTTP_command = TRACE;
				strcpy(sockets[index].sSocketData, &sockets[index].sSocketData[5]);
				sockets[index].iSocketDataLen = strlen(sockets[index].sSocketData);
				sockets[index].sSocketData[sockets[index].iSocketDataLen] = NULL;
			}
			else if (strncmp(sockets[index].sSocketData, "POST", 4) == 0)
			{
				sockets[index].HTTP_command = POST;
				strcpy(sockets[index].sSocketData, &sockets[index].sSocketData[6]);
				sockets[index].iSocketDataLen = strlen(sockets[index].sSocketData);
				sockets[index].sSocketData[sockets[index].iSocketDataLen] = NULL;				
			}
			else if (strncmp(sockets[index].sSocketData, "OPTIONS", 7) == 0)
			{				
				sockets[index].HTTP_command = OPTIONS;
				//strcpy(sockets[index].sSocketData, &sockets[index].sSocketData[5]);
				//sockets[index].iSocketDataLen = strlen(sockets[index].sSocketData);
				//sockets[index].sSocketData[sockets[index].iSocketDataLen] = NULL;			
			}
			sockets[index].send = SEND;
			return;
		}
	}
}

bool sendMessage(int index)
{
	int bytesSent = 0;
	string sendBuff;
	int buffLen = 0, intFsize = 0;
	string strFsize;
	string sFullMessage, tmpStringBuff;
	char* tmp = 0;
	char ctmp[20];

	char tmpbuff[BUFF_SIZE];
	char readBuff[512], filename[128];
	int retCode;
	time_t rawtime;
	ifstream inFile;
	time(&rawtime);
	SOCKET msgSocket = sockets[index].id;
	sockets[index].tLastActivity = time(0);
	switch (sockets[index].HTTP_command)
	{
	case HEAD:
		sendHeadResponse(sockets[index].sSocketData, &sendBuff);
		break;
	case GET:
		sendGetResponse(sockets[index].sSocketData, &sendBuff);
		break;
	case PUT:
		sendPutResponse(sockets[index].sSocketData, &sendBuff);
		break;
	case D_DELETE:
		sendDeleteRepsonse(sockets[index].sSocketData, &sendBuff);
		break;
	case TRACE:

		intFsize = strlen("TRACE");
		intFsize += strlen(sockets[index].sSocketData);
		sFullMessage = "HTTP/1.1 200 OK \r\nContent-type: message/http\r\nDate: ";
		sFullMessage += ctime(&rawtime);
		sFullMessage += "Content-length: ";
		strFsize = _itoa(intFsize, ctmp, 10);
		sFullMessage += strFsize;
		sFullMessage += "\r\n\r\n";

		sFullMessage += "TRACE";
		sFullMessage += sockets[index].sSocketData;
		buffLen = sFullMessage.size();
		sendBuff = sFullMessage;
		break;
	case OPTIONS:
			sendOptionsResponse(sockets[index].sSocketData, &sendBuff);
			break;
	case POST:
			PrintPostToConsole(sockets[index].sSocketData, &sendBuff);
			break;
	}

	buffLen = sendBuff.size();
	bytesSent = send(msgSocket, sendBuff.c_str(), buffLen, 0);
	memset(sockets[index].sSocketData, 0, BUFF_SIZE);
	sockets[index].iSocketDataLen = 0;
	if (SOCKET_ERROR == bytesSent)
	{
		cout << "HTTP Server: Error at S_SEND(): " << WSAGetLastError() << endl;
		return false;
	}

	cout << "HTTP Server: Sent: " << bytesSent << "\\" << buffLen << " bytes of\n \"" << sendBuff << "\n---message---\n";
	sockets[index].send = IDLE;
	return true;
}
