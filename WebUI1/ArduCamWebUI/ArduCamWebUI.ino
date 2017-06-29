


#include "Debug.h"
#include <ESP8266WebServer.h>
#include "Storage.h"
#include "WebGui.h"
#include "Network.h"
#include "Cam.h"
#include "Esp8266.h"



ESP8266WebServer server(80);
Storage storage;
Network network;//(wifi of course) TODO: server&fs as input for wifi settings through web interface
Cam pbCam(&server, &storage);
WebGui gui(&server, &storage);
Esp8266 myEsp(&server);



void setup(void){
  PRINT_INIT(115200);
  PRINT("\n");
  DBGOUTPUT(true);
  network.begin();
  pbCam.begin();//TODO: handle bool return value
  gui.begin();
  myEsp.begin();

}

void loop(void){
  server.handleClient();
}


