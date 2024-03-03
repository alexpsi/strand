#include "definitions.h"
#include <WIFIConfig.h>
#include "checkUpdates.h"
#include "pubsub.h"

ESPWifiConfig WifiConfig("myESP", 80, CONFIG_RESET_BUTTON, DEBUG);

unsigned long last_wifi_connect_time = 0;
unsigned long reconnect_delay = 10000;
boolean firstTime = true;

boolean ensure_wifi_connectivity()
{
  // Client Mode
  if (WifiConfig.ESP_mode == CLIENT_MODE)
  {
    if (WifiConfig.wifi_connected)
    {
      last_wifi_connect_time = millis();
      return true;
    }
    else
    {
      // No or lost connection to the WiFi
      if ((millis() - last_wifi_connect_time) > NO_CONNECTION_RESTART_DELAY) // Restart after 1 hour of no connection
      {
        Serial.println("Restarting...");
        ESP.restart();
      }
    }
  }
  return false;
}

void setup()
{
  Serial.begin(115200);
  delay(500);
  if (WifiConfig.initialize() == AP_MODE)
  {
    WifiConfig.Start_HTTP_Server(600000);
  }
}

void loop()
{
  WifiConfig.handle(reconnect_delay);
  if (ensure_wifi_connectivity())
  {
    if (firstTime)
    {
      firstTime = false;
      checkUpdates();
      setupMQTT();
    }
    client.loop();
  }
}
