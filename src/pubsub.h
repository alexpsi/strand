#include "Arduino.h"
#include "ESP8266WiFi.h"
#include <WiFiClientSecureBearSSL.h>
#include <PubSubClient.h>

WiFiClient wifiClient;

/***** Call back Method for Receiving MQTT messages and Switching LED ****/

void callback(char* topic, uint8_t* payload, unsigned int length)
{
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();
    Serial.println("-----------------------");
}

PubSubClient client("192.168.0.102", 1883, callback, wifiClient);

/**** Method for Publishing MQTT Messages **********/
void publishMessage(const char *topic, String payload)
{
    if (client.publish(topic, payload.c_str(), true))
        Serial.println("Message publised [" + String(topic) + "]: " + payload);
}

/************* Connect to MQTT Broker ***********/
void reconnect()
{
    // Loop until we're reconnected
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        String clientId = "ESP8266Client-"; // Create a random client ID
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        if (client.connect(clientId.c_str()))
        {
            Serial.println("connected");

            // subscribe the topics here
            publishMessage("esp8266/test", "braty1");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds"); // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void setupMQTT()
{
    reconnect();

    client.subscribe("esp8266");
}