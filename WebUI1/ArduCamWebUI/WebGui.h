/*
 * WebGui.h
 *
 *  Created on: 09.05.2017
 *      Author: maro
 */

#ifndef WEBGUI_H_
#define WEBGUI_H_
#include <vector>//MRO: use this before #include <ESP8266WebServer.h>, otherwise comple error
//(->https://github.com/esp8266/Arduino/issues/2549)
#include <ESP8266WebServer.h>
#include "Storage.h"


class WebGui {
public:

	//typedef std::function<void()> customFunction_t;	//reference: http://stackoverflow.com/questions/3257896/c-for-each-calling-a-vector-of-callback-functions-and-passing-each-one-an-argu
	//typedef std::vector <customFunction_t> customFunctionList_t;
	//typedef std::vector <String> customServerPathList_t;
    //struct CustomFunctionRecord
    //{
	//	customFunction_t  customFunction;
	//    String customServerPath;
    //} ;
    //typedef CustomFunctionRecord customFunctionRecord_t;
	//typedef std::vector <customFunctionRecord_t> customFunctionRecordVector_t;


	//WebGui();
	WebGui(ESP8266WebServer *, Storage *);
	virtual ~WebGui();
	void handleFileList();
	void begin();
	unsigned char h2int(char c);
	String urldecode(String input) ;
	bool handleFileDownload(String fname);
	String getContentType(String filename);
	void handleFileDelete(String fname);
	void handleFileBrowser();
	void handleRoot();
	bool handleFileRead(String path);
	void handleFileUpload();

	//reference: http://stackoverflow.com/questions/3257896/c-for-each-calling-a-vector-of-callback-functions-and-passing-each-one-an-argu
	//void addCustomFunction(customFunction_t cb, String serverPath);
	//void handleCustomFunctions();
	//customFunctionList_t _customFunctionList;
    //typedef void result_type;//TODO: try to understand this (compiler error if this is not there)
    //void operator() (customFunctionRecord_t cb) { cb(); };
    //customServerPathList_t _customServerPathList;
    //customFunctionRecordVector_t _customFunctionRecordVector;
    //customFunctionRecord_t _customFunctionRecord;
protected:
	ESP8266WebServer *_server;
	Storage * _storage;
	File _fsUploadFile;
	//void (*customFunction_)() = NULL;
	//callback_vector m_cb;



};

#endif /* WEBGUI_H_ */
