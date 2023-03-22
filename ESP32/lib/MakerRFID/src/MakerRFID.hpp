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



#include "../../SimplePgSQL/src/SimplePgSQL.h"

#define SCREEN_ADDRESS 0x3F

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

// No tocar el 0 y el 1
#define SS_PIN          26
#define RST_PIN         27
#define SIZE_BUFFER     18
#define MAX_SIZE_BLOCK  16
#define greenPin        14
#define redPin          32

#define boozerPin       12
#define relay1Pin       33
#define relay2Pin       25
#define relay3Pin       13
#define PERM            "ARM1"
#define randomSeedPin   

#define SDAPin       21
#define SSLPin       22

class MakerRFID {
  public:
    MakerRFID();
    ~MakerRFID();

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

    // ####### DB operations #######

    void connectDB(void);
    String executeQuery(byte* Buffer);

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
    void sendPacket(std::string serverAddress, byte* password);
    
  private:
    hd44780_I2Cexp display_;
    MFRC522 rfid_; // https://github.com/miguelbalboa/rfid
    
    // char* ssid_;
    // char* password_;
    MFRC522::MIFARE_Key key_;
    MFRC522::StatusCode status_;
    uint8_t locker_;
    PGconnection* conn_;
    char dbBuffer_[1024]; // Internal buffer for ddb connection
    WiFiClient client_;
};  


#endif