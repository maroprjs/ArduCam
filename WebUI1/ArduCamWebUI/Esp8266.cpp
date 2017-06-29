/*
 * Esp8266.cpp
 *
 *  Created on: 15.05.2017
 *      Author: maro
 */

#include "Esp8266.h"
#include "Debug.h"


Esp8266::Esp8266(ESP8266WebServer *server) {
	_server = server;

}

Esp8266::~Esp8266() {
	// TODO Auto-generated destructor stub
}

void Esp8266::begin(){
	_server->on("/restartesp", std::bind(&Esp8266::restart, this));
	_server->on("/flashupdate", HTTP_POST, [this]() { _server->send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK Restarting....wait");ESP.restart(); }, std::bind(&Esp8266::flashFirmware, this));
	_server->on("/espstatus", std::bind(&Esp8266::reportEspStatus, this));

}

void Esp8266::flashFirmware(){
	//if (isAdmin() == false) return;
	// handler for the file upload, get's the sketch bytes, and writes
	// them through the Update object
	HTTPUpload& upload = _server->upload();
	if (upload.status == UPLOAD_FILE_START) {
		//TODO: check if this is needed: WiFiUDP::stopAll();
		//TODO: make this semaphpre MyWebServer.OTAisflashing = true;
		PRINTLN("Update: " + upload.filename);
		uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
		if (!Update.begin(maxSketchSpace)) {//start with max available size
			Update.printError(Serial);
		}
	}
	else if (upload.status == UPLOAD_FILE_WRITE) {
		PRINT(".");
		if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
			Update.printError(Serial);

		}
	}
	else if (upload.status == UPLOAD_FILE_END) {
		if (Update.end(true)) { //true to set the size to the current progress
			PRINTLN("Update Success: \nRebooting...\n"+ String(upload.totalSize));
		}
		else {
			Update.printError(Serial);
		}
	}
	else if (upload.status == UPLOAD_FILE_ABORTED) {
		Update.end();
		PRINTLN("Update was aborted");
		//TODO: make this semaphpre MyWebServer.OTAisflashing = false;
	}
	delay(0);
};

void Esp8266::restart() {
	//if (isAdmin() == false) return;
	_server->send(200, "text/plain", "Restarting ESP...");
	delay(100);
	ESP.restart();
}


void Esp8266::reportEspStatus(){
	PRINTLN("reportEspStatus called ");
	String json = "{";
	json += "\"heap\":"+String(ESP.getFreeHeap());
	json += ", \"analog\":"+String(analogRead(A0));
	json += ", \"gpio\":"+String((uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16)));
	json += "}";
	_server->send(200, "text/json", json);
	json = String();
}
