#ifndef __MAKERRFID_HPP__
#define __MAKERRFID_HPP__

#include <Arduino.h>
#include <string>
#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <HTTPClient.h>
#include "logos.h"
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>

#define SCREEN_ADDRESS 0x3F

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
    LiquidCrystal_I2C GetDisplay();
    MFRC522 GetRFID();

    // ####### SET UP #######
    void StartWiFi(String ssid, String password);
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
    void ReadSector(byte*, int blockAddr);
    void ReadSectors(byte*, int, int);
    void ReadAllSectors(byte*, int = 16);

    // ####### Write data on card #######
    void DumpByteArray(byte *buffer, byte bufferSize);

    // Comunication with server
    String compareData(byte* buffer);

    // ####### Display information #######
    void ReadingMessage();
    void PrintCardDetails();

    // ####### Relés #######
    void PermissionMessage(bool);
    void readLockerFromKeyboard(Keypad &keypad);
    void openLocker();
    
  private:
    LiquidCrystal_I2C display_;
    MFRC522 rfid_; // https://github.com/miguelbalboa/rfid
    // char* ssid_;
    // char* password_;
    MFRC522::MIFARE_Key key_;
    MFRC522::StatusCode status_;
    uint8_t locker_;
};

#endif