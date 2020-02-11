#include <Arduino.h>
#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>

// telia sim card at the moment, check for your own apn settings
// https://www.telia.fi/asiakastuki/liittymat/puhelimen-kaytto/mms-internet-asetukset
const char apn[] = "internet";
const char gprsUser[] = "";
const char gprsPass[] = "";

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

const char server[] = "my.tampere.hacklab.fi";
const char resource[] = "/auth/phonenumber";
const int port = 443;

TinyGsmClientSecure connection(modem);
HttpClient client(connection, server, port);

// tmp string for reading from the modem
String s;

// our chipid
String chipId = String((uint32_t)ESP.getEfuseMac(), HEX);

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

  if (!modem.hasSSL())
  {
    Serial.println("ERROR: this model does not support SSL communications, going to sleep forever");
    while (1)
    {
      delay(1000);
    };
  }

  // set ate1 for echo for easier debugging
  Serial.println("Activating echo for modem serial comm.");
  Serial1.println("ATE1");

  // set clip on so that we get the number
  Serial.println("Activating clip to get caller number.");
  Serial1.println("AT+CLIP=1");

  // start gprs connection
  Serial.println("Starting GPRS connections. This might take a while...");
  if (!modem.gprsConnect(apn, gprsUser, gprsPass))
  {
    Serial.println("GPRS connection failed, restarting after 30 seconds!");
    sleep(30);
    ESP.restart();
  }
  sleep(3);
  if (modem.isGprsConnected())
  {
    Serial.println("GPRS connection started");
  }

  Serial.println("Now call me :)");
}

// read ser1 and forward to ser2 and vice versa
// usefull for trying out different AT commands
// from serial monitos
void loopBackDebug()
{
  while (Serial1.available())
  {
    Serial.write(Serial1.read());
  }
  while (Serial.available())
  {
    Serial1.write(Serial.read());
  }
}

// wait for a call and when one happens
// capture the number
// hangup
// return the number (or empty if number cannot be figured out)
String ListenForIncomingCall()
{
  // everytime cleanup before checking
  s = "";
  String number = "";
  while (Serial1.available())
  {
    // read the buffer until newline
    s = Serial1.readStringUntil('\n');
    // Serial.println(s);
    if (s.startsWith("+CLIP: "))
    {
      // flush everythin else from the serial buffer
      Serial.print("Call coming in: ");
      Serial.println(s);
      Serial.println("Hanging up");

      int start = s.indexOf('"') + 1;
      int end = s.indexOf('"', start);

      // find the number
      number = s.substring(start, end);

      modem.callHangup();
      Serial1.flush(); // seems like the modem library does not do this
    }
  }
  return number;
}

void loop()
{
  //loopBackDebug();
  String number = ListenForIncomingCall();
  if (!number.isEmpty())
  {
    Serial.print("And we have a number: ");
    Serial.println(number);

    if (modem.isGprsConnected())
    {
      Serial.println("GPRS connection is active, which is nice");
    }

    // now ask from mulysa if this number is ok or not
    String payload = "{\"";
    payload.concat("deviceid\":\"");
    payload.concat(chipId);
    payload.concat("\",\"");
    payload.concat("number\":\"");
    payload.concat(number);
    payload.concat("\"}");

    Serial.println("Starting http request");
    client.beginRequest();
    client.post(resource, "application/json", payload);
    client.endRequest();

    if(client.responseStatusCode() == 200) {
      Serial.println("Access granted");
    } else {
      Serial.println("Nope, no access");
    }

  }
}