#define DEBUG_ETHERNET_WEBSERVER_PORT       Serial

// Debug Level from 0 to 4
#define _ETHERNET_WEBSERVER_LOGLEVEL_       3

#include <WebServer_WT32_ETH01.h>
#include <ElegantOTA.h>
#include "wificonfig.h"
#include "config.h"
#include "wiegandOutput2.h"


WebServer server(80);


// Select the IP address according to your local network
IPAddress myIP(192, 168, 0, 233);
IPAddress myGW(192, 168, 0, 1);
IPAddress mySN(255, 255, 255, 0);

// Google DNS Server IP
IPAddress myDNS(8, 8, 8, 8);


WiegandOut wiegandOut1(32,33);
WiegandOut wiegandOut2(5,17);
WiegandOut wiegandOut3(15,14);
WiegandOut wiegandOut4(12,4);

void sendvalues(WiegandOut bus,unsigned long long numberL,int code,int bits){

  if (bits==26) 
  {
    bus.send(numberL,26,code); // Send 26 bits with facility code
    delay(100);
  }
  if (bits==32) 
  {
    bus.send(numberL,32,code); // Send 32 bits with facility code
    delay(1000);
  }
   if (bits==37) 
  {
    bus.send(numberL,37,code); // Send 37 bits with facility code
    delay(1000);
  }
}

void handlenumber(){
   if (!server.hasArg("number")) 
  {
    server.send(500, "text/plain", "BAD ARGS");
    return;
  }
   if (!server.hasArg("code")) 
  {
    server.send(500, "text/plain", "BAD ARGS");
    return;
  }
  if (!server.hasArg("bits")) 
  {
    server.send(500, "text/plain", "BAD ARGS");
    return;
  }
  
  String number = server.arg("number");
  String code = server.arg("code");
  String bits=server.arg("bits");
  String gate=server.arg("gate");
  String msg = "number: " + number + " code: " +code + " bits: "+bits + " gate: " +gate;
  unsigned long long numberH,numberL;

  //---------------- Create a long long from a String
numberL = 0; //Will be a long long number

for (int i = 0; i < number.length(); i++) {
    char c = number.charAt(i);
   if (c < '0' || c > '9') break;
   numberL *= 10;
   numberL += (c - '0');   
  }
//----------
  switch (gate.toInt()){

    case 1:
      sendvalues(wiegandOut1,numberL,code.toInt(),bits.toInt());
      break;
    case 2:
      sendvalues(wiegandOut2,numberL,code.toInt(),bits.toInt());
      break;
    case 3:
      sendvalues(wiegandOut3,numberL,code.toInt(),bits.toInt());
      break;
    case 4:
      sendvalues(wiegandOut4,numberL,code.toInt(),bits.toInt());
      break;  
  }
  


  server.send(200, "text/plain", msg);
  
  
  number.clear();code.clear();bits.clear();gate.clear();
  delay(100);

}


void handlewifi(){
  server.send(200, "text/plain", "reset WiFi configuration");

   
  delay(100);

}

void  handleroot(){
  char html[1300];
  Serial.print("server uri=");Serial.println(server.uri());
  snprintf(html, 1300,

           "<html>\
  <head>\
  <h1>Wiegand API use:</h1>\
<p ><span style=\"text-decoration: underline;\">Wiegand calls</span></p>\
<p><em><strong>http://%s/id?number=</strong>person_number_id<strong>&amp;code=</strong>facility_code&amp;<strong>bits=</strong>bits_format&amp;<strong>gate=</strong>gate_number</em></p>\
<ul>\
<li>person_number= long</li>\
<li>facility_code= int</li>\
<li>bits_format= 26,32 or 37</li>\
<li>gate_number= 1,2,3 or 4 </li>\
</ul>\
<p><span style=\"text-decoration: underline;\">Wifi reconfigure credentials</span></p>\
<ul>\
<li>Option 1</li>\
</ul>\
<p>http://%s/reconfig</p>\
<ul>\
<li>Option 2\
<ul>\
<li>Press reset button</li>\
<li>wait 3 sec</li>\
<li>Press flash button</li>\
</ul>\
</li>\
</ul>\
<p>&nbsp;</p>\
<p><span style=\"text-decoration: underline;\">OTA</span></p> \
<p>The firmware can be updated via OTA. The .bin must be accesible on (case sensitive)  http://ota.aerin.es/%s.bin</p>\
<p>When .bin is in the correct place just call to OTA process with</p>\
<p>http://%s/OTA</p>\
<p>To enter the configuration portal select Aerin_XXXX access point (pass Aerin)</p>\
  </body>\
</html>",

           server.hostHeader().c_str(),server.hostHeader().c_str(),server.hostHeader().substring(0,17).c_str(),server.hostHeader().c_str()
          );
  
server.send(200, "text/html", html);

}
void update_started() {
  Serial.println("CALLBACK:  HTTP update process started");
}

void update_finished() {
  Serial.println("CALLBACK:  HTTP update process finished");
}

void update_progress(int cur, int total) {
  Serial.printf("CALLBACK:  HTTP update process at %d of %d bytes...\n", cur, total);
}

void update_error(int err) {
  Serial.printf("CALLBACK:  HTTP update fatal error code %d\n", err);
}


void setup()
{
  

  //Change : on MAC by - to avoid http calls
  String ESP_host=ESP_getChipId();
  Serial.begin(115200);

  
   WT32_ETH01_onEvent();

  //bool begin(uint8_t phy_addr=ETH_PHY_ADDR, int power=ETH_PHY_POWER, int mdc=ETH_PHY_MDC, int mdio=ETH_PHY_MDIO, 
  //           eth_phy_type_t type=ETH_PHY_TYPE, eth_clock_mode_t clk_mode=ETH_CLK_MODE);
  //ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER, ETH_PHY_MDC, ETH_PHY_MDIO, ETH_PHY_TYPE, ETH_CLK_MODE);
  ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER);

  // Static IP, leave without this line to get IP via DHCP
  //bool config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1 = 0, IPAddress dns2 = 0);
  ETH.config(myIP, myGW, mySN, myDNS);

  WT32_ETH01_waitForConnect();
  
  server.on("/id", HTTP_GET, handlenumber);
  server.on("/reconfig", HTTP_GET, handlewifi);
  server.on("/", HTTP_GET, handleroot);
//  server.on("/OTA", HTTP_GET, handleOTA);
ElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
  

  ESP_host.replace(":","-");
  Serial.print(F("Open http://"));
  Serial.print(ESP_host);
  Serial.println(F(".local/ to see the file browser"));
 //   
 
}

void loop()
{ 
  // put your main code here, to run repeatedly
  //check_status();
 
  
  server.handleClient();
  
}
