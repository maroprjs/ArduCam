/*
 * Network.cpp
 *
 *  Created on: 10.05.2017
 *      Author: maro
 */

#include "Network.h"
#include "ssid.h"
#include "Debug.h"
#include <ESP8266WebServer.h>

Network::Network() {
	// TODO Auto-generated constructor stub

}

Network::~Network() {
	// TODO Auto-generated destructor stub
}

void Network::begin(){
	//WIFI INIT
	PRINT("Connecting to "); PRINTLN(ssid);
	if (String(WiFi.SSID()) != String(ssid)) {
		WiFi.begin(ssid, password);
	}

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		PRINT(".");
	}
	PRINTLN("");
	PRINT("Connected! IP address: ");
	PRINTLN(WiFi.localIP());
}

