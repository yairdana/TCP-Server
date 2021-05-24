#include "server.h";

ifstream inFile;

// ==== GET request ==== //
void sendGetResponse(char* dataRequest, string* sendBuff) {
	string httpStatus, response, fileData, stringFileSize, lang, filename;
	string uri(strtok(dataRequest, " "));
	char readBuff[512] = { 0 };
	int fileSize = 0;

	// get query params and initial filename and lang
	setGetParams(uri, &filename, &lang);

	// openfile and determine httpstatus
	bool ret = openFile(eRequestType::GET, &inFile, &filename, &httpStatus);

	// copy file data
	while (ret && inFile.getline(readBuff, 512))
	{
		fileData += readBuff;
	}

	stringFileSize = convertFileLengthToString(fileData.length());
	response = buildResponse(eRequestType::GET, &httpStatus, &stringFileSize, &fileData);
	*sendBuff = response;
	closeFile(&inFile);
}

// ==== HEAD request ==== //
void sendHeadResponse(char* dataRequest, string* sendBuff) {
	string lang, filename, httpStatus;
	string uri(strtok(dataRequest, " "));

	// get query params and initial filename and lang
	setGetParams(uri, &filename, &lang);

	// openfile and determine httpstatus
	bool ret = openFile(eRequestType::HEAD, &inFile, &filename, &httpStatus);
	string body = "";
	string bodySize = "0";

	string response = buildResponse(eRequestType::HEAD, &httpStatus, &bodySize, &body);
	*sendBuff = response;
	closeFile(&inFile);
}

// ==== POST request==== //
void sendPostResponse(char* dataRequest, string* sendBuff)
{
	string body = extractBodyFromReq(dataRequest);

	cout << "HTTP Server: Got Post Request:\n";
	cout << "------------------------------\n" << body << endl << endl;
	string httpStatus = "200 OK";
	string bodySize = convertFileLengthToString(body.length());

	string response = buildResponse(eRequestType::POST, &httpStatus, &bodySize, &body);
	*sendBuff = response;
	inFile.close();
}

// ==== OPTION request ==== //
void sendOptionsResponse(char* dataRequest, string* sendBuff)
{
	string httpStatus = "204 No Content";
	string response = buildResponse(eRequestType::OPTIONS, &httpStatus, NULL, NULL);
	*sendBuff = response;
}

// ==== DELETE request ==== // 
void sendDeleteRepsonse(char* dataRequest, string* sendBuff)
{
	string httpStatus;
	string fileToRemove = folderPath + strtok(dataRequest, " ");

	bool ret = removeFile(fileToRemove, &httpStatus);
	string body = ret ? "File deleted." : "Cannot delete file.";
	string bodySize = convertFileLengthToString(body.length());

	string response = buildResponse(eRequestType::_DELETE, &httpStatus, &bodySize, &body);
	*sendBuff = response;
}

// ==== PUT request ==== // 
void sendPutResponse(char* dataRequest, string* sendBuff) {
	string strFsize = "0";
	string filename, httpStatus;

	string mydata = extractBodyFromReq(dataRequest);
	string body = "";

	string copy(dataRequest);
	string uri(strtok((char*)copy.c_str(), " "));

	setGetParams(uri, &filename, NULL);

	filename = folderPath + filename;
	int retCode = createOrOverwriteFile(mydata, filename);
	switch (retCode)
	{
	case 0:
		cout << "PUT " << filename << "Failed";
		httpStatus = "412 Precondition failed";
		break;
	case 200:
		httpStatus = "200 OK";
		break;
	case 201:
		httpStatus = "201 Created";
		break;
	case 204:
		httpStatus = "204 No Content";
		break;
	default:
		httpStatus = "501 Not Implemented";
		break;
	}

	string response = buildResponse(eRequestType::TRACE, &httpStatus, &strFsize, &body);
	*sendBuff = response;
}

// ==== TRACE request ==== // 
void sendTraceResponse(char* dataRequest, string* sendBuff)
{
	string bodySize, httpStatus = "200 OK";
	string body = "TRACE /";
	body += dataRequest;
	bodySize = convertFileLengthToString(body.length());

	string response = buildResponse(eRequestType::_DELETE, &httpStatus, &bodySize, &body);
	*sendBuff = response;
}
