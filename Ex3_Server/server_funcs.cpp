#include "server.h"
ifstream inFile;

bool sendGetResponse(char* dataRequest, string* sendBuff) {
	string httpStatus;
	string sFullMessage, tmpStringBuff;
	char readBuff[512];
	int buffLen = 0, intFsize = 0;
	string strFsize;
	char ctmp[20];
	time_t rawtime;
	time(&rawtime);
	string folderPath("C:/temp/");


	char tmpbuff[BUFF_SIZE];
	memset(tmpbuff, 0, BUFF_SIZE);
	string lang, filename;
	string uri(strtok(dataRequest, " "));
	setGetParams(uri, &filename, &lang);

	inFile.open(folderPath+filename);
	if (inFile.is_open() == false)
	{
		inFile.open(folderPath + "not_found.html");
		if (inFile.is_open() == false)
		{
			cout << "HTTP Server: Error at S_SEND(): " << WSAGetLastError() << endl;
			return false;
		}
		httpStatus = "404 Not Found";
	}
	else
	{
		httpStatus = "200 OK";
	}


	sFullMessage = tmpStringBuff = tmpbuff;
	while (inFile.getline(readBuff, 512))
	{
		tmpStringBuff += readBuff;
		intFsize += strlen(readBuff);
	}

	strFsize = _itoa(intFsize, ctmp, 10);
	sFullMessage = "HTTP/1.1 " + httpStatus + " \r\nContent-type: text/html\r\nDate: ";
	sFullMessage += ctime(&rawtime);
	sFullMessage += "Content-length: ";
	sFullMessage += strFsize;
	sFullMessage += "\r\n\r\n";

	sFullMessage += tmpStringBuff;

	*sendBuff = sFullMessage;
	inFile.close();
	return true;
}

void setGetParams(string uri, string* filename, string* lang) {
	const int idx = uri.find("?");
	if (idx != -1)
	{
		*filename = uri.substr(0, idx);
		const int idx2 = uri.substr(idx).find("lang=");
		if (idx2 != -1)
		{
			*lang = uri.substr(idx2 + idx + 5);
			const int dotIndex = (*filename).find('.');
			string name = (*filename).substr(0, dotIndex);
			string end = (*filename).substr(dotIndex);
			*filename = name + "-" + *lang + end;
		}
	}
	else
	{
		*filename = uri;
	}
}


bool sendHeadResponse(char* dataRequest, string* sendBuff) {
	string lang, filename;
	string uri(strtok(dataRequest, " "));
	string folderPath("C:/temp/");

	setGetParams(uri, &filename, &lang);
	string sFullMessage;
	time_t rawtime;
	time(&rawtime);

	inFile.open(folderPath+filename);
	if (!inFile.is_open())
	{
		cout << "HTTP Server: Error at SEND(): " << WSAGetLastError() << endl;
		return false;
	}
	else
	{
		sFullMessage = "HTTP/1.1 200 OK\r\nDate: ";

		sFullMessage += ctime(&rawtime);
		sFullMessage += "\r\nContent-type: text/html";
		sFullMessage += "\r\n\r\n";
		sFullMessage += "    ";
		*sendBuff = sFullMessage;
		inFile.close();
	}
}

void PrintPostToConsole(char* dataRequest, string* sendBuff) {

	string sFullMessage;
	time_t rawtime;
	time(&rawtime);	
	string msg(dataRequest);	
	int contIdx = msg.find("Content-Length:");
	contIdx += msg.substr(contIdx).find("\n");
	msg = msg.substr(contIdx);
	
	cout << "HTTP Server: Got Post Request:\n"<< msg << endl;
		
	
	sFullMessage = "HTTP/1.1 200 OK\r\nDate: ";

	sFullMessage += ctime(&rawtime);
	sFullMessage += "\r\nContent-type: text/html";
	sFullMessage += "\r\n\r\n";
	sFullMessage += "    ";
	*sendBuff = sFullMessage;
	inFile.close();	
}

bool sendOptionsResponse(char* dataRequest, string* sendBuff)
{

	string httpStatus;
	string sFullMessage, tmpStringBuff;
	char readBuff[512];
	int buffLen = 0, intFsize = 0;
	string strFsize;
	char ctmp[20];
	time_t rawtime;
	time(&rawtime);
	string folderPath("C:/temp/");


	char tmpbuff[BUFF_SIZE];
	memset(tmpbuff, 0, BUFF_SIZE);
	string filename = "options.html";	

	inFile.open(folderPath + filename);
	if (inFile.is_open() == false)
	{		
			cout << "HTTP Server: Error at S_SEND(): " << WSAGetLastError() << endl;
			return false;
		
	}
	else
	{
		httpStatus = "204 No Content";
	}


	sFullMessage = tmpStringBuff = tmpbuff;
	while (inFile.getline(readBuff, 512))
	{
		tmpStringBuff += readBuff;
		intFsize += strlen(readBuff);
	}

	strFsize = _itoa(intFsize, ctmp, 10);
//	sFullMessage = "HTTP/1.1 " + httpStatus + " \r\nContent-type: text/html\r\nDate: ";
//	sFullMessage += ctime(&rawtime);
//	sFullMessage += "Content-length: ";
//	sFullMessage += strFsize;
	//sFullMessage += "\r\n\r\n";

	//sFullMessage += tmpStringBuff;
	sFullMessage = "HTTP/1.1 " + httpStatus + " \r\n";
	sFullMessage += "Allow: OPTIONS, GET, HEAD, POST\r\nDate: ";
	sFullMessage += ctime(&rawtime);
	sFullMessage += "Content-length: ";
	strFsize = _itoa(intFsize, ctmp, 10);
	sFullMessage += strFsize;
	sFullMessage += "\r\n\r\n";
	*sendBuff = sFullMessage;
	inFile.close();
	return true;
}

bool sendDeleteRepsonse(char* dataRequest, string* sendBuff) {
		string strFsize;
		time_t rawtime;
		time(&rawtime);
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
		sFullMessage += ctime(&rawtime);
		sFullMessage += "Content-length: ";
		strFsize = _itoa(intFsize, ctmp, 10);
		sFullMessage += strFsize;
		sFullMessage += "\r\n\r\n";
		*sendBuff = sFullMessage;
		return true;
}

bool sendPutResponse(char* dataRequest, string* sendBuff) {
	string strFsize;
	time_t rawtime;
	time(&rawtime);
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
	sFullMessage += ctime(&rawtime);
	sFullMessage += "Content-length: ";
	strFsize = _itoa(intFsize, ctmp, 10);
	sFullMessage += strFsize;
	sFullMessage += "\r\n\r\n";

	*sendBuff = sFullMessage;
	return true;
}
	
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



