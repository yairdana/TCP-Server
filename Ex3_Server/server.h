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

const int PORT = 8000;
const int MAX_SOCKETS = 60;

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


// =====  Main Functions ==== //
bool addSocket(SOCKET id, enStatusList what);
void removeSocket(int index);
void acceptConnection(int index);
void receiveMessage(int index);
bool sendMessage(int index);

// =====  Aux Functions ==== //
bool sendGetResponse(char* dataRequest, string* sendBuff);
void setGetParams(string uri, string* filename, string* lang);
bool sendHeadResponse(char* dataRequest, string* sendBuff);
void PrintPostToConsole(char* dataRequest, string* sendBuff);
bool sendOptionsResponse(char* dataRequest, string* sendBuff);
bool sendDeleteRepsonse(char* dataRequest, string* sendBuff);
bool sendPutResponse(char* dataRequest, string* sendBuff);
int createOrOverwriteFile(string data, string filename);


