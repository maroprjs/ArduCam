/*
 * Esp8266.h
 *
 *  Created on: 15.05.2017
 *      Author: maro
 */

#ifndef ESP8266_H_
#define ESP8266_H_


#include <ESP8266WebServer.h>

class Esp8266 {
	public:
		Esp8266(ESP8266WebServer *);
		virtual ~Esp8266();
		void begin();
	private:
		void flashFirmware();
		void restart();
		void reportEspStatus();
	private:
		ESP8266WebServer *_server;

};



#endif /* ESP8266_H_ */
