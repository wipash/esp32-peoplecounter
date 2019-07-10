#include "Arduino.h"
#include "heltec.h"

#define left_sensor 17
#define right_sensor 23

int count_in = 0;
int count_out = 0;
bool counting = false;

void setup() {
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, true /*Serial Enable*/);

  Heltec.display->flipScreenVertically();
  Heltec.display->setFont(ArialMT_Plain_10);

  pinMode(left_sensor, INPUT);
  pinMode(right_sensor, INPUT);
}
void loop() {
  Heltec.display->clear();

  int left_val = digitalRead( left_sensor );
  int right_val = digitalRead( right_sensor );

  if (left_val == 0) {
    Heltec.display->fillRect(0,0,64,64);
  }
  if (right_val == 0) {
    Heltec.display->fillRect(64,0,64,64);
  }

  if (left_val == 0 && right_val == 1 && counting == false){
    count_in ++;
    counting = true;
  }

  if (left_val == 1 && right_val == 0 && counting == false){
    count_out ++;
    counting = true;
  }

  if (left_val == 1 && right_val == 1 && counting == true){
    counting = false;
  }

  Heltec.display->setColor(BLACK);
  Heltec.display->fillRect(44,22,40,20);
  Heltec.display->setColor(WHITE);
  Heltec.display->drawString(44,22,"In:");
  Heltec.display->drawString(65,22,String(count_in));

  Heltec.display->drawString(44,32,"Out:");
  Heltec.display->drawString(65,32,String(count_out));
  Heltec.display->display();
  delay(10);
}