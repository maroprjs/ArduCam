/*
 * WebGui.cpp
 *
 *  Created on: 09.05.2017
 *      Author: maro
 */

#include "WebGui.h"
#include "Debug.h"
#include "htmlEmbed.h"
#include<algorithm> //needed for std:for_each(..); ref: http://stackoverflow.com/questions/30706652/for-each-is-not-a-member-of-std-c11-support-already-enabled



WebGui::WebGui(ESP8266WebServer *server, Storage *storage) {
	_server = server;
	_storage = storage;

}
//WebGui::WebGui() {

//}

WebGui::~WebGui() {
	// TODO Auto-generated destructor stub
}

void WebGui::begin() {
	SHOWSPIFFS;//TODO: include spiffs.begin also in non debug modus
	//from: https://community.platformio.org/t/esp8266webserver-inside-custom-class/237
	_server->on("/",std::bind(&WebGui::handleRoot, this));
	_server->on("/favicon.ico", [this]() { _server->send(200, "text/html", "");   });
	_server->on("/browse", std::bind(&WebGui::handleFileBrowser, this));
	_server->on("/upload", HTTP_POST, [this]() { _server->send(200, "text/plain", ""); }, std::bind(&WebGui::handleFileUpload, this));
	_server->onNotFound([this](){
		if(!handleFileRead(_server->uri()))
			_server->send(404, "text/plain", "FileNotFound");
	});
	_server->begin();
	PRINTLN("HTTP server started");

}

void WebGui::handleRoot(){//handles root of website (used in case of virgin systems.)

	  if (!handleFileRead("/")) {   //if new system without index we either show wifisetup or if already setup/connected we show filebrowser for config.
		  PRINTLN("handlefileRead() negativ");
		  //if (isAdmin()) {
	      if (WiFi.status() != WL_CONNECTED) {
	       //TODO: implement handleWifiConfig();
	      }
	      else { handleFileBrowser(); }
	    //}
	  }
	 //use indexhtml or use embedded wifi setup...
}

void WebGui::handleFileBrowser(){
	  //if (isAdmin() == false) return;
	  if (_server->arg("do") == "list") {
	    handleFileList();
	  }
	  else
	    if (_server->arg("do") == "delete") {
	      handleFileDelete(_server->arg("file"));
	    }
	    else
	      if (_server->arg("do") == "download") {
	        handleFileDownload(_server->arg("file"));
	      }
	      else
	      {
	        if (!handleFileRead("/filebrowse.html")) { //send GZ version of embedded browser
	                                _server->sendHeader("Content-Encoding", "gzip");
	                                _server->send_P(200, "text/html", PAGE_FSBROWSE, sizeof(PAGE_FSBROWSE));
	                             }
	        //TODO: implment semaphore ->MyWebServer.isDownloading = true; //need to stop all cloud services from doing anything!  crashes on upload with mqtt...
	      }

}

void WebGui::handleFileList()
{
  //if (isAdmin() == false) return;
  Dir dir = SPIFFS.openDir("/");

  String output = "{\"success\":true, \"is_writable\" : true, \"results\" :[";
  bool firstrec = true;
  while (dir.next()) {
    if (!firstrec) { output += ','; }  //add comma between recs (not first rec)
    else {
      firstrec = false;
    }
    String fn = dir.fileName();
    fn.remove(0, 1); //remove slash
    output += "{\"is_dir\":false";
    output += ",\"name\":\"" + fn;
    output += "\",\"size\":" + String(dir.fileSize());
    output += ",\"path\":\"";
    output += "\",\"is_deleteable\":true";
    output += ",\"is_readable\":true";
    output += ",\"is_writable\":true";
    output += ",\"is_executable\":true";
    output += ",\"mtime\":1452813740";   //dummy time
    output += "}";
  }
  output += "]}";
  //DebugPrintln("got list >"+output);
  _server->send(200, "text/json", output);
}

void WebGui::handleFileDelete(String fname)
{
  //if (isAdmin() == false) return;
  PRINTLN("handleFileDelete: " + fname);
  fname = '/' + fname;
  fname = urldecode(fname);
  if (!SPIFFS.exists(fname))
    return _server->send(404, "text/plain", "FileNotFound");
  if (SPIFFS.exists(fname))
  {
    SPIFFS.remove(fname);
    _server->send(200, "text/plain", "");
  }
}

bool WebGui::handleFileDownload(String fname)
{
  PRINTLN("handleFileDownload: " + fname);
  String contentType = "application/octet-stream";
  fname = "/" + fname;
  fname = urldecode(fname);
  //if (isPublicFile(fname) == false)
  //{
  //  if (isAdmin() == false) return false;
  //}  //check if a public file.

  if (SPIFFS.exists(fname)) {
    File file = SPIFFS.open(fname, "r");
    _server->streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

bool WebGui::handleFileRead(String path){
	  PRINTLN("handleFileRead: " + path);
	  if(path.endsWith("/")) path += "index.html";
	  String contentType = getContentType(path);
	  String pathWithGz = path + ".gz";
	  path = urldecode(path);
	  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
	    if(SPIFFS.exists(pathWithGz))
	      path += ".gz";
	    File file = SPIFFS.open(path, "r");
	    //size_t sent = server.streamFile(file, contentType);
	    _server->streamFile(file, contentType);
	    file.close();
	    return true;
	  }
	  return false;

}

void WebGui::handleFileUpload(){
	  if (_server->uri() != "/upload") return;
	  //if (isAdmin() == false) return;
	  //TODO: implement semaphore MyWebServer.isDownloading = true;
	  HTTPUpload& upload = _server->upload();
	  if (upload.status == UPLOAD_FILE_START) {
	    String filename = upload.filename;
	    if (!filename.startsWith("/")) filename = "/" + filename;
	    PRINTLN("handleFileUpload Name: "); PRINTLN(filename);
	    _fsUploadFile = SPIFFS.open(filename, "w");
	    filename = String();
	  }
	  else if (upload.status == UPLOAD_FILE_WRITE) {
	    //SerialLog.print("handleFileUpload Data: "); DebugPrintln(upload.currentSize);
	    if (_fsUploadFile)
	      _fsUploadFile.write(upload.buf, upload.currentSize);
	  }
	  else if (upload.status == UPLOAD_FILE_END) {
	    if (_fsUploadFile)
	      _fsUploadFile.close();

	    PRINT("handleFileUpload Size: "); PRINTLN(upload.totalSize);
	//    MyWebServer.isDownloading = false;
	  }

}


//reference: http://stackoverflow.com/questions/3257896/c-for-each-calling-a-vector-of-callback-functions-and-passing-each-one-an-argu
//void WebGui::addCustomFunction(customFunction_t cb, String serverPath){
	 //_customFunctionList.push_back(cb);
	// _customServerPathList.push_back(serverPath);
	//_customFunctionRecord.customFunctionList.push_back(cb);
	//_customFunctionRecord.customServerPathList.p
	//_customFunctionRecord.customFunction = cb;
	//_customFunctionRecord.customServerPath = serverPath;
	//_customFunctionRecordVector.push_back(_customFunctionRecord);
	//_server->on((const char*)cfr->customServerPath.c_str(), cfr->customFunction);
	//_server->on((const char*)serverPath.c_str(), cb);

//}

//reference: http://stackoverflow.com/questions/3257896/c-for-each-calling-a-vector-of-callback-functions-and-passing-each-one-an-argu
//void WebGui::handleCustomFunctions() {
	//int i = 0;
//	PRINTLN("in handleCustomFunctions");
//	std::vector<customFunctionRecord_t>::const_iterator cfr;
	//std::for_each(_customFunctionList.begin(), _customFunctionList.end(), std::ref(*this));
//    for(cfr=_customFunctionRecordVector.begin(); cfr!=_customFunctionRecordVector.end(); ++cfr){
    	//_server->on(cfr.customServerPath, std::bind(&WebGui::handleCustomFunctions, this));
 //   	PRINTLN(cfr->customServerPath);
 //   	_server->on((const char*)cfr->customServerPath.c_str(), cfr->customFunction);
 //   }
//}

String WebGui::getContentType(String filename){
  if(_server->hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

String WebGui::urldecode(String input) // (based on https://code.google.com/p/avr-netino/)
{
  char c;
  String ret = "";

  for (byte t = 0; t<input.length(); t++)
  {
    c = input[t];
    if (c == '+') c = ' ';
    if (c == '%') {


      t++;
      c = input[t];
      t++;
      c = (h2int(c) << 4) | h2int(input[t]);
    }

    ret.concat(c);
  }
  yield();//TODO: check if sesnible from https://github.com/zenmanenergy/ESP8266-Arduino-Examples/blob/master/helloWorld_urlencoded/urlencode.ino
  return ret;

}


// convert a single hex digit character to its integer value (from https://code.google.com/p/avr-netino/)
unsigned char WebGui::h2int(char c)
{
  if (c >= '0' && c <= '9') {
    return((unsigned char)c - '0');
  }
  if (c >= 'a' && c <= 'f') {
    return((unsigned char)c - 'a' + 10);
  }
  if (c >= 'A' && c <= 'F') {
    return((unsigned char)c - 'A' + 10);
  }
  return(0);
}









