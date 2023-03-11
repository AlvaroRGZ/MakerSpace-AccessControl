// Salvador Pérez del Pino
// ESP32 Debugger
#include "MakerRFID.hpp"

//String serverName = "http://127.0.0.1/getdata.php";

MFRC522::MIFARE_Key key;
MFRC522::StatusCode status;
MFRC522 mfrc522(SS_PIN, RST_PIN);



MakerRFID makerspace;
byte default_key[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
byte alternative_key[6] = {0x4D, 0x6F, 0x72, 0x67, 0x61, 0x6E};
byte buffer[34];

void setup() {
    char* ssid = "A";
    char* password = ";";

    makerspace.StartDisplay();
    makerspace.ShowLogos();
    
    pinMode(greenPin, OUTPUT);
    pinMode(redPin, OUTPUT);
    pinMode(relayPin, OUTPUT);
    digitalWrite(greenPin, LOW);
    digitalWrite(redPin, LOW);
    digitalWrite(relayPin, LOW);

    makerspace.StartSerial();
    makerspace.StartSPI();
    // makerspace.SetWiFi(ssid, password);
    makerspace.StartWiFi(ssid, password);
    makerspace.SetKey(alternative_key);
    // Conectar con teclado
}

void loop(){
  
  // makerspace.waitForKeyboard();
  // makerspace.displayRequest();
  makerspace.DetectCard();
  makerspace.ReadingMessage();
  digitalWrite(greenPin, HIGH);
  digitalWrite(redPin, HIGH);
  makerspace.PrintCardDetails();

  makerspace.validateCard();

  makerspace.AuthenticateCard();

  makerspace.ReadSector(buffer, 2);
  
  // Llamada al server con los datos de la tarjeta
  if (!makerspace.compareData(buffer) == "") {
    makerspace.displayConfirmation();
    makerspace.openLocker();
  } else {
    makerspace.displayCancelation();
  }
  
  makerspace.StopRFID();
}