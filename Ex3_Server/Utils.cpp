#include "server.h";

void closeFile(ifstream* inFile)
{
	if (inFile->is_open())
	{
		inFile->close();
	}
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

bool removeFile(string fileToRemove, string* httpStatus)
{
	if (remove(fileToRemove.c_str()) == NULL)
	{
		*httpStatus = "200 OK";
		return true;
	}

	//check if file exist
	ifstream file(fileToRemove);
	if (!file)
	{
		*httpStatus = "400 Bad Request";
	}
	else
	{
		*httpStatus = "500 Internal Server Error";
		file.close();
	}

	cout << "\nCouldn't remove " << fileToRemove << endl; // the server do not indicate the client whether the operation was succesfull or not but prints out a faliure for debugging.
	return false;
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

string extractBodyFromReq(char* dataRequest)
{
	string body(dataRequest);
	int contIdx = body.find("Content-Length:");
	contIdx += body.substr(contIdx).find("\n");
	body = body.substr(contIdx);
	trim(body);
	return body;
}

string convertFileLengthToString(int fileSize)
{
	char temp[30];
	return _itoa(fileSize, temp, 10);
}

string buildResponse(eRequestType httpRequest, string* httpStatus, string* strFsize, string* fileData)
{
	time_t currentTime;
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

	if (httpRequest == eRequestType::GET || httpRequest == eRequestType::POST || httpRequest == eRequestType::HEAD
		|| httpRequest == eRequestType::_DELETE || httpRequest == eRequestType::TRACE
		|| httpRequest == eRequestType::PUT)
	{
		response += "Content-type: text/html\r\n";
		response += "Content-length: ";
		response += *strFsize;
		response += "\r\n\r\n";
		response += *fileData;
	}
	return response;
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

int createOrOverwriteFile(string data, string filename)
{
	int retCode = 200;//ok

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

	if (data.empty()) //should it write an empty file or not touch existing one ?
	{
		retCode = 204; //no content
	}
	else
	{
		outPutFile << data;
	}
	outPutFile.close();
	return retCode;
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
