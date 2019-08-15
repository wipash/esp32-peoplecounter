#include "Arduino.h"
#include "heltec.h"
#include "WiFi.h"
#include "SimpleTimer.h"

#include "config.h"

int count_in = 0;
int count_out = 0;

int left_sensor_state = 0;
int right_sensor_state = 0;

int left_sensor_state_last = -1;
int right_sensor_state_last = -1;

bool inbound = false;
bool outbound = false;

unsigned long tm;

SimpleTimer timer;

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
  Heltec.display->drawString(65, 22, String(count_in));

  Heltec.display->drawString(44, 32, "Out:");
  Heltec.display->drawString(65, 32, String(count_out));
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
    count_in++;
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
    outbound = false;
    count_out++;
    Serial.println("count_out");
  }
}

void checkSensors()
{
  left_sensor_state = digitalRead(LEFT_SENSOR_PIN);
  right_sensor_state = digitalRead(RIGHT_SENSOR_PIN);
  checkIn();
  checkOut();
  if ((millis() - tm) > TIMEOUT)
  {
    inbound = false;
    outbound = false;
  }
  drawScreen();
}

void setup()
{
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, true /*Serial Enable*/);

  Heltec.display->flipScreenVertically();
  Heltec.display->setFont(ArialMT_Plain_10);

  Heltec.display->clear();
  Heltec.display->drawString(0, 0, "Connecting to WiFi SSID:");
  Heltec.display->drawString(0, 10, WIFISSID);
  Heltec.display->display();

  Serial.print("Connecting to WiFi SSID: ");
  Serial.println(WIFISSID);

  WiFi.begin(WIFISSID, WIFIPWD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("...");
  }

  Serial.println("Connected to WiFi!");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());

  pinMode(LEFT_SENSOR_PIN, INPUT);
  pinMode(RIGHT_SENSOR_PIN, INPUT);

  timer.setInterval(10, checkSensors);
}

void loop()
{
  timer.run();
}