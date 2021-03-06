/*
 * Debug.h
 *
 *  Created on: 21.10.2016
 *      Author: maro
 */

#ifndef DEBUG_H_
#define DEBUG_H_





#define DEBUG
#ifdef DEBUG
    #include <FS.h>
	String formatBytes(size_t bytes);
	#define PRINT_INIT(x)	Serial.begin(x)
	#define PRINT(x)  	Serial.print(x)
	#define PRINTLN(x)  Serial.println(x)
	#define DBGOUTPUT(x) Serial.setDebugOutput(x)
	#define ZVERSION "debug xxx " + String(__DATE__)
	#define ZUP_TIME " " + String(millis()/1000) + " [s]"
	//#define SHOWSPIFFS
	#define SHOWSPIFFS   \
						SPIFFS.begin();\
						{\
							Dir dir = SPIFFS.openDir("/");\
							while (dir.next()) {\
								String fileName = dir.fileName();\
								size_t fileSize = dir.fileSize();\
								Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());\
							}\
							Serial.printf("\n");\
						}




#else
	#define PRINT_INIT(...)
	#define PRINT(...)
	#define PRINTLN(...)
    #define DBGOUTPUT(...)
	#define SHOWSPIFFS
	#define ZVERSION "xxxx" + String(__DATE__)
	#define ZUP_TIME " " + String(millis()/1000) + " [s]"
#endif

#endif /* DEBUG_H_ */
