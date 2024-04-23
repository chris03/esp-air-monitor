#include <ESP8266WiFi.h>

// const String serverUrl = "https://";
// const char *ssid = "***CHANGE ME***";
// const char *password = "***CHANGE ME***";
#include "secrets.h" // Contains the above values

void setupWifi()
{
    DEBUGln("Connecting to wifi...");
    int wifiRetries = 0;
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    do
    {
        delay(100);

        if (wifiRetries == 300)
        {
            DEBUGln("Failed to connect, reset in 5 sec.");
            delay(5000);
            ESP.reset();
            return;
        }

        wifiRetries++;

    } while (WiFi.status() != WL_CONNECTED);

    DEBUGln("");
    DEBUG("Connected to ");
    DEBUGln(ssid);
    DEBUG("IP address: ");
    DEBUGln(WiFi.localIP());
    DEBUG("MAC:  ");
    DEBUGln(WiFi.macAddress());
}
