/*
  First Configuration
  This sketch demonstrates the usage of MKR WAN 1300/1310 LoRa module.
  This example code is in the public domain.
*/

#include <MKRWAN.h>
#include <HardwareSerial.h>

// LoRa module pins for Serial2
#define	J14_5	35 // I35
#define	J14_6	27 // IO27
#define	J14_7	34 // I34
#define	J14_8	25 // IO25
#define	J14_9	26 // IO26
#define	J14_10	14 // IO14
#define	J14_11	13 // IO13
#define	J14_12	15 // IO15

#define LoRaRxPin    J14_5
#define LoRaLowPin   J14_6
#define LoRaTxPin    J14_8
#define LoRaBootPin  J14_11
#define LoRaResetPin J14_12
HardwareSerial SerialModem(2);

LoRaModem modem(SerialModem);

String appEui;
String appKey;
String devAddr;
String nwkSKey;
String appSKey;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  pinMode(LoRaLowPin, INPUT_PULLUP);

  pinMode(LoRaBootPin, OUTPUT);
  digitalWrite(LoRaBootPin, LOW);

  pinMode(LoRaResetPin, OUTPUT);
  digitalWrite(LoRaResetPin, HIGH);
  delay(200);
  digitalWrite(LoRaResetPin, LOW);
  delay(200);
  digitalWrite(LoRaResetPin, HIGH);
  delay(200);

  SerialModem.begin(19200, SERIAL_8N1, LoRaRxPin, LoRaTxPin);

  Serial.println("Welcome to MKRWAN1300/1310 first configuration sketch");
  Serial.println("Register to your favourite LoRa network and we are ready to go!");
  // change this to your regional band (eg. US915, AS923, ...)
  if (!modem.begin(EU868)) {
    Serial.println("Failed to start module");
    while (1) {}
  };
  Serial.print("Your module version is: ");
  Serial.println(modem.version());
  if (modem.version() != ARDUINO_FW_VERSION) {
    Serial.println("Please make sure that the latest modem firmware is installed.");
    Serial.println("To update the firmware upload the 'MKRWANFWUpdate_standalone.ino' sketch.");
  }
  Serial.print("Your device EUI is: ");
  Serial.println(modem.deviceEUI());

  int mode = 0;
  while (mode != 1 && mode != 2) {
    Serial.println("Are you connecting via OTAA (1) or ABP (2)?");
    while (!Serial.available());
    mode = Serial.readStringUntil('\n').toInt();
  }

  int connected;
  if (mode == 1) {
    Serial.println("Enter your APP EUI");
    while (!Serial.available());
    appEui = Serial.readStringUntil('\n');

    Serial.println("Enter your APP KEY");
    while (!Serial.available());
    appKey = Serial.readStringUntil('\n');

    appKey.trim();
    appEui.trim();

    connected = modem.joinOTAA(appEui, appKey);
  } else if (mode == 2) {

    Serial.println("Enter your Device Address");
    while (!Serial.available());
    devAddr = Serial.readStringUntil('\n');

    Serial.println("Enter your NWS KEY");
    while (!Serial.available());
    nwkSKey = Serial.readStringUntil('\n');

    Serial.println("Enter your APP SKEY");
    while (!Serial.available());
    appSKey = Serial.readStringUntil('\n');

    devAddr.trim();
    nwkSKey.trim();
    appSKey.trim();

    connected = modem.joinABP(devAddr, nwkSKey, appSKey);
  }

  if (!connected) {
    Serial.println("Something went wrong; are you indoor? Move near a window and retry");
    while (1) {}
  }

  delay(5000);

  int err;
  modem.setPort(3);
  modem.beginPacket();
  modem.print("HeLoRA world!");
  err = modem.endPacket(true);
  if (err > 0) {
    Serial.println("Message sent correctly!");
  } else {
    Serial.println("Error sending message :(");
  }
}

void loop() {
  while (modem.available()) {
    Serial.write(modem.read());
  }
  modem.poll();
}
