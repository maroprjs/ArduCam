/*
 * Cam.h
 *
 *  Created on: 13.05.2017
 *      Author: MaRo
 *
 *      -in memorysaver(enabled HW plattform: OV2640_MINI_2MP, disable OV5642_MINI_5MP_PLUS)
 *
 *      
 */

#ifndef CAM_H_
#define CAM_H_
#include "ArduCAM.h"
#include "memorysaver.h"
#include <ESP8266WebServer.h>
#include "Storage.h"

class Cam {
	public:
		Cam(ESP8266WebServer *, Storage *);
		virtual ~Cam();
		void begin();
		// set GPIO16 as the slave select for cam :
		const int CS = 16;//TODO: make this dynamic!?
		//const int SD_CS = 0;
		const uint32_t SPI_SPEED = 2000000; //4MHz //TODO: is somewhere SPI_HALF_SPEED defined?
		const uint8_t INITIAL_JPG_SIZE = OV2640_640x480;
		void serverCapture();
		void serverStream();
		void handleServerCaptureArgs();
		void startTimeMeasure();
		void stopTimeMeasure();
		uint32_t measuredTime();
		bool captureReady();
		//uint32_t pictureSize();
	private:
		bool init();
		bool spiConnectionOk();
		bool camDetected();
		void showPicCaptureOnWeb();
		void sizeInfoToGui();

	private:
		ArduCAM *_arduCam;
		ESP8266WebServer *_server;
		Storage *_storage;
		uint32_t _total_time = 0;//TODO: check if unsigned long = uint32_t
		uint32_t _picSize = 0;
};

#endif /* CAM_H_ */
