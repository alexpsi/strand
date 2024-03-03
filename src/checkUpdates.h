#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>
#include <semver.h>
#include "definitions.h"
#include <ESP8266httpUpdate.h>

void update_started()
{
    Serial.println("CALLBACK:  HTTP update process started");
}

void update_finished()
{
    Serial.println("CALLBACK:  HTTP update process finished");
}

void update_progress(int cur, int total)
{
    Serial.printf("CALLBACK:  HTTP update process at %d of %d bytes...\n", cur, total);
}

void update_error(int err)
{
    Serial.printf("CALLBACK:  HTTP update fatal error code %d\n", err);
}

void checkUpdates()
{
    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);

    client->setInsecure();

    HTTPClient https;

    if (https.begin(*client, "alexpsi.github.io", 443))
    { // HTTPS

        Serial.print("[HTTPS] GET...\n");
        // start connection and send HTTP header
        int httpCode = https.GET();

        // httpCode will be negative on error
        if (httpCode > 0)
        {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

            // file found at server
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
            {
                String payload = https.getString();
                Serial.println(payload);
                JsonDocument doc;
                DeserializationError error = deserializeJson(doc, payload);

                if (error)
                {
                    Serial.print(F("deserializeJson() failed: "));
                    Serial.println(error.f_str());
                    return;
                }

                JsonArray arr = doc["base"]["esp8266"].as<JsonArray>();
                int count = arr.size();
                Serial.printf("arr.size()=%d \n", count);
                for (int i = 0; i < count; i++)
                {
                    semver_t current_version = {};
                    semver_t compare_version = {};
                    const char *version = doc["base"]["esp8266"][i]["version"];
                    if (semver_parse(FRAMEWORK_VERSION, &current_version) || semver_parse(version, &compare_version))
                    {
                        fprintf(stderr, "Invalid semver string\n");
                    }
                    else
                    {
                        int resolution = semver_compare(compare_version, current_version);

                        if (resolution == 0)
                        {
                            printf("Versions %s is equal to: %s\n", version, FRAMEWORK_VERSION);
                        }
                        else if (resolution == -1)
                        {
                            printf("Version %s is lower than: %s\n", version, FRAMEWORK_VERSION);
                        }
                        else
                        {
                            printf("Version %s is higher than: %s\n", version, FRAMEWORK_VERSION);
                            // The line below is optional. It can be used to blink the LED on the board during flashing
                            // The LED will be on during download of one buffer of data from the network. The LED will
                            // be off during writing that buffer to flash
                            // On a good connection the LED should flash regularly. On a bad connection the LED will be
                            // on much longer than it will be off. Other pins than LED_BUILTIN may be used. The second
                            // value is used to put the LED on. If the LED is on with HIGH, that value should be passed
                            ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);

                            // Add optional callback notifiers
                            ESPhttpUpdate.onStart(update_started);
                            ESPhttpUpdate.onEnd(update_finished);
                            ESPhttpUpdate.onProgress(update_progress);
                            ESPhttpUpdate.onError(update_error);

                            const char *firmware_location = doc["base"]["esp8266"][i]["filename"];
                            t_httpUpdate_return ret = ESPhttpUpdate.update(
                                *client, strcat("https://alexpsi.github.io/firmware/", firmware_location));

                            switch (ret)
                            {
                            case HTTP_UPDATE_FAILED:
                                Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
                                break;

                            case HTTP_UPDATE_NO_UPDATES:
                                Serial.println("HTTP_UPDATE_NO_UPDATES");
                                break;

                            case HTTP_UPDATE_OK:
                                Serial.println("HTTP_UPDATE_OK");
                                break;
                            }
                        }
                    }
                    semver_free(&current_version);
                    semver_free(&compare_version);
                }
            }
        }
        else
        {
            Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
        }
        https.end();
    }
}