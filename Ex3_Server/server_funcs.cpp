#include "server.h"

ifstream inFile;
string folderPath("C:/temp/");
time_t currentTime;

// ==== GET ==== //
void sendGetResponse(char* dataRequest, string* sendBuff) {
	string httpStatus, response, fileData, stringFileSize, lang, filename;
	string uri(strtok(dataRequest, " "));
	char readBuff[512] = { 0 }, ctmp[20];
	int buffLen = 0, fileSize = 0;
	
	// get query params and initial filename and lang
	setGetParams(uri, &filename, &lang);

	// openfile and determine httpstatus
	bool ret = openFile(eRequestType::GET, &inFile, &filename, &httpStatus);

	// copy file data
	while (ret && inFile.getline(readBuff, 512))
	{
		fileData += readBuff;
		fileSize += strlen(readBuff);
	}

	stringFileSize = _itoa(fileSize, ctmp, 10);
	response = buildResponse(eRequestType::GET, &httpStatus, &stringFileSize, &fileData);
	*sendBuff = response;
	closeFile(&inFile);
}


// ==== HEAD ==== //
void sendHeadResponse(char* dataRequest, string* sendBuff) {
	string lang, filename, response, httpStatus;
	string uri(strtok(dataRequest, " "));

	// get query params and initial filename and lang
	setGetParams(uri, &filename, &lang);

	// openfile and determine httpstatus
	bool ret = openFile(eRequestType::HEAD, &inFile, &filename, &httpStatus);

	response = buildResponse(eRequestType::HEAD, &httpStatus, NULL, NULL);
	*sendBuff = response;
	closeFile(&inFile);
}

// ==== POST ==== //
void sendPostResponse(char* dataRequest, string* sendBuff)
{
	string response;
	string body = extractBodyFromReq(dataRequest);
	char ctmp[20];

	cout << "HTTP Server: Got Post Request:\n"<< body << endl;
	string httpStatus = "200 OK";

	int intBodySize = body.length(); 
	string bodySize = _itoa(intBodySize, ctmp, 10);

	response = buildResponse(eRequestType::POST, &httpStatus, &bodySize, &body);
	*sendBuff = response;
	inFile.close();	
}

// ==== OPTION ==== //
void sendOptionsResponse(char* dataRequest, string* sendBuff)
{
	string httpStatus = "204 No Content";
	string response = buildResponse(eRequestType::OPTIONS, &httpStatus, NULL, NULL);
	*sendBuff = response;
}

bool sendDeleteRepsonse(char* dataRequest, string* sendBuff) {
		string strFsize;
		time_t currentTime;
		time(&currentTime);
		char ctmp[20];
		int intFsize = 0;
		string folderPath("C:/temp/");

		string sFullMessage;	
		string fileToRemove =folderPath + strtok(dataRequest, " ");
		string httpStatus = "204 No Content";
		if (remove(fileToRemove.c_str()) != NULL) {
			ifstream file(fileToRemove);
			if(!file)
			{
				httpStatus = "400 Bad Request";
			}
			else 
			{
				httpStatus = "500 Internal Server Error";
				file.close();
			}
	
			cout << "Couldn't remove " << fileToRemove << endl; // the server do not indicate the client whether the operation was succesfull or not but prints out a faliure for debugging.
		}
		sFullMessage = "HTTP/1.1 "+ httpStatus +"\r\nDate: ";
		sFullMessage += ctime(&currentTime);
		sFullMessage += "Content-length: ";
		strFsize = _itoa(intFsize, ctmp, 10);
		sFullMessage += strFsize;
		sFullMessage += "\r\n\r\n";
		*sendBuff = sFullMessage;
		return true;
}

bool sendPutResponse(char* dataRequest, string* sendBuff) {
	string strFsize;
	time_t currentTime;
	time(&currentTime);
	char ctmp[20];
	int intFsize = 0;
	string folderPath("C:/temp/");
	string filename,sFullMessage;

	string copy (dataRequest);
	
	string uri(strtok((char*)copy.c_str(), " "));
	

	setGetParams(uri, &filename, NULL);

	string data(dataRequest);
	int Idx = data.find("Content-Length:");
	Idx += data.substr(Idx).find("\n");
	data = data.substr(Idx);

	//string datarequest(dataRequest);
	//string filename = datarequest.substr(0, datarequest.find(" "));
	//string data = datarequest.substr(datarequest.find(" "));

	int retCode = createOrOverwriteFile(data, filename);
	switch (retCode)
	{
	case 0:
		cout << "PUT " << filename << "Failed";
		sFullMessage = "HTTP/1.1 412 Precondition failed \r\nDate: ";
		break;
	case 200:
		sFullMessage = "HTTP/1.1 200 OK \r\nDate: ";
		break;
	case 201:
		sFullMessage = "HTTP/1.1 201 Created \r\nDate: ";
		break;
	case 204:
		sFullMessage = "HTTP/1.1 204 No Content \r\nDate: ";
		break;
	default:
		sFullMessage = "HTTP/1.1 501 Not Implemented \r\nDate: ";

		break;
	}
	sFullMessage += ctime(&currentTime);
	sFullMessage += "Content-length: ";
	strFsize = _itoa(intFsize, ctmp, 10);
	sFullMessage += strFsize;
	sFullMessage += "\r\n\r\n";

	*sendBuff = sFullMessage;
	return true;
}


// ====== AUXILIRY FUNCTIONS ======= ///
	
int createOrOverwriteFile(string data, string filename)
{
	string tmp;
	char* tmp1 = 0;
	int buffLen = 0;
	int retCode = 200;//ok

//	tmp = strtok(NULL, ":");
//	tmp = strtok(NULL, ":");
//	tmp = data.substr(data.find("Content-Length:"));
//	sscanf(tmp1, "%d", &buffLen);
	ofstream outPutFile;
	outPutFile.open(filename, ios::in);

	if (!outPutFile.is_open())
	{
		outPutFile.open(filename, ios::trunc);
		retCode = 201;//created
	}

	if (!outPutFile.is_open())
	{
		cout << "HTTP Server: Error writing file to local storage: " << WSAGetLastError() << endl;
		return 0;//error
	}
	tmp1 = strtok(NULL, "\r\n\r\n"); //tmp1 = strtok(data, "\r\n\r\n")

	if (tmp1 == 0)
	{
		retCode = 204; //no content
	}
	else
	{
		while (*tmp1 != 0)
		{
			outPutFile << tmp1;
			tmp1 += (strlen(tmp1) + 1); 
		}
	}
	outPutFile.close();
	return retCode;
}



bool openFile(eRequestType request, ifstream* inFile, string* filename, string* httpStatus)
{

	inFile->open(folderPath + *filename);
	if (inFile->is_open())
	{
		*httpStatus = "200 OK";
		return true;
	}

	if (request == eRequestType::GET) {
		inFile->open(folderPath + "not_found.html");
		if (inFile->is_open())
		{
			*httpStatus = "404 Not Found";
			return true;
		}
	}

	if (request == eRequestType::HEAD) 
	{
		*httpStatus = "400 Bad Request";
		return true;
	}


	cout << "HTTP Server: Error at S_SEND(): " << WSAGetLastError() << endl;
	*httpStatus = "500 Internal Server Error";
	return false;
}


string buildResponse(eRequestType httpRequest, string* httpStatus, string* strFsize, string* fileData)
{
	string response;
	time(&currentTime);

	response = "HTTP/1.1 " + *httpStatus + " \r\n";

	response += "Date: ";
	response += ctime(&currentTime);
	if (httpRequest == eRequestType::OPTIONS)
	{
		response += "Allow: OPTIONS, GET, HEAD, POST, PUT, DELETE, TRACE\r\n";
		response += "Content-type: text/html\r\n";
		response += "Content-length: 0";
		response += "\r\n\r\n";
	}

	if (httpRequest == eRequestType::GET || httpRequest == eRequestType::POST)
	{
		response += "Content-type: text/html\r\n";
		response += "Content-length: ";
		response += *strFsize;
		response += "\r\n\r\n";
		response += *fileData;
	}
	return response;
}

void closeFile(ifstream* inFile)
{
	if (inFile->is_open())
	{
		inFile->close();
	}
}

string extractBodyFromReq(char* dataRequest)
{
	string body(dataRequest);
	int contIdx = body.find("Content-Length:");
	contIdx += body.substr(contIdx).find("\n");
	body = body.substr(contIdx);
	trim(body);
	return body;
}


void setGetParams(string uri, string* filename, string* lang) {
	const int idx = uri.find("?");
	if (idx == -1)
	{
		*filename = uri;
		return;
	}

	*filename = uri.substr(0, idx);
	const int idx2 = uri.substr(idx).find("lang=");
	if (idx2 == -1)
	{
		return;
	}

	*lang = uri.substr(idx2 + idx + 5);
	const int dotIndex = (*filename).find('.');
	string name = (*filename).substr(0, dotIndex);
	string end = (*filename).substr(dotIndex);
	*filename = name + "-" + *lang + end;

}



// trim from start (in place)
static inline void ltrim(std::string& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
		return !std::isspace(ch);
	}));
}

// trim from end (in place)
static inline void rtrim(string& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
	}).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(string& s) {
	ltrim(s);
	rtrim(s);
}