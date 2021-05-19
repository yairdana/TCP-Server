#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
using namespace std;
#include <winsock2.h>
#include <string.h>
#include <sys/stat.h> 
#include <time.h>
#include <string>
#include <fstream>
#define BUFF_SIZE 1024

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
using namespace std;
#include <winsock2.h>
#include <string.h>
#include <sys/stat.h> 
#include <time.h>
#include <string>
#include <fstream>

#pragma comment(lib, "Ws2_32.lib")


const int PORT = 8000;
const int MAX_SOCKETS = 60;



#pragma comment(lib, "Ws2_32.lib")
bool sendGetResponse(char* dataRequest, string* sendBuff);
void setGetParams(string uri, string* filename, string* lang);
bool sendHeadResponse(char* dataRequest, string* sendBuff);
void PrintPostToConsole(char* dataRequest, string* sendBuff);
bool sendOptionsResponse(char* dataRequest, string* sendBuff);
bool sendDeleteRepsonse(char* dataRequest, string* sendBuff);
bool sendPutResponse(char* dataRequest, string* sendBuff);
int createOrOverwriteFile(string data, string filename);
