// Salvador Pérez del Pino
// ESP32 Debugger
#include <MakerRFID.hpp>

MakerRFID makerspace;
byte default_key[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
byte alternative_key[6] = {0x4D, 0x6F, 0x72, 0x67, 0x61, 0x6E};
byte buffer[34];
const unsigned dataBlock = 10;

#define KEYBOARD_R1_PIN 9
#define KEYBOARD_R2_PIN 8
#define KEYBOARD_R3_PIN 7
#define KEYBOARD_R4_PIN 6
#define KEYBOARD_C1_PIN 5
#define KEYBOARD_C2_PIN 4
#define KEYBOARD_C3_PIN 3
#define KEYBOARD_C4_PIN 2

// setup keypad
const byte ROWS = 4; 
const byte COLS = 4; 
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {KEYBOARD_R1_PIN, KEYBOARD_R2_PIN, KEYBOARD_R3_PIN,
    KEYBOARD_R4_PIN};
byte colPins[COLS] = {KEYBOARD_C1_PIN, KEYBOARD_C2_PIN, KEYBOARD_C3_PIN,
    KEYBOARD_C4_PIN}; 
Keypad keypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

void setup() {
    String ssid = "Phoenix";
    String password = 'aizc3525';

    makerspace.StartDisplay();
    makerspace.ShowLogos();
    
    pinMode(greenPin, OUTPUT);
    pinMode(redPin, OUTPUT);
    pinMode(relayPin, OUTPUT);
    // digitalWrite(greenPin, LOW);
    // digitalWrite(redPin, LOW);
    // digitalWrite(relayPin, LOW);

    makerspace.StartSerial();
    makerspace.StartSPI();
    makerspace.StartRFID();
    makerspace.StartWiFi(ssid, password);
    makerspace.SetKey(alternative_key);
    // Conectar con teclado
}

void loop() {
  
  // makerspace.waitForKeyboard();
  // makerspace.displayRequest();
  makerspace.DetectCard();
  makerspace.ReadingMessage();
  digitalWrite(greenPin, HIGH);
  makerspace.PrintCardDetails();

  makerspace.validateCard();

  makerspace.AuthenticateCard();

  makerspace.ReadSector(buffer, dataBlock);
  digitalWrite(greenPin, LOW);
  makerspace.readLockerFromKeyboard(keypad);
  
  // Llamada al server con los datos de la tarjeta
  if (makerspace.compareData(buffer) != "") {
    makerspace.PermissionMessage(true);
    digitalWrite(greenPin, HIGH);
    makerspace.openLocker();
  } else {
    makerspace.PermissionMessage(false);
    digitalWrite(redPin, HIGH);
  }
  
  makerspace.StopRFID();
}