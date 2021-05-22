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


// =====  Main Functions ==== //
bool addSocket(SOCKET id, eSocketStatus what);
void removeSocket(int index);
void acceptConnection(int index);
void receiveMessage(int index);
bool sendMessage(int index);
WSAData initWsaData();
SOCKET initSocket();
sockaddr_in createSocketAddr();
void bindSocketForClientRequests(SOCKET* listenSocket, sockaddr_in* serverService);
void listenOnSocketForIncomingConnection(SOCKET* listenSocket);
void updateSocketRequestAndData(SocketState* socket, eRequestType request);
eRequestType extractRequestFromData(SocketState* socket);
void deleteStuckRequests();

fd_set initWaitRecvSockets();
fd_set initWaitSendSockets();

void handleWaitRecvSockets(int nfd, fd_set* waitRecv);
void handleWaitSendSockets(int nfd, fd_set* waitSend);
// =====  Aux Functions ==== //
void closeFile(ifstream* inFile);
bool openFile(eRequestType request, ifstream* inFile, string* filename, string* httpStatus);
string buildResponse(eRequestType httpRequest, string* httpStatus, string* strFsize, string* fileData);
void sendGetResponse(char* dataRequest, string* sendBuff);
void setGetParams(string uri, string* filename, string* lang);

string extractBodyFromReq(char* dataRequest);

void sendHeadResponse(char* dataRequest, string* sendBuff);
void sendPostResponse(char* dataRequest, string* sendBuff);
void sendOptionsResponse(char* dataRequest, string* sendBuff);
void sendDeleteRepsonse(char* dataRequest, string* sendBuff);
bool sendPutResponse(char* dataRequest, string* sendBuff);
int createOrOverwriteFile(string data, string filename);

bool removeFile(string fileToRemove, string* httpStatus);

static inline void ltrim(string& s);
static inline void rtrim(string & s);
static inline void trim(string & s);
