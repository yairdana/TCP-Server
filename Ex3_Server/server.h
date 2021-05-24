#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma comment(lib, "Ws2_32.lib")

#include <iostream>
using namespace std;
#include <winsock2.h>
#include <string.h>
#include <sys/stat.h> 
#include <time.h>
#include <string>
#include <fstream>
#include <regex>

#define BUFF_SIZE 1024
#define ERROR -1
const int PORT = 8000;
const int MAX_SOCKETS = 60;

enum class eSocketStatus {
	EMPTY = 0,
	LISTEN,
	RECEIVE,
	IDLE,
	SEND,
};

enum class eRequestType {
	UNKNOWN = -1,
	GET = 0,
	HEAD,
	POST,
	PUT,
	_DELETE,
	TRACE,
	OPTIONS,
};


struct SocketState
{
	SOCKET id;
	eSocketStatus recv;
	eSocketStatus send;
	eRequestType httpRequest;
	time_t requestTime;
	char data[BUFF_SIZE];
	int dataLen;
};

static const char* httpRequests[] =
{
	"GET",
	"HEAD",
	"POST",
	"PUT",
	"DELETE",
	"TRACE",
	"OPTIONS"
};

static string folderPath("C:/temp/");

// =====  Main Functions ==== //
WSAData initWsaData();
SOCKET initSocket();
sockaddr_in createSocketAddr();
void bindSocketForClientRequests(SOCKET* listenSocket, sockaddr_in* serverService);
void listenOnSocketForIncomingConnection(SOCKET* listenSocket);
bool addSocket(SocketState* sockets, int* socketsCount, SOCKET id, eSocketStatus what);
void removeSocket(SocketState* sockets, int* socketsCount, int index);
void acceptConnection(SocketState* sockets, int* socketsCount, int index);
void receiveMessage(SocketState* sockets, int* socketsCount, int index);
bool sendMessage(SocketState* sockets, int* socketsCount, int index);
void updateSocketRequestAndData(SocketState* socket, eRequestType request);
void deleteStuckRequests(SocketState* sockets, int* socketsCount);
fd_set initWaitRecvSockets(SocketState* sockets);
fd_set initWaitSendSockets(SocketState* sockets);
void handleWaitRecvSockets(SocketState* sockets, int* socketsCount, int nfd, fd_set* waitRecv);
void handleWaitSendSockets(SocketState* sockets, int* socketsCount, int nfd, fd_set* waitSend);

// =====  Requests Functions ==== //
void sendGetResponse(char* dataRequest, string* sendBuff);
void sendHeadResponse(char* dataRequest, string* sendBuff);
void sendPostResponse(char* dataRequest, string* sendBuff);
void sendOptionsResponse(char* dataRequest, string* sendBuff);
void sendDeleteRepsonse(char* dataRequest, string* sendBuff);
void sendPutResponse(char* dataRequest, string* sendBuff);
void sendTraceResponse(char* dataRequest, string* sendBuff);

// =====  Utils ==== //
void closeFile(ifstream* inFile);
bool openFile(eRequestType request, ifstream* inFile, string* filename, string* httpStatus);
bool removeFile(string fileToRemove, string* httpStatus);
eRequestType extractRequestFromData(SocketState* socket);
string extractBodyFromReq(char* dataRequest);
string convertFileLengthToString(int fileSize);
string buildResponse(eRequestType httpRequest, string* httpStatus, string* strFsize, string* fileData);
void setGetParams(string uri, string* filename, string* lang);
int createOrOverwriteFile(string data, string filename);
static inline void ltrim(string& s);
static inline void rtrim(string & s);
static inline void trim(string & s);
