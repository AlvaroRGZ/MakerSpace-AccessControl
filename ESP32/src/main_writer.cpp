// Salvador Pérez del Pino
// ESP32 Debugger
#include <MakerRFID.hpp>

const unsigned dataBlock = 10;
const std::string serverAddress = "http://127.0.0.1/paginas/reply_comparation.php?";

// TODO: set wifi credentials
// TODO: set server address

MakerRFID makerspace;
byte default_key[16] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
byte alternative_key[16] = {0x4D, 0x6F, 0x72, 0x67, 0x61, 0x6E, 0xFF, 0x07, 0x80, 0x69, 0x4D, 0x6F, 0x72, 0x67, 0x61, 0x6E};
byte buffer[34];

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
}

void loop() {
  
  // makerspace.waitForKeyboard();
  // makerspace.displayRequest();
  makerspace.DetectCard();
  makerspace.ReadingMessage();
  digitalWrite(greenPin, HIGH);
  digitalWrite(redPin, HIGH);
  makerspace.PrintCardDetails();

  makerspace.validateCard();

  makerspace.AuthenticateCard();

  byte newPassword[16];
  makerspace.generatePassword(newPassword);
  makerspace.writePassword(newPassword, dataBlock);
  if ((makerspace.registerNewCard(newPassword)) != "") {
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

