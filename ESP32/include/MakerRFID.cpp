#include "MakerRFID.hpp"

MakerRFID::MakerRFID() {
  rfid_ = MFRC522(SS_PIN, RST_PIN);
}

Adafruit_SSD1306 MakerRFID::GetDisplay() {
  return display_;
}


MFRC522 MakerRFID::GetRFID() {
  return rfid_;
}

/*
void MakerRFID::SetWiFi(char* ssid, char* password) {
  ssid_ = ssid;
  password_ = password;
}
*/

void MakerRFID::StartWiFi(char* ssid, char* password) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password); //Establece la conexión

  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }

  Serial.println(WiFi.localIP());
  Serial.print("RRSI: ");
  Serial.println(WiFi.RSSI());
}

void MakerRFID::StartSerial(int baudrate) {
  Serial.begin(baudrate);
}

void MakerRFID::StartSPI() {
  SPI.begin();
}

void MakerRFID::StartRFID() {
  rfid_.PCD_Init();
}

void MakerRFID::StopRFID() {
  rfid_.PICC_HaltA(); // Halt PICC
  rfid_.PCD_StopCrypto1();  // Stop encryption on PCD
}

void MakerRFID::AuthenticateCard(int keytype) {
  // Type 0: Key A
  // Type 1: Key B
  int trailerBlock = 3;
  if (keytype == 0) {
    status_ = (MFRC522::StatusCode) rfid_.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key_, &(rfid_.uid));
    if (status_ != MFRC522::STATUS_OK) {
      Serial.print(F("PCD_Authenticate() failed: "));
      Serial.println(rfid_.GetStatusCodeName(status_));
    }
  } else if (keytype == 1) {
    status_ = (MFRC522::StatusCode) rfid_.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &key_, &(rfid_.uid));
    if (status_ != MFRC522::STATUS_OK) {
      Serial.print(F("PCD_Authenticate() failed: "));
      Serial.println(rfid_.GetStatusCodeName(status_));
    }
  }
}

// Comprueba que la tarjeta leida sea compatible con el sistema
bool MakerRFID::validateCard(void) {
  MFRC522::PICC_Type piccType = rfid_.PICC_GetType(rfid_.uid.sak); //Obtiene el tipo de tarjeta
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI
    &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
    &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    // Serial.println(F("Tarjeta no compatible :("));
    return false;
  }
  // Serial.println("Tarjeta compatible con el sistema");
  return true;
}

void MakerRFID::StartDisplay() {
  if (!display_.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever | estaa en la version antigua
  }
}

void MakerRFID::ShowLogos(int delay_time) {
  display_.clearDisplay();
  display_.drawBitmap(0, 0, logoCepsa, 120, 32, 1);
  display_.display();
  delay(delay_time); // Pause for 2.5 seconds
  display_.clearDisplay();
  display_.drawBitmap(0, 0, logoULL, 120, 32, 1);
  display_.display();
  delay(delay_time); // Pause for 2.5 seconds
  display_.clearDisplay();
}

void MakerRFID::DetectCard() {
  while (!rfid_.PICC_IsNewCardPresent()) {
    //Serial.println("No se detecta tarjeta");
    digitalWrite(greenPin, LOW);
    digitalWrite(redPin, LOW);
    display_.clearDisplay();
    display_.setCursor(0, 0);
    display_.setTextColor(WHITE);
    display_.setTextSize(2);
    display_.println("Acerca la");
    display_.print("tarjeta");
    display_.display();
    delay(1000);
  } 

  while (!rfid_.PICC_ReadCardSerial()) {}

}

void MakerRFID::ReadingMessage() {
  display_.clearDisplay();
  display_.setCursor(0, 0);
  display_.setTextColor(WHITE);
  display_.setTextSize(2);
  display_.print("Leyendo...");
  display_.display();
  Serial.println("Tarjeta detectada");
}

void MakerRFID::PrintCardDetails() {
  Serial.println("Detalles: ");
  rfid_.PICC_DumpDetailsToSerial(&(rfid_.uid));
}

void MakerRFID::SetKey(byte newkey[]) {
  for (byte i = 0; i < 6; i++) { 
    key_.keyByte[i] = newkey[i];
  }
}

void MakerRFID::DumpByteArray(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.println(buffer[i] < 0x10 ? " 0" : " ");
    Serial.println(buffer[i], HEX);
  }
}

void MakerRFID::ReadSector(byte* buffer, int sector) {
  int blockAddr = sector;
  byte size = sizeof(buffer);
  status_ = (MFRC522::StatusCode) rfid_.MIFARE_Read(blockAddr, buffer, &size);
  if (status_ != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Read() failed: "));
    Serial.println(rfid_.GetStatusCodeName(status_));
  }
  Serial.print(F("Data in block ")); 
  Serial.print(blockAddr); 
  Serial.println(F(":"));
  DumpByteArray(buffer, 16); 
  Serial.println();
}

void MakerRFID::ReadSectors(byte* buffer, int start, int finish) {
  //TODO: Incluir lectura en buffer de varios sectores
  byte size = sizeof(buffer);
  for (int i = start; i <= finish; i++) {
    int blockAddr = i;
    status_ = (MFRC522::StatusCode) rfid_.MIFARE_Read(blockAddr, buffer, &size);
    if (status_ != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Read() failed: "));
      Serial.println(rfid_.GetStatusCodeName(status_));
    }
    Serial.print(F("Data in block ")); 
    Serial.print(blockAddr); 
    Serial.println(F(":"));
    DumpByteArray(buffer, 16); 
    Serial.println();
  }
}

void MakerRFID::ReadAllSectors(byte* buffer, int max_sectors) {
  this->ReadSectors(buffer, 0, 16);
}

// Muestra la informacion por el display y acciona el relé
// Deberiamos poder determinar que rele abrir
void MakerRFID::PermissionMessage(bool has_permission) {
  if (has_permission) {
    display_.clearDisplay();
    display_.setCursor(0, 0);
    display_.setTextColor(WHITE);
    display_.setTextSize(2);
    display_.print("Permiso concedido.");
    display_.display();
    digitalWrite(relayPin, HIGH);
    delay(5000);
    digitalWrite(relayPin, LOW);

  } else {
    display_.clearDisplay();
    display_.setCursor(0, 0);
    display_.setTextColor(WHITE);
    display_.setTextSize(2);
    display_.print("Permisos insuficientes.");
    display_.display();
  }
}

// Comunication with server
// send uid, password and locker to server for it to check if valid.
String MakerRFID::compareData(byte* buffer) {
  String serverName = "http://127.0.0.1/getdata.php?";
  String uidString = "";
  for (uint8_t i = 0; i < rfid_.uid.size; i++) {
    uidString += String(rfid_.uid.uidByte[i]);
  }
  String user = "user=" + uidString;
  String password = "pass=";
  for (uint8_t i = 0; i < sizeof(buffer); i++) {
    password += String(buffer[i]);
  }
  String locker = "locker=" + String(locker_);
  String request = serverName + user + "&" + password + "&" + locker;

  String output;
  HTTPClient http;
  http.begin(request);
  int httpCode = http.GET();
  if(httpCode > 0 && http.getString()){
    output = http.getString();
  } else {
    display_.println("[ERROR] Server request failed. Aborting...");
  }
  // Si no se recibe nada devuelve una cadena vacia
  return output;
}

void MakerRFID::openLocker() {
  /*
    switch (locker_) { 
      case 0:
        digitalWrite(LOCKER_0_PIN);
        break;
      case 1:
        digitalWrite(LOCKER_1_PIN);
        break;
      case 2:
        digitalWrite(LOCKER_2_PIN);
        break;
      case 3:
        digitalWrite(LOCKER_3_PIN);
        break;
      (...)
    }
  */
}

void readLockerFromKeyboard() {
  // display "please enter locker number on keyboard"

  // using Keypad.h arduino library
  // https://github.com/Chris--A/Keypad

  // add keypad_ as private field?

  // uint8_t locker = 0;
  // char pressedKey = keypad_.waitForKey();
  // locker = uint8_t(pressedKey);

}
