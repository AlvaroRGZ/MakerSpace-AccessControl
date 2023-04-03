/*
 * Salvador Pérez del Pino
 *
 * Revised by:
 * Álvaro Rodríguez Gómez 
 * Áram Pérez Dios
 * Marcos Barrios Lorenzo
 * 
 * ESP32 Card writer
 */
#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <HTTPClient.h>
#include "logos.h"
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

#define SCREEN_ADDRESS 0x3F ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
LiquidCrystal_I2C display(SCREEN_ADDRESS);

#define SS_PIN          27
#define RST_PIN         26
#define SIZE_BUFFER     18
#define MAX_SIZE_BLOCK  16

const char* ssid = "SSID DEL WIFI DE LA RASPBERRY";
const char* password = "CONTRASEÑA DEL WIFI DE LA RASPBERRY";

String serverName = "http://127.0.0.1/putdata.php?";

MFRC522::MIFARE_Key key;
MFRC522::StatusCode status;
MFRC522 mfrc522(SS_PIN, RST_PIN);

void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.println(buffer[i] < 0x10 ? " 0" : " ");
        Serial.println(buffer[i], HEX);
    }
}

void setup() {
  display.begin(20, 4);
  // display.clearDisplay();
  // display.drawBitmap(0, 0, logoCepsa, 120, 32, 1);
  // display.display();
  // delay(2500); // Pause for 2.5 seconds
  // display.clearDisplay();
  // display.drawBitmap(0, 0, logoULL, 120, 32, 1);
  // display.display();
  // delay(2500); // Pause for 2.5 seconds
  // display.clearDisplay();
  
  Serial.begin(9600);
  SPI.begin();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password); //Establece la conexión

  Serial.print("Conectando al WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
      Serial.print('.');
      delay(1000);
  }

  Serial.println(WiFi.localIP());
  Serial.print("RRSI: ");
  Serial.println(WiFi.RSSI());

  mfrc522.PCD_Init();
}

void loop() {
  byte newkeyAB[] = {0x4D, 0x6F, 0x72, 0x67, 0x61, 0x6E, 0xFF, 0x07, 0x80, 0x69, 0x4D, 0x6F, 0x72, 0x67, 0x61, 0x6E};

  //Comentar el bucle según el caso
  for (byte i = 0; i < 6; i++) key.keyByte[i] = newkeyAB[i]; //Para las ya establecidas
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF; //Para las nuevas

  //En caso de no haber tarjeta o no leerla, no continúa el proceso
  if (!mfrc522.PICC_IsNewCardPresent()){
    Serial.println("No hay tarjeta.");
    delay(1000);
    return;
  }
  
  if (!mfrc522.PICC_ReadCardSerial()){
    Serial.println("Numero de serie no disponible");
    delay(1000);
    return;
  }

  Serial.println("Tarjeta detectada.");

  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak); //Obtiene el tipo de tarjeta
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI
    &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
    &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Tarjeta no compatible :("));
    return;
  }
  Serial.println("Tarjeta compatible con el sistema");

  MFRC522::StatusCode status;

  byte buffer[34];  //PENDIENTE DE REVISAR QUÉ VALORES DEBE TENER EL BUFFER
  byte sector = 0;
  byte block = 2;
  byte trailerBlock =  3;

  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
      Serial.print(F("PCD_Authenticate() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return;
  }

  Serial.println(F("Datos actuales del sector: "));
  mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
  Serial.println();

  //Generación de la contraseña
  byte hexChar[16]; //Todos los caracteres en hexadecimal
  String pass = "";
  for (int i = 0; i < 16; i++) {
    //Genera un carácter aleatorio
    //randomSeed(); //genera una seed aleatoria con un pin flotante
    hexChar[i] = byte(random(0, 255));
    pass += hexChar[i];
  }
  
  //Escritura de la contraseña
  status = mfrc522.MIFARE_Write(block, hexChar, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.println(F("Escritura fallida :("));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else{
    Serial.println(F("¡Escritura realizada correctamente! "));
    for(int i = 0; i < 16; i++){
      Serial.print(hexChar[i], HEX);
    }
  }
  Serial.println();

  //Escritura de la key nueva
  status = mfrc522.MIFARE_Write(trailerBlock, newkeyAB, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.println(F("Escritura de key fallida :("));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("¡Nueva key configurada correctamente! "));

  String userid;
  for (int i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    userid += String(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();
  Serial.println("UID:");
  Serial.println(userid);

  //Subida al servidor 
  HTTPClient http;
  serverName += "uid=" + userid + "&pass=" + pass;
  http.begin(serverName);
  int httpCode = http.GET();
  String httpResult = http.getString();
  if(httpResult){
    Serial.println("¡Subida al servidor correcta!");
  }else{
    Serial.println("Error en la subida, comprueba la conexión y contacta con el administrador.");
  }

  Serial.println(httpCode);
  Serial.println("");
  mfrc522.PICC_HaltA(); // Halt PICC
  mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
}