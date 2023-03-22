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

#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

#include <SimplePgSQL.h>

#define SCREEN_ADDRESS 0x3F

// No tocar el 0 y el 1
#define SS_PIN          26
#define RST_PIN         27
#define SIZE_BUFFER     18
#define MAX_SIZE_BLOCK  16
#define greenPin        12
#define redPin          32
#define relayPin        33
#define greenLED        14
#define PERM            "ARM1"
#define randomSeedPin   

/**
 * (Relés son interruptores)
 * RELÉ1 -> D33
 * RELÉ2 -> D25
 * RELÉ3 -> D13
 * BUZZER -> D12
 * PANTALLA -> D22, D21
 * LED R -> D32
 * LED V -> D14
 */

/**
 * D15, D2, D4, RX2, TX2, D5
 * 
 * D12, D14 (SOLAPAMIENTO)
 */

class MakerRFID {
  public:
    MakerRFID();

    // ####### GETTERS #######
    hd44780_I2Cexp GetDisplay();
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

    void setKey(byte* buffer);
    byte* generatePassword();
    void writePassword(byte* password, uint8_t block);
    void initializeDBConnection(char* buffer);
    void sendPacket(std::string serverAddress, byte* password);
    
  private:
    hd44780_I2Cexp display_;
    MFRC522 rfid_; // https://github.com/miguelbalboa/rfid
    
    PGconnection connection_;
    // char* ssid_;
    // char* password_;
    MFRC522::MIFARE_Key key_;
    MFRC522::StatusCode status_;
    uint8_t locker_;
};

#endif