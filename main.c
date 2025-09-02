#include <EEPROM.h>
#include <SPI.h>
#include <MFRC522.h>
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);

char rcv[16] = "";
char list[20][14] = {};
int cardnum = 0;
int lengthv = 0;
bool snext = false;
void setup() {
  Serial.begin(9600);

  SPI.begin();
  mfrc522.PCD_Init();
  pinMode(5, INPUT);
  pinMode(8, INPUT);
  pinMode(3, OUTPUT);  
  pinMode(7, OUTPUT);
  pinMode(6, OUTPUT);

  bool reading = false;
  int point = 0;
  for (unsigned int i = 0; i < EEPROM.length(); i++) {
    int j = EEPROM.read(i);

    if (j == 255) {
      break;
    }
    lengthv ++;
    if (j == 89) {
      reading = true;
    }
    if (j == 90) {
      reading = false;
      point = 0;
      cardnum ++;
    }
    if (reading == true && j != 89) {
      list[cardnum][point] = char(j);
      point ++;
    }
  }
}
void(* resetFunc) (void) = 0;
bool programming = false;
bool add = false;
int reset = 0;
int over = 0;
void loop() {

  delay(100);
  int value = digitalRead(8);
  int value2 = digitalRead(5);

  if (programming == true) {
    reset = reset + 1;
  }
  if (reset == 30) {
    reset = 0;
    programming = false;
    digitalWrite(7, LOW);
    digitalWrite(3, LOW);
  }
  if (value == 1) {
    reset = 0;
    programming = true;
    digitalWrite(7, HIGH);
    digitalWrite(3, LOW);
    add = true;
  }
  if (value2 == 1) {
    reset = 0;
    digitalWrite(3, HIGH);
    digitalWrite(7, LOW);
    add = false;
    programming = true;
  }
  if(programming == false){
    over ++;
  }
  if (over > 6000){
    resetFunc();
  }
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  String content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  String z = content.substring(1);
  z.toUpperCase();
  z.replace(" ", "");
  for (unsigned int i = 0; i < z.length(); i++) {
    rcv[i] = z[i];
  }
  Serial.println(rcv);

  if (programming == true) {
    if (add == true) {
      Add();
    } else {
      Sub();
    }
    delay(1000);
    snext = true;
    return;

  } else if (snext == false) {
    for (int i = 0; i < 15; i++) {
      if ( strcmp(rcv, list[i]) == 0) {
        digitalWrite(6, HIGH);
        delay(1000);
        digitalWrite(6, LOW);
      }
    }
  } else {
    snext = false;
  }
}
void Add() {
  bool match = false;
  for (int i = 0; i < 15; i++) {
    if ( strcmp(rcv, list[i]) == 0) {
      match = true;
    }
  }
  if (match == false) {
    int rlen = 0;
    for (unsigned int i = 0; i < sizeof(rcv); i ++) {
      if (rcv[i] != 0) {
        rlen ++;
      }
    }
    char placeholder[rlen + 2] = "Y";
    strcat(placeholder, rcv);
    strcat(placeholder, "Z");
    for (int i = 0; i < rlen; i++) {
      list[cardnum][i] = rcv[i];
    }
    cardnum++;
    for (unsigned int i = 0; i < sizeof(placeholder); i++) {
      EEPROM.write(i + lengthv, placeholder[i]);
    }
    lengthv += sizeof(placeholder);

    flicker(7);
  }
}
void flicker(int pin) {
  programming = false;
  digitalWrite(pin, LOW);
  delay(150);
  digitalWrite(pin, HIGH);
  delay(150);
  digitalWrite(pin, LOW);
  delay(150);
  digitalWrite(pin, HIGH);
  delay(150);
  digitalWrite(pin, LOW);
}
void Sub() {
  bool match = false;
  int rcard = 0;
  for (int i = 0; i < 15; i++) {
    if ( strcmp(rcv, list[i]) == 0) {
      match = true;
      rcard = i;
    }
  }

  if (match == true) {
    int posn = 0;
    char megastring[350] = "";
    for (int i = 0; i < 15; i++) {
      if (i != rcard && list[i][0] != 0) {
        megastring[posn] = 89;
        posn ++;
        for (unsigned int s = 0; s < sizeof(list[i]); s++) {
          if (list[i][s] != 0) {
            megastring[posn] = list[i][s] ;
            posn ++;
          }
        }
        megastring[posn] = 90;
        posn++;
      }
    }
    memset(list[rcard], 0, sizeof(list[rcard]));
    for (unsigned int i = 0; i < EEPROM.length(); i++) {

      EEPROM.write(i, 255);
    }
    lengthv = 0;
    for (unsigned int i = 0; i < sizeof(megastring); i++) {
      if (megastring[i] != 0) {
        EEPROM.write(i, megastring[i]);
        lengthv ++;
      }
    }
    flicker(3);
  }
}
