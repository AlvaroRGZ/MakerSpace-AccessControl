//Salvador Pérez del Pino
//Programa para escribir los datos de usuario en el ESP32
#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HTTPClient.h>
#include "logos.h"

#include <Keypad.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 displayReader(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define SS_PIN          27
#define RST_PIN         26
#define SIZE_BUFFER     18
#define MAX_SIZE_BLOCK  16
#define greenPin        12
#define redPin          32
#define relayPin        33

// Temporal pins, to be checked with the electronic team
// after arduino assembly
#define KEYBOARD_R1_PIN 9
#define KEYBOARD_R2_PIN 8
#define KEYBOARD_R3_PIN 7
#define KEYBOARD_R4_PIN 6
#define KEYBOARD_C1_PIN 5
#define KEYBOARD_C2_PIN 4
#define KEYBOARD_C3_PIN 3
#define KEYBOARD_C4_PIN 2

const char* ssidReader = "SSID RASPBERRY";
const char* passwordReader = "CONTRASEÑA RASPBERRY";

String serverNameReader = "http://127.0.0.1/getdata.php?";

MFRC522::MIFARE_Key keyReader;
MFRC522::StatusCode statusReader;
MFRC522 mfrc522Reader(SS_PIN, RST_PIN);

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

void dump_byte_array(byte *buffer, byte bufferSize, String resultado) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.println(buffer[i] < 0x10 ? " 0" : " ");
    Serial.println(buffer[i], HEX);
    resultado += String(buffer[i]);
  }
}

void setup() {
  if(!displayReader.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  displayReader.clearDisplay();
  displayReader.drawBitmap(0, 0, logoCepsa, 120, 32, 1);
  displayReader.display();
  delay(2500); // Pause for 2.5 seconds
  displayReader.clearDisplay();
  displayReader.drawBitmap(0, 0, logoULL, 120, 32, 1);
  displayReader.display();
  delay(2500); // Pause for 2.5 seconds
  displayReader.clearDisplay();
  
  pinMode(greenPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(greenPin, LOW);
  digitalWrite(redPin, LOW);
  //Serial.begin(9600);
  SPI.begin();
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssidReader, passwordReader); //Establece la conexión
  

  //Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
  //  Serial.print('.');
    delay(1000);
  }

  //Serial.println(WiFi.localIP());
  //Serial.print("RRSI: ");
  //Serial.println(WiFi.RSSI());

  // setup keypad

  mfrc522Reader.PCD_Init();
}


void loop() {
  
  byte newkeyAB[] = {0x4D, 0x6F, 0x72, 0x67, 0x61, 0x6E, 0xFF, 0x07, 0x80, 0x69, 0x4D, 0x6F, 0x72, 0x67, 0x61, 0x6E};
  for (byte i = 0; i < 6; i++) keyReader.keyByte[i] = newkeyAB[i]; //Para las ya establecidas

  //En caso de no haber tarjeta o no leerla, no continúa el proceso
  if (!mfrc522Reader.PICC_IsNewCardPresent()){
    digitalWrite(greenPin, LOW);
    digitalWrite(redPin, LOW);
    displayReader.clearDisplay();
    displayReader.setCursor(0, 0);
    displayReader.setTextColor(WHITE);
    displayReader.setTextSize(2);
    displayReader.println("Acerca la");
    displayReader.print("tarjeta");
    displayReader.display();
    delay(1000);
    //Serial.println("No hay tarjeta");
    //delay(1000);
    return;
  }
  
  if (!mfrc522Reader.PICC_ReadCardSerial()){
    //Serial.println("Numero de serie no disponible");
    //delay(1000);
    return;
  }

  displayReader.clearDisplay();
  displayReader.setCursor(0, 0);
  displayReader.setTextColor(WHITE);
  displayReader.setTextSize(2);
  displayReader.println("Leyendo...");
  displayReader.display();
  //Serial.println("Tarjeta detectada");

  MFRC522::PICC_Type piccType = mfrc522Reader.PICC_GetType(mfrc522Reader.uid.sak); //Obtiene el tipo de tarjeta
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI
    &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
    &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    //Serial.println(F("Tarjeta no compatible :("));
    return;
  }


  MFRC522::StatusCode status;

  byte buffer[34];
  byte size = *buffer;
  byte sector = 0;
  byte block = 2;
  byte trailerBlock =  3;

  status = (MFRC522::StatusCode) mfrc522Reader.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &keyReader, &(mfrc522Reader.uid));
  if (status != MFRC522::STATUS_OK) {
    //Serial.print(F("PCD_Authenticate() failed: "));
    //Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = (MFRC522::StatusCode) mfrc522Reader.MIFARE_Read(block, buffer, &size);
  if(status != MFRC522::STATUS_OK) {
    //Serial.print(F("MIFARE_Read() failed: "));
    //Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  String result = "";
  dump_byte_array(buffer, 16, result);
  serverNameReader += "pass=" + result;

  block = 0;
  status = (MFRC522::StatusCode) mfrc522Reader.MIFARE_Read(block, buffer, &size);
  if(status != MFRC522::STATUS_OK) {
    //Serial.print(F("MIFARE_Read() failed: "));
    //Serial.println(mfrc522.GetStatusCodeName(status));
  }

  String uid = "";
  dump_byte_array(buffer, 4, uid);
  serverNameReader += "&uid=" + uid;

  String locker = "";
  char pressedKey = keypad.waitForKey();
  locker = String(pressedKey);
  serverNameReader += "&locker=" + locker;

  HTTPClient http;
  http.begin(serverNameReader);
  int httpCode = http.GET();
  bool allowed = false;
  if(httpCode > 0){
    allowed = http.getString();
  }
  else{
    displayReader.clearDisplay();
    displayReader.setCursor(0, 0);
    displayReader.setTextColor(WHITE);
    displayReader.setTextSize(2);
    displayReader.println("No hay conexión");
    displayReader.display();
    digitalWrite(redPin, HIGH);
    delay(3000);
    digitalWrite(redPin, LOW);
    return;
  }
  if(allowed){
    digitalWrite(relayPin, HIGH);
    digitalWrite(greenPin, HIGH);
    displayReader.clearDisplay();
    displayReader.setCursor(0, 0);
    displayReader.setTextColor(WHITE);
    displayReader.setTextSize(2);
    displayReader.println("¡Acceso concedido!");
    displayReader.print("¡Bienvenid@, Maker!");
    displayReader.display();
    delay(2000);
    digitalWrite(relayPin, LOW);
    digitalWrite(greenPin, LOW);
  }
}