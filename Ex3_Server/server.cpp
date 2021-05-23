#include "server.h"


void main()
{
	SocketState sockets[MAX_SOCKETS] = { 0 };
	int socketsCount = 0;

	WSAData wsaData = initWsaData();
	SOCKET listenSocket = initSocket();
	sockaddr_in serverService = createSocketAddr();
	bindSocketForClientRequests(&listenSocket, &serverService);
	listenOnSocketForIncomingConnection(&listenSocket);
	addSocket(sockets, &socketsCount, listenSocket, eSocketStatus::LISTEN);

	cout << "Waiting for client connections..." << endl;
	// Accept connections and handles them one by one.
	while (true)
	{
		deleteStuckRequests(sockets, &socketsCount);
		fd_set waitRecv= initWaitRecvSockets(sockets);
		fd_set waitSend = initWaitSendSockets(sockets);

		// Wait for interesting event.
		int nfd;
		nfd = select(0, &waitRecv, &waitSend, NULL, NULL);
		if (nfd == SOCKET_ERROR)
		{
			cout << "HTTP Server: Error at select(): " << WSAGetLastError() << endl;
			WSACleanup();
			return;
		}


		handleWaitRecvSockets(sockets, &socketsCount, nfd, &waitRecv);
		handleWaitSendSockets(sockets, &socketsCount, nfd, &waitSend);
	}

	// Closing connections and Winsock.
	cout << "HTTP Server: Closing Connection.\n";
	closesocket(listenSocket);
	WSACleanup();
}



