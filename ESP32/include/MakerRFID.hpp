#ifndef MAKERRFID_HPP
#define MAKERRFID_HPP

#include <Arduino.h>
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

    Adafruit_SSD1306 GetDisplay();
    MFRC522 GetRFID();

    void SetWiFi(char*, char*);
    void StartWiFi();
    void EndWiFi();

    void StartSerial(int = 9600);
    void StartSPI();


    void StartRFID();
    void StopRFID();
    void SetKey(byte[]);
    void AuthenticateCard(int = 0);
    void DetectCard();
    void ReadingMessage();
    void PrintCardDetails();
    void ReadSector(byte*, int);
    void ReadSectors(byte*, int, int);
    void ReadAllSectors(byte*, int = 16);

    void StartDisplay();
    void ShowLogos(int = 2500);

    void DumpByteArray(byte *buffer, byte bufferSize);

    void PermissionMessage(bool);

  private:
    Adafruit_SSD1306 display_;
    MFRC522 rfid_;
    char* ssid_;
    char* password_;
    MFRC522::MIFARE_Key key_;
    MFRC522::StatusCode status_;
};

#endif