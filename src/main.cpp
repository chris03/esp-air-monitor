#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>

#include "debug.h"
#include "wifi.h"
#include "ota.h"

#define STRINGIFY(x) #x
#define STR(x) STRINGIFY(x)

// Uses BUILD_ENV_NAME (defined in platform.ini) to generate name of file to include
#include STR(boards/BUILD_ENV_NAME.h)

#ifdef BME_ENABLED
#include <Wire.h>
#include "SparkFunBME280.h"

BME280 bmeSensor;
bool bmeSensorOk = false;
#endif

#ifdef SI7021_ENABLED
#include <SI7021.h>
SI7021 si7021;
bool si7021Ok;
#endif

float sensorTemp;
float sensorHum;
int sensorPres;

#include <s8_uart.h>

SoftwareSerial S8_serial(PIN_RX_TO_S8 /*RX pin to S8 TX*/, PIN_TX_TO_S8 /*TX*/);
// HardwareSerial S8_serial(2);
S8_UART *sensor_S8;
S8_sensor sensor;
bool s8Ok;

ESP8266WebServer webServer(80);

bool restartRequested = false;
unsigned long nextRefresh = 0;

void refreshSensorData();

void setup()
{
  SERIALBEGIN();
  DEBUGln("");

  DEBUG("Reset reason: ");
  DEBUGln(ESP.getResetReason());

  setupWifi();

#ifdef BME_ENABLED
  Wire.begin();
  Wire.setClock(400000);

  DEBUG("BME280 init on address ");
  DEBUG(BME_I2C_ADDRESS);
  DEBUG(" : ")
  bmeSensor.setI2CAddress(BME_I2C_ADDRESS);
  bmeSensorOk = bmeSensor.beginI2C();
  DEBUGln(bmeSensorOk ? "Ok" : "Fail");
#endif

#ifdef SI7021_ENABLED
  DEBUG("Init Si7021: ");
  si7021Ok = si7021.begin(SDA, SCL);
  DEBUGln(si7021Ok ? "Ok" : "Fail");
#endif

  // Initialize S8 sensor
  S8_serial.begin(S8_BAUDRATE);
  sensor_S8 = new S8_UART(S8_serial);
  sensor_S8->get_firmware_version(sensor.firm_version);
  s8Ok = strlen(sensor.firm_version) > 0;
  if (s8Ok)
  {
    DEBUG("SenseAir S8 Firmware version:");
    DEBUGln(sensor.firm_version);
  }
  else
  {
    DEBUGln("SenseAir S8 CO2 sensor not found!");
  }

  // Configure web server
  webServer.on("/", []()
               {
                DEBUGln("webServer");
                 char buffer[128];
                 int charsWritten = 0;

#ifdef BME_ENABLED
                 if (bmeSensorOk && s8Ok && sensor.co2 > 0)
                 {
                   charsWritten = snprintf(buffer, sizeof(buffer), "{\"temp\": %.2lf, \"hum\": %.2lf, \"press\": %i, \"s8co2\": %i}", sensorTemp, sensorHum, sensorPres, sensor.co2);
                 }
                 else if (bmeSensorOk)
                 {
                   charsWritten = snprintf(buffer, sizeof(buffer), "{\"temp\": %.2lf, \"hum\": %.2lf, \"press\": %i, \"info\": \"%s val: %i\"}", sensorTemp, sensorHum, sensorPres, s8Ok ? "S8 ok" : "s8 not init", sensor.co2);
                 }
                 else if (s8Ok && sensor.co2 > 0)
                 {
                   charsWritten = snprintf(buffer, sizeof(buffer), "{\"s8co2\": %i}", sensor.co2);
                 }
#endif

#ifdef SI7021_ENABLED
                 if (si7021Ok && s8Ok && sensor.co2 > 0)
                 {
                   charsWritten = snprintf(buffer, sizeof(buffer), "{\"temp\": %.2lf, \"hum\": %.2lf, \"s8co2\": %i}", sensorTemp, sensorHum, sensor.co2);
                 }
                 else if (si7021Ok)
                 {
                   charsWritten = snprintf(buffer, sizeof(buffer), "{\"temp\": %.2lf, \"hum\": %.2lf}", sensorTemp, sensorHum);
                 }
                 else if (s8Ok && sensor.co2 > 0)
                 {
                   charsWritten = snprintf(buffer, sizeof(buffer), "{\"s8co2\": %i}", sensor.co2);
                 }
#endif
                 
                 webServer.send(200, "application/json; charset=utf-8", buffer, charsWritten); });

  webServer.on("/reset", []()
               {    
    webServer.send(200, "text/plain; charset=utf-8", "Rebooting...");
    restartRequested = true; });

  webServer.on("/ping", []()
               { webServer.send(200, "text/plain; charset=utf-8", "pong"); });

  webServer.on("/refresh", []()
               { 
                nextRefresh = 0;
                webServer.sendHeader("Location", "/");
                webServer.send(302, "text/plain; charset=utf-8", "Refresh"); });

  webServer.on("/info", []()
               { 
                 char buffer[128];
                 int charsWritten = snprintf(buffer, sizeof(buffer), "{\"name\": \"%s\", \"pioenv\": \"%s\", \"built\": %s, \"info\": \"%s\"}", NAME, STR(BUILD_ENV_NAME), STR(BUILD_TIME), boardInfo());

                webServer.send(200, "application/json; charset=utf-8", buffer, charsWritten); });

  webServer.begin();

  setupOta();
}

void loop()
{
  handleOTA();

  if (!isOtaInProgress)
  {
    if (nextRefresh < millis())
    {
      refreshSensorData();
      nextRefresh = millis() + 30000; // 30 sec.
    }

    webServer.handleClient();

    if (restartRequested)
    {
      DEBUGln("Trigger ESP restart");
      ESP.restart();
    }
  }
}

void refreshSensorData()
{
  DEBUGln("refreshSensorData()");

#ifdef BME_ENABLED
  if (bmeSensorOk)
  {
    sensorTemp = bmeSensor.readTempC();
    sensorHum = bmeSensor.readFloatHumidity();
    sensorPres = (int)bmeSensor.readFloatPressure();

    DEBUG("BME280 Temp: ");
    DEBUG(sensorTemp);
    DEBUG("°C  Hum: ");
    DEBUG(sensorHum);
    DEBUG("%  Baro: ");
    DEBUGln(sensorPres);
  }
#endif

#ifdef SI7021_ENABLED
  if (si7021Ok)
  {
    sensorTemp = si7021.getCelsiusHundredths() / 100.0;
    sensorHum = si7021.getHumidityPercent();

    DEBUG("SI7021 Temp: ");
    DEBUG(siTemp);
    DEBUG("°C  Hum: ");
    DEBUG(siHum);
    DEBUGln("%");
  }
#endif

  if (s8Ok)
  {
    sensor.co2 = sensor_S8->get_co2();
    DEBUG("CO2: ");
    DEBUG(sensor.co2);
    DEBUGln(" ppm");
  }
}