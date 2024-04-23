#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>

bool isOtaInProgress = false;

void setupOta()
{
   // OTA
    //ArduinoOTA.setPort(8266);
    //ArduinoOTA.setHostname("esp1");
    // ArduinoOTA.setPassword((const char *)"123");

    ArduinoOTA.onStart([]() {
        DEBUGln("OTA Start");
        isOtaInProgress = true;
    });
    ArduinoOTA.onEnd([]() {
        DEBUGln("\nOTA End");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        DEBUGf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        DEBUGf("Error[%u]: ", error);

        if (error == OTA_AUTH_ERROR)
        {
            DEBUGln("Auth Failed");
        }
        else if (error == OTA_BEGIN_ERROR)
        {
            DEBUGln("Begin Failed");
        }
        else if (error == OTA_CONNECT_ERROR)
        {
            DEBUGln("Connect Failed");
        }
        else if (error == OTA_RECEIVE_ERROR)
        {
            DEBUGln("Receive Failed");
        }
        else if (error == OTA_END_ERROR)
        {
            DEBUGln("End Failed");
        }

        DEBUGln("Restarting...");
        ESP.restart();
    });
    ArduinoOTA.begin();
}

void handleOTA()
{
    ArduinoOTA.handle();
}
