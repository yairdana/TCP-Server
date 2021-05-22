#include "server.h"
//infinite loop when send HEAD - it will trigger remove socket
// post - body is big then the size (the reponse and the req are different)
// 
struct SocketState sockets[MAX_SOCKETS] = { 0 };
int socketsCount = 0;

const char* httpRequests[] =
{
	"GET",
	"HEAD",
	"POST",
	"PUT",
	"DELETE",
	"TRACE",
	"OPTIONS"
};


void main()
{

	WSAData wsaData = initWsaData();
	SOCKET listenSocket = initSocket();
	sockaddr_in serverService = createSocketAddr();
	bindSocketForClientRequests(&listenSocket, &serverService);
	listenOnSocketForIncomingConnection(&listenSocket);
	addSocket(listenSocket, eSocketStatus::LISTEN);

	cout << "Waiting for client connections..." << endl;
	// Accept connections and handles them one by one.
	while (true)
	{
		deleteStuckRequests();
		fd_set waitRecv= initWaitRecvSockets();
		fd_set waitSend = initWaitSendSockets();

		// Wait for interesting event.
		int nfd;
		nfd = select(0, &waitRecv, &waitSend, NULL, NULL);
		if (nfd == SOCKET_ERROR)
		{
			cout << "HTTP Server: Error at select(): " << WSAGetLastError() << endl;
			WSACleanup();
			return;
		}


		handleWaitRecvSockets(nfd, &waitRecv);
		handleWaitSendSockets(nfd, &waitSend);
	}

	// Closing connections and Winsock.
	cout << "HTTP Server: Closing Connection.\n";
	closesocket(listenSocket);
	WSACleanup();
}

bool addSocket(SOCKET id, eSocketStatus what)
{
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if (sockets[i].recv == eSocketStatus::EMPTY)
		{
			sockets[i].id = id;
			sockets[i].recv = what;
			sockets[i].send = eSocketStatus::IDLE;
			sockets[i].dataLen = 0;
			sockets[i].requestTime = time(0);
			socketsCount++;
			return (true);
		}
	}
	return (false);
}

void removeSocket(int index)
{
	cout << "The socket number " << index << " has been removed" << endl;
	sockets[index].recv = eSocketStatus::EMPTY;
	sockets[index].send = eSocketStatus::EMPTY;
	sockets[index].dataLen = 0;
	sockets[index].requestTime = 0;
	socketsCount--;
}

void acceptConnection(int index)
{
	SOCKET id = sockets[index].id;
	sockets[index].requestTime = time(0);
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

	if (addSocket(msgSocket, eSocketStatus::RECEIVE) == false)
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

	int len = sockets[index].dataLen;
	int bytesRecv = recv(msgSocket, &sockets[index].data[len], sizeof(sockets[index].data) - len, 0);

	if (bytesRecv == SOCKET_ERROR || bytesRecv == 0)
	{
		if (bytesRecv == SOCKET_ERROR)
		{
			cout << "HTTP Server: Error at recv(): " << WSAGetLastError() << endl;
		}
		closesocket(msgSocket);
		removeSocket(index);
		return;
	}
	sockets[index].requestTime = time(0);

	sockets[index].data[len + bytesRecv] = '\0';
	cout << "HTTP Server: Recieved: " << bytesRecv << " bytes of \"" << &sockets[index].data[len] << "\n---message---\n";
	sockets[index].dataLen += bytesRecv;

	if (sockets[index].dataLen > 0)
	{	
		eRequestType httpRequest = extractRequestFromData(&(sockets[index]));
		if (httpRequest == eRequestType::UNKNOWN)
		{
			cout << "HTTP Server: server does not support this http request" << endl;
			closesocket(msgSocket);
			removeSocket(index);
			return;
		}

		updateSocketRequestAndData(&(sockets[index]), httpRequest);
		return;
	}
}

// TODO: REFACTOR //
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
	sockets[index].requestTime = time(0);
	switch (sockets[index].httpRequest)
	{
		case eRequestType::HEAD:
			sendHeadResponse(sockets[index].data, &sendBuff);
			break;
		case eRequestType::GET:
			sendGetResponse(sockets[index].data, &sendBuff);
			break;
		case eRequestType::PUT:
			sendPutResponse(sockets[index].data, &sendBuff);
			break;
		case eRequestType::_DELETE:
			sendDeleteRepsonse(sockets[index].data, &sendBuff);
			break;
		case eRequestType::TRACE:
			sendTraceResponse(sockets[index].data, &sendBuff);			
			break;
		case eRequestType::OPTIONS:
				sendOptionsResponse(sockets[index].data, &sendBuff);
				break;
		case eRequestType::POST:
				sendPostResponse(sockets[index].data, &sendBuff);
				break;
	}

	buffLen = sendBuff.size();
	bytesSent = send(msgSocket, sendBuff.c_str(), buffLen, 0);
	memset(sockets[index].data, 0, BUFF_SIZE);
	sockets[index].dataLen = 0;
	if (SOCKET_ERROR == bytesSent)
	{
		cout << "HTTP Server: Error at S_SEND(): " << WSAGetLastError() << endl;
		return false;
	}

	cout << "HTTP Server: Sent: " << bytesSent << "\\" << buffLen << " bytes of\n \"" << sendBuff << "\n---message---\n";
	sockets[index].send = eSocketStatus::IDLE;
	return true;
}


WSAData initWsaData()
{
	WSAData wsaData;

	// Call WSAStartup and return its value as an integer and check for errors.
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		cout << "HTTP Server: Error at WSAStartup()\n";
		exit(ERROR);
	}
	return wsaData;
}

SOCKET initSocket()
{
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Check for errors to ensure that the socket is a valid socket.
	if (INVALID_SOCKET == listenSocket)
	{
		cout << "HTTP Server: Error at socket(): " << WSAGetLastError() << endl;
		WSACleanup();
		exit(ERROR);
	}
	return listenSocket;
}


void bindSocketForClientRequests(SOCKET* listenSocket, sockaddr_in* serverService)
{
	if (SOCKET_ERROR == bind(*listenSocket, (SOCKADDR*)serverService, sizeof(*serverService)))
	{
		cout << "HTTP Server: Error at bind(): " << WSAGetLastError() << endl;
		closesocket(*listenSocket);
		WSACleanup();
		exit(ERROR);
	}
}

void listenOnSocketForIncomingConnection(SOCKET* listenSocket)
{
	if (SOCKET_ERROR == listen(*listenSocket, 50))
	{
		cout << "HTTP Server: Error at LISTEN(): " << WSAGetLastError() << endl;
		closesocket(*listenSocket);
		WSACleanup();
		exit(ERROR);
	}
}

sockaddr_in createSocketAddr()
{
	sockaddr_in serverService;
	serverService.sin_family = AF_INET;
	serverService.sin_addr.s_addr = INADDR_ANY;
	serverService.sin_port = htons(PORT);
	return serverService;
}

void updateSocketRequestAndData(SocketState* socket, eRequestType request)
{
	int dataindex = strlen(httpRequests[(int)request]) + 2;
	socket->send = eSocketStatus::SEND;
	socket->httpRequest = request;
	strcpy(socket->data, &(socket->data[dataindex]));
	socket->dataLen = strlen(socket->data);
	socket->data[socket->dataLen] = NULL;
}


eRequestType extractRequestFromData(SocketState* socket)
{	
	eRequestType result = eRequestType::UNKNOWN;
	if (strncmp(socket->data, httpRequests[(int)eRequestType::GET], strlen(httpRequests[(int)eRequestType::GET])) == 0)
	{
		result = eRequestType::GET;
	}
	else if (strncmp(socket->data, httpRequests[(int)eRequestType::HEAD], strlen(httpRequests[(int)eRequestType::HEAD])) == 0)
	{
		result = eRequestType::HEAD;
	}
	else if (strncmp(socket->data, httpRequests[(int)eRequestType::POST], strlen(httpRequests[(int)eRequestType::POST])) == 0)
	{
		result = eRequestType::POST;
	}
	else if (strncmp(socket->data, httpRequests[(int)eRequestType::PUT], strlen(httpRequests[(int)eRequestType::PUT])) == 0)
	{
		result = eRequestType::PUT;
	}
	else if (strncmp(socket->data, httpRequests[(int)eRequestType::_DELETE], strlen(httpRequests[(int)eRequestType::_DELETE])) == 0)
	{
		result = eRequestType::_DELETE;
	}
	else if (strncmp(socket->data, httpRequests[(int)eRequestType::TRACE], strlen(httpRequests[(int)eRequestType::TRACE])) == 0)
	{
		result = eRequestType::TRACE;
	}
	else if (strncmp(socket->data, httpRequests[(int)eRequestType::OPTIONS], strlen(httpRequests[(int)eRequestType::OPTIONS])) == 0)
	{
		result = eRequestType::OPTIONS;
	}

	return result;
}

void deleteStuckRequests()
{
	time_t currentTime;
	for (int i = 1; i < MAX_SOCKETS; i++)
	{
		currentTime = time(0);
		if ((currentTime - sockets[i].requestTime > 120) && (sockets[i].requestTime != 0))
		{
			removeSocket(i);
		}
	}

}


fd_set initWaitRecvSockets()
{
	fd_set waitRecv;
	FD_ZERO(&waitRecv);
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if ((sockets[i].recv == eSocketStatus::LISTEN) || (sockets[i].recv == eSocketStatus::RECEIVE))
			FD_SET(sockets[i].id, &waitRecv);
	}
	return waitRecv;
}


fd_set initWaitSendSockets()
{
	fd_set waitSend;
	FD_ZERO(&waitSend);

	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if (sockets[i].send == eSocketStatus::SEND)
			FD_SET(sockets[i].id, &waitSend);
	}
	return waitSend;
}


void handleWaitRecvSockets(int nfd, fd_set* waitRecv)
{
	for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
	{
		if (FD_ISSET(sockets[i].id, waitRecv))
		{
			nfd--;
			switch (sockets[i].recv)
			{
			case eSocketStatus::LISTEN:
				acceptConnection(i);
				break;

			case eSocketStatus::RECEIVE:
				receiveMessage(i);
				break;
			}
		}
	}
}
void handleWaitSendSockets(int nfd, fd_set* waitSend)
{
	for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
	{
		if (FD_ISSET(sockets[i].id, waitSend))
		{
			nfd--;
			switch (sockets[i].send)
			{
			case eSocketStatus::SEND:
				if (!sendMessage(i))
					continue;
				break;
			}
		}
	}
}



