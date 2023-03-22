// Salvador Pérez del Pino
// ESP32 Debugger
#include <MakerRFID.hpp>

MakerRFID makerspace;
byte default_key[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
byte alternative_key[6] = {0x4D, 0x6F, 0x72, 0x67, 0x61, 0x6E};
const unsigned dataBlock = 10;

void setup() {
    String ssid = "makerspace-controlserver";
    String password = "meikspeis";

    makerspace.StartDisplay();
    makerspace.ShowLogos();
    
    pinMode(greenPin, OUTPUT);
    pinMode(redPin, OUTPUT);
    pinMode(relay1Pin, OUTPUT);
    pinMode(relay2Pin, OUTPUT);
    pinMode(relay3Pin, OUTPUT);
    pinMode(boozerPin, OUTPUT);
    digitalWrite(boozerPin, HIGH);
    // digitalWrite(greenPin, LOW);
    // digitalWrite(redPin, LOW);
    // digitalWrite(relayPin, LOW);

    makerspace.StartSerial();
    makerspace.StartSPI();
    makerspace.StartRFID();
    makerspace.StartWiFi(ssid, password);
    makerspace.SetKey(default_key);
    Serial.println("ANTES DE CONNECT");
    // makerspace.connectDB();
    Serial.println("FUEEEEEEEEEEEEEEEEERA");
    // Conectar con teclado
}

void loop() {
  
  byte buffer[32];
  // makerspace.waitForKeyboard();
  // makerspace.displayRequest();
  makerspace.DetectCard();
  makerspace.ReadingMessage();
  //digitalWrite(greenPin, HIGH);
  makerspace.PrintCardDetails();

  makerspace.validateCard();

  makerspace.AuthenticateCard();

  makerspace.ReadSector(buffer, dataBlock);
  // digitalWrite(greenPin, LOW);
  // makerspace.readLockerFromKeyboard(keypad);
  
  // Llamada al server con los datos de la tarjeta
  if (makerspace.entryRequest(buffer) == "") {
    makerspace.PermissionMessage(true);
    digitalWrite(greenPin, HIGH);
    digitalWrite(boozerPin, LOW);
    makerspace.openLocker();
  } else {
    makerspace.PermissionMessage(false);
    digitalWrite(redPin, HIGH);
    delay(1000);
  }
  
  makerspace.StopRFID();
}
