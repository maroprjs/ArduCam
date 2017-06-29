/*
 * Cam.cpp
 *
 *  Created on: 13.05.2017
 *      Author: maro
 *      MaRo
 *      
 */

#include "Cam.h"
#include <Arduino.h>
#include <SPI.h>
#include "Debug.h"
#include <Wire.h>

Cam::Cam(ESP8266WebServer *server, Storage *storage) {
	_server = server;
	_storage = storage;

	//ArduCAM myCAM(OV2640, CS);
	_arduCam = new ArduCAM(OV2640, CS);


}

Cam::~Cam() {
	// TODO Auto-generated destructor stub
}

void Cam::begin(){
	init();//TODO: Handle bool value
	_server->on("/capture", std::bind(&Cam::serverCapture, this));
	_server->on("/stream", std::bind(&Cam::serverStream, this));

}



bool Cam::init(){
	bool ret = true;
	// set the CS as an output:
	pinMode(CS, OUTPUT);
	// initialize SPI:
	SPI.begin();
	SPI.setFrequency(SPI_SPEED); //4MHz
	ret = spiConnectionOk();
	// start I2C SPI:
	Wire.begin();
	if (ret == false){return ret;}
	if (camDetected() == true){
		//Change to JPEG capture mode and initialize the OV2640 module
		_arduCam->set_format(JPEG);
		_arduCam->InitCAM();
		_arduCam->OV2640_set_JPEG_size(INITIAL_JPG_SIZE);
		//_arduCam->clear_fifo_flag();
	}

	return ret;

}

bool Cam::spiConnectionOk(){
	uint8_t temp;
	bool ret = true;
	//Check if the ArduCAM SPI bus is OK
	_arduCam->write_reg(ARDUCHIP_TEST1, 0x55);
	temp = _arduCam->read_reg(ARDUCHIP_TEST1);
	if (temp != 0x55){
		PRINTLN("SPI1 interface Error!");
		ret = false;
	}

	return ret;
}

bool Cam::camDetected(){
	uint8_t vid, pid;
	bool ret = true;
	//Check if the camera module type is OV2640
	_arduCam->wrSensorReg8_8(0xff, 0x01);
	_arduCam->rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
	_arduCam->rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
	PRINT("vid: ");PRINTLN(vid);PRINT("pid: ");PRINTLN(pid);
	if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 ))){
		PRINTLN("Can't find OV2640 module!");
		ret = false;
	}
	else {PRINTLN("OV2640 detected.");}

	return ret;
}

void Cam::serverCapture(){
	handleServerCaptureArgs();
	SPI.end();
	SPI.begin();
	_arduCam->flush_fifo();
	_arduCam->clear_fifo_flag();PRINTLN("clear fifo");PRINTLN("starting capture..");
	startTimeMeasure();
	_arduCam->start_capture();
	if(captureReady()){
		stopTimeMeasure();
		PRINTLN("");PRINT("capture ready, elapsed time: ");PRINTLN(measuredTime());PRINT("starting transmission..");
		_picSize = _arduCam->read_fifo_length();//pictureSize();
		PRINT("picture length: ");PRINTLN(_picSize);
		if (_picSize >= MAX_FIFO_SIZE) PRINTLN("Over size.");//8M
		if (_picSize == 0 ) PRINTLN("Size is 0."); //0 kb
		startTimeMeasure();
		//sizeInfoToGui();//send size info to gui:
		//tranmit picture:
		showPicCaptureOnWeb();PRINTLN("");
		stopTimeMeasure();
		PRINTLN("");PRINT("transmission ready, elapsed time: ");PRINTLN(measuredTime());
	}

}

void Cam::serverStream(){
	String response = "HTTP/1.1 200 OK\r\n";
	response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
	_server->sendContent(response);
	handleServerCaptureArgs();
	SPI.end();
	SPI.begin();
	_arduCam->flush_fifo();
	while(1){
	_arduCam->clear_fifo_flag();PRINTLN("clear fifo");PRINTLN("starting capture..");
	startTimeMeasure();
	_arduCam->start_capture();
	if(captureReady()){
		stopTimeMeasure();
		PRINTLN("");PRINT("capture ready, elapsed time: ");PRINTLN(measuredTime());PRINT("starting transmission..");
		_picSize = _arduCam->read_fifo_length();//pictureSize();
		PRINT("picture length: ");PRINTLN(_picSize);
		if (_picSize >= MAX_FIFO_SIZE) PRINTLN("Over size.");//8M
		if (_picSize == 0 ) PRINTLN("Size is 0."); //0 kb
		response = "--frame\r\n";
		response += "Content-Type: image/jpeg\r\n\r\n";
		_server->sendContent(response);
		startTimeMeasure();
		//sizeInfoToGui();//send size info to gui:
		//tranmit picture:
		showPicCaptureOnWeb();PRINTLN("");
		stopTimeMeasure();
		PRINTLN("");PRINT("transmission ready, elapsed time: ");PRINTLN(measuredTime());
	}
	}

}

void Cam::handleServerCaptureArgs(){
	if (_server->hasArg("ql")){
		int ql = _server->arg("ql").toInt();
		_arduCam->OV2640_set_JPEG_size(ql);
		delay(1000);
		PRINTLN("QL change to: " + _server->arg("ql"));
	}
}



void Cam::sizeInfoToGui(){//TODO: not needed!
	String response = "HTTP/1.1 200 OK\r\n";
	response += "Content-Type: image/jpeg\r\n";
	response += "Content-len: " + String(_picSize) + "\r\n\r\n";
	_server->sendContent(response);

}




void Cam::showPicCaptureOnWeb(){
	//static const size_t bufferSize = 4096;
	static const size_t bufferSize = 1024;
	static uint8_t buffer[bufferSize] = {0xFF};
	uint8_t lowByte = 0, highByte = 0;
	size_t i = 0;
	bool is_header = false;
	const uint8_t DUMMY_SPI_REG = 0x00;
	//End Of Image (EOI) marker values FFh D9h.
	const uint8_t EOI_LOW_BYTE = 0xD9;
	const uint8_t EOI_HIGH_BYTE = 0xFF;
	// Start Of Image (SOI) marker values FFh D8h.
	const uint8_t SOI_LOW_BYTE = 0xD8;
	const uint8_t SOI_HIGH_BYTE = 0xFF;
	WiFiClient client = _server->client();

	_arduCam->CS_LOW();
	_arduCam->set_fifo_burst();

	while ( _picSize-- )
	{
		if (i%1000 == 0)PRINT(".");
		highByte = lowByte;
		lowByte =  SPI.transfer(DUMMY_SPI_REG);
		//Read JPEG data from FIFO
		if ( (lowByte == EOI_LOW_BYTE) && (highByte == EOI_HIGH_BYTE) ) //If find the end ,break while,
		{
			buffer[i++] = lowByte;  //save the last  0XD9
			//Write the remain bytes in the buffer
			//if (!client.connected()) break;
			client.write(&buffer[0], i);
			is_header = false;
			i = 0;
			_arduCam->CS_HIGH();
			break;
		}
		if (is_header == true)
		{
			//Write image data to buffer if not full
			if (i < bufferSize){
				buffer[i++] = lowByte;
			}
			else{
				//Write bufferSize bytes image data to file
				//if (!client.connected()) break;
				client.write(&buffer[0], bufferSize);
				i = 0;
				buffer[i++] = lowByte;
			}
		}
		else if ((lowByte == SOI_LOW_BYTE) & (highByte == SOI_HIGH_BYTE))
		{
			is_header = true;
			buffer[i++] = highByte;
			buffer[i++] = lowByte;
		}
	}

}

void Cam::startTimeMeasure(){
	PRINTLN("start time measure (stopwatch)");
	//_total_time = 0;
	_total_time = millis();
}

void Cam::stopTimeMeasure(){
	PRINTLN("stop time measure (stopwatch)");
	_total_time = millis() - _total_time;
}

uint32_t Cam::measuredTime(){
	return _total_time;
}

bool Cam::captureReady(){//TODO: this into arducam...or necesssary a all?
	int i = 0;
	while (!_arduCam->get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK)){
		i++;
		if ((i%200) == 0) PRINT(".");
		//if ((i%3000) == 0) PRINTLN("");
		yield();
	};
	return true;
}
