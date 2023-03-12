#ifndef __MAKERRFID_HPP__
#define __MAKERRFID_HPP__

#include <Arduino.h>
#include <string>
#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "logos.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
// No tocar el 0 y el 1
#define SS_PIN          27
#define RST_PIN         26
#define SIZE_BUFFER     18
#define MAX_SIZE_BLOCK  16
#define greenPin        12
#define redPin          32
#define relayPin        33
#define PERM            "ARM1"

class MakerRFID {
  public:
    MakerRFID();

    // ####### GETTERS #######
    Adafruit_SSD1306 GetDisplay();
    MFRC522 GetRFID();

    // ####### SET UP #######
    void StartWiFi(char* ssid, char* password);
    // void SetWiFi(char*, char*);
    void StartSerial(int = 9600);
    void StartSPI();
    void SetKey(byte[]);
    // void EndWiFi(); // no hace nada
    void StartDisplay();
    void ShowLogos(int = 2500);


    // ####### Read card data #######
    void StartRFID();
    void StopRFID();
    void AuthenticateCard(int = 0);
    bool validateCard();
    void DetectCard();
    void ReadSector(byte*, int);
    void ReadSectors(byte*, int, int);
    void ReadAllSectors(byte*, int = 16);

    // ####### Write data on card #######
    void DumpByteArray(byte *buffer, byte bufferSize);

    // Comunication with server
    std::string compareData(char* buffer);

    // ####### Display information #######
    void ReadingMessage();
    void PrintCardDetails();

    // ####### Relés #######
    void PermissionMessage(bool);
    void readLockerFromKeyboard();
    void openLocker();
    
  private:
    Adafruit_SSD1306 display_; // Conexion con pantalla https://github.com/adafruit/Adafruit_SSD1306
    MFRC522 rfid_; // https://github.com/miguelbalboa/rfid
    // char* ssid_;
    // char* password_;
    MFRC522::MIFARE_Key key_;
    MFRC522::StatusCode status_;
    uint8_t locker_;
};

#endif