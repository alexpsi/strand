#include "WIFIConfig.h"

#define AP_NAME "Strand"

const EEstore<int> eeInt(-3685); // for storing int

const EEstring eeWIFI_SSID(20, ""); // for storing String (20 chars max)
const EEstring eeWIFI_PASS(20, "");
const EEstore<boolean> eeConfigured(false); // for storing int

int ESPWifiConfig::initialize(void)
{
  ESP_debug(F("ESPWifiConfig::initialize"));
  pinMode(reset_btn, INPUT_PULLUP);
  debug_log.reserve(50);
  WiFi.persistent(false);
  WiFi.disconnect();

  if (eeConfigured.get())
  {
    ESP_mode = CLIENT_MODE;
    wifi_connected = false;
    ESP_debug(CLIENT_MODE_NAME);
    delay(500);
    WiFi.mode(WIFI_STA);
    ESP_debug(F("ESPWifiConfig::initialize:connectWIFI"));
    ESP_debug(eeWIFI_SSID.get());
    ESP_debug(eeWIFI_PASS.get());
    wifiMulti.addAP(eeWIFI_SSID.get().c_str(), eeWIFI_PASS.get().c_str());
    try_wifi_connect();
  }
  else
  {
    ESP_debug(F("ESPWifiConfig::initialize:accessPointNotConfigured"));
    ESP_mode = AP_MODE;
    WiFi.mode(WIFI_AP);
    ESP_debug(AP_MODE_NAME);
    WiFi.disconnect();
    delay(500);

    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(AP_NAME);

    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(DNS_PORT, "*", apIP);
    // Setup MDNS responder
    if (!MDNS.begin(sys_name))
    {
      ESP_debug("Warn: Can't setup MDNS");
    }
    else
    {
      ESP_debug("Setup URL:\r\nhttp://" + String(sys_name) + ".local");
      // Add service to MDNS-SD
      MDNS.addService("http", "tcp", 80);
    }

  }
  return ESP_mode;
}

boolean ESPWifiConfig::try_wifi_connect()
{
  ESP_debug("ESPWifiConfig::try_wifi_connect");
  if (wifiMulti.run() == WL_CONNECTED)
  {
    ESP_debug(F("ESPWifiConfig::try_wifi_connect::WiFi connected"));
    wifi_connected = true;
  }
  else
  {
    if (wifi_connected)
      ESP_debug("ESPWifiConfig::try_wifi_connect::WiFi disconnected!");
    wifi_connected = false;
  }
  return wifi_connected;
}

void ESPWifiConfig::handle(unsigned long reconnect_delay)
{
  if ((ESP_mode != AP_MODE) && (((millis() - last_try_wifi_connect) > reconnect_delay) || (wifi_connected && (WiFi.status() != WL_CONNECTED))))
  {

    if (is_reset_pressed(reset_btn))
    {
      reset_pressed_count++;
      if (reset_pressed_count >= 2)
      {
        ESP_debug(F("ESP:Settings RESET"));
        eeWIFI_PASS << "";
        eeWIFI_SSID << "";
        eeConfigured << false;
        ESP.restart();
        // initialize();
      }
    }
    else
      reset_pressed_count = 0;

    last_try_wifi_connect = millis();

    if (WiFi.status() != WL_CONNECTED)
    {
      if (try_wifi_connect())
        fails_try_wifi_connect = 0;
      else
        fails_try_wifi_connect++;
    }
    else
    {
      wifi_connected = true;
      fails_try_wifi_connect = 0;
    }
  }
  if (isHTTPserverRunning)
  {
    dnsServer.processNextRequest();
    server.handleClient();
    if ((ESP_mode == CLIENT_MODE) && (client_mode_active_time > 0) && (millis() > client_mode_active_time))
    {
      isHTTPserverRunning = false;
      ESP_debug(F("HTTP server closed\n"));
      server.close();
      dnsServer.stop();
    }
  }
}

void ESPWifiConfig::Start_HTTP_Server(unsigned long client_mode_active_time_x)
{
  client_mode_active_time = client_mode_active_time_x;
  server.on("/setup", std::bind(&ESPWifiConfig::handle_setup, this));
  server.onNotFound(std::bind(&ESPWifiConfig::handleNotFound, this));
  const char *headerkeys[] = {"User-Agent", "Cookie"};
  size_t headerkeyssize = sizeof(headerkeys) / sizeof(char *);
  // ask server to track these headers
  server.collectHeaders(headerkeys, headerkeyssize);
  server.begin();
  isHTTPserverRunning = true;
  ESP_debug(F("HTTP server started\n"));
}

void ESPWifiConfig::handle_setup()
{
  // ESP_debug(F("ESP:req_setup"));
  delay(0);
  yield();
  if (server.hasArg("WIFI_SSID") && server.arg("WIFI_SSID").length() > 0 &&
      server.hasArg("WIFI_PASS") && server.arg("WIFI_PASS").length() > 0)
  {
    eeWIFI_PASS << server.arg("WIFI_PASS");
    eeWIFI_SSID << server.arg("WIFI_SSID");
    eeConfigured << true;
    server.send(200, "text/plain", "");
    delay(0);
    yield();
    ESP.restart();
  }
  server.send(500, "text/plain", "");
  delay(0);
  yield();
}

void ESPWifiConfig::handleNotFound(void)
{
  delay(0);
  yield();
  server.send(404, F("text/plain"), "Not found");
  delay(0);
  yield();
}

boolean ESPWifiConfig::is_reset_pressed(int reset_button)
{
  for (int i = 0; i < 50; i++)
  {
    if (digitalRead(reset_button) == 1)
    {
      return false;
    }
    delay(1);
  }
  return true;
}

unsigned long ESPWifiConfig::getmacID()
{
  unsigned long mac_addr_short = 0;
  String mac_real = WiFi.macAddress();
  unsigned long power = 1;
  for (int i = 3; i < mac_real.length(); i++)
  {
    if ((mac_real[i] != ':') && (mac_real[i] != ' '))
    {
      mac_addr_short += (((unsigned char)mac_real[i]) * (power));
    }
    power = power * 10;
  }
  return mac_addr_short % 1000000000;
}

void ESPWifiConfig::ESP_debug(String line)
{
  if (show_debug)
    Serial.println(line);

  debug_log += line;
  debug_log += "\n";
  if (debug_log.length() > 50)
  {
    debug_log = debug_log.substring(10);
  }
}