#include <Arduino.h>
#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>

// telia sim card at the moment, check for your own apn settings
// https://www.telia.fi/asiakastuki/liittymat/puhelimen-kaytto/mms-internet-asetukset
const char apn[] = "internet";

// esp32 t-call pins
#define MODEM_RST 5
#define MODEM_PWKEY 4
#define MODEM_POWER_ON 23
#define MODEM_TX 27
#define MODEM_RX 26

//#define DUMP_AT_COMMANDS

// for debugging AT commands
#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(Serial1, Serial);
TinyGsm modem(debugger);
#else
TinyGsm modem(Serial1);
#endif

// tmp string for reading from the modem
String s;

void setup()
{
  // for debugging
  Serial.begin(115200);
  delay(100);
  Serial.println("");
  Serial.println("Starting up!");

  // set pins for modem
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);

  // start serial for modem
  Serial1.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(500);

  Serial.println("Initializing modem, please wait. this takes a while...");
  if (modem.restart())
  {
    Serial.println("Modem restarted.");
  }

  String modemInfo = modem.getModemInfo();
  Serial.print("Modem initialized: ");
  Serial.println(modemInfo);
}

// read ser1 and forward to ser2 and vice versa
// usefull for trying out different AT commands
// from serial monitos
void loopBackDebug() {
  while (Serial1.available()) {
    Serial.write(Serial1.read());
  }
  while (Serial.available()) {
    Serial1.write(Serial.read());
  }
}

void loop()
{
  loopBackDebug();

  // wait for data from the modem
/*
  while (Serial1.available())
  {
    s = Serial1.readString();
    Serial.println(s);
  }
  if (s == "RING")
  {
    Serial.println("Incoming call");
    s = "";
    modem.callHangup();
    Serial.println("Hangup");
  }
  */
}