#define OTA_ENDPOINT "https://alexpsi.github.io/"
#define OTA_PACKAGES "packages.json"
#define OTA_PACKAGES_ENDPOINT OTA_ENDPOINT OTA_PACKAGES
#define FRAMEWORK_VERSION "0.0.3"

#define NO_CONNECTION_RESTART_DELAY 3600000 //ms, 1 hour
#define NO_CONNECTION_GO_WILD_DELAY 1800000 //ms, 30 minutes

#define AP_NAME mycelium

#define CONFIG_RESET_BUTTON 0  // GPIO0, D0 on Node32, D3 on NodeMCU8266. Pressing this button for more than 5-10sec will reset the WiFi configuration

#define DEBUG true