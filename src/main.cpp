#include <Arduino.h>
#include <heltec.h>
#include <WiFi.h>
#include <SimpleTimer.h>

#include "config.h"

#include <Esp32MQTTClient.h>
#include <ArduinoJson.h>

// Global Vars

static bool hasIoTHub;

static uint32_t count_in;
static uint32_t count_out;

static uint32_t total_count_in;
static uint32_t total_count_out;

static uint8_t left_sensor_state;
static uint8_t right_sensor_state;

static int8_t left_sensor_state_last = -1;
static int8_t right_sensor_state_last = -1;

static bool inbound;
static bool outbound;

static uint32_t tm;

static SimpleTimer timer;

TaskHandle_t telemetryTaskHandle;

void wifiConnect()
{
  WiFi.disconnect();

  Heltec.display->clear();
  Heltec.display->drawString(0, 0, "Connecting to WiFi SSID:");
  Heltec.display->drawString(0, 10, WIFISSID);
  Heltec.display->display();

  Serial.print("Connecting to WiFi SSID: ");
  Serial.println(WIFISSID);

  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(WIFISSID, WIFIPWD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("...");
  }

  Serial.println("Connected to WiFi!");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
}

void iotcConnect()
{
  Heltec.display->clear();
  Heltec.display->drawString(0, 0, "Connecting to Azure IoTC");
  Heltec.display->display();
  if (!Esp32MQTTClient_Init((const uint8_t *)IOTC_CONNECTION_STRING))
  {
    hasIoTHub = false;
    Serial.println("Initializing IoT hub failed.");
    return;
  }

  // Signal iot hub connected
  hasIoTHub = true;
  digitalWrite(BUILTIN_LED, HIGH);
}

void drawScreen()
{
  Heltec.display->clear();

  if (left_sensor_state == 0)
  {
    Heltec.display->fillRect(0, 0, 64, 64);
  }
  if (right_sensor_state == 0)
  {
    Heltec.display->fillRect(64, 0, 64, 64);
  }

  Heltec.display->setColor(BLACK);
  Heltec.display->fillRect(44, 22, 40, 20);
  Heltec.display->setColor(WHITE);
  Heltec.display->drawString(44, 22, "In:");
  Heltec.display->drawString(65, 22, String(total_count_in));

  Heltec.display->drawString(44, 32, "Out:");
  Heltec.display->drawString(65, 32, String(total_count_out));
  Heltec.display->display();
}

void checkIn()
{
  if (left_sensor_state_last != left_sensor_state)
  {
    left_sensor_state_last = left_sensor_state;
    if ((inbound == false) && (left_sensor_state == LOW) && (right_sensor_state == HIGH))
    {
      inbound = true;
      tm = millis();
    }
  }

  if ((inbound == true) && (left_sensor_state == HIGH) && (right_sensor_state == LOW))
  {
    inbound = false;
    outbound = false;
    count_in++;
    total_count_in++;
    Serial.println("count_in");
  }
}

void checkOut()
{
  if (right_sensor_state_last != right_sensor_state)
  {
    right_sensor_state_last = right_sensor_state;
    if ((outbound == false) && (right_sensor_state == LOW) && (left_sensor_state == HIGH))
    {
      outbound = true;
      tm = millis();
    }
  }

  if ((outbound == true) && (right_sensor_state == HIGH) && (left_sensor_state == LOW))
  {
    inbound = false;
    outbound = false;
    count_out++;
    total_count_out++;
    Serial.println("count_out");
  }
}

void checkSensors()
{
  left_sensor_state = digitalRead(LEFT_SENSOR_PIN);
  right_sensor_state = digitalRead(RIGHT_SENSOR_PIN);
  checkIn();
  checkOut();
  if (((millis() - tm) > TIMEOUT) && (right_sensor_state == HIGH) && (left_sensor_state == HIGH))
  {
    inbound = false;
    outbound = false;
  }
  drawScreen();
}

void sendTelemetry(void *parameter)
{
  if (hasIoTHub)
  {
    uint32_t telemetry_tm = millis();
    Serial.println("Beginning upload");
    DynamicJsonDocument root(MAX_MESSAGE_LEN);
    root["people_in"] = count_in;
    root["people_out"] = count_out;

    char buff[MAX_MESSAGE_LEN];
    serializeJson(root, buff);
    Serial.println(buff);
    if (Esp32MQTTClient_SendEvent(buff))
    {
      Serial.println("Sending data succeed");
      count_in = 0;
      count_out = 0;
    }
    else
    {
      Serial.println("Failure...");
    }
    Serial.print("Upload process duration: ");
    Serial.println(millis() - telemetry_tm);
  }
  vTaskDelete(telemetryTaskHandle);
}

void telemetryTask()
{
  xTaskCreate(
      sendTelemetry,
      "sendTelemetry",
      4096,
      NULL,
      1,
      &telemetryTaskHandle);
}

void setup()
{
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, true /*Serial Enable*/);

  Heltec.display->flipScreenVertically();
  Heltec.display->setFont(ArialMT_Plain_10);

  wifiConnect();
  iotcConnect();

  pinMode(LEFT_SENSOR_PIN, INPUT);
  pinMode(RIGHT_SENSOR_PIN, INPUT);

  timer.setInterval(10, checkSensors);
  timer.setInterval(UPLOAD_INTERVAL, telemetryTask);
}

void loop()
{
  timer.run();
}