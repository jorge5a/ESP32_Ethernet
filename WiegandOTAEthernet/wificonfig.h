

  #include <DNSServer.h>
  
  #include "config.h"
#define ESP_getChipId()   (WiFi.macAddress())
void setupwifi();
void resetwifi();
void check_status();
