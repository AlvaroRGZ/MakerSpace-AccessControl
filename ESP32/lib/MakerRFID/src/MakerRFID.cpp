#include "MakerRFID.hpp"

MakerRFID::MakerRFID(): display_(SCREEN_ADDRESS, 20, 4) {
  conn_ = NULL;
  rfid_ = MFRC522(SS_PIN, RST_PIN);
}

MakerRFID::~MakerRFID() {
  if (conn_ != nullptr) {
    delete conn_;
  }
}

LiquidCrystal_I2C MakerRFID::GetDisplay() {
  return display_;
}

MFRC522 MakerRFID::GetRFID() {
  return rfid_;
}

void MakerRFID::StartWiFi(String ssid, String password) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str()); //Establece la conexión

  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }

  Serial.println(WiFi.localIP());
  Serial.print("RRSI: ");
  Serial.println(WiFi.RSSI());
  Serial.println("SALGO DE WIFI");
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
  int trailerBlock = 11;
  if (keytype == 0) {
    status_ = rfid_.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key_, &(rfid_.uid));
    if (status_ != MFRC522::STATUS_OK) {
      Serial.print(F("PCD_Authenticate() failed: "));
      Serial.println(rfid_.GetStatusCodeName(status_));
    }
  } else if (keytype == 1) {
    status_ = rfid_.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &key_, &(rfid_.uid));
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
    Serial.println(F("Tarjeta no compatible :("));
    return false;
  }
  Serial.println("Tarjeta compatible con el sistema.");
  return true;
}

void MakerRFID::StartDisplay() {
  Serial.println("Start display() called.");
  display_.init();
  display_.clear();
  display_.backlight();
  display_.setCursor(2, 0);
  display_.print("Pantalla encendida.");
}

void MakerRFID::ShowLogos(int delay_time) {
  // This no longer uses Adafruit so a drawBitmap substitute is needed
  //
  // display_.clearDisplay();
  // display_.drawBitmap(0, 0, logoCepsa, 120, 32, 1);
  // display_.display();
  // delay(delay_time); // Pause for 2.5 seconds
  // display_.clearDisplay();
  // display_.drawBitmap(0, 0, logoULL, 120, 32, 1);
  // display_.display();
  // delay(delay_time); // Pause for 2.5 seconds
  // display_.clearDisplay();
}

void MakerRFID::DetectCard() {
  while (!rfid_.PICC_IsNewCardPresent()) {
    Serial.println("No se ha detectado la tarjeta.");
    digitalWrite(greenPin, LOW);
    digitalWrite(redPin, LOW);
    display_.print("Acerca la tarjeta.");
    delay(2000);
  }

  while (!rfid_.PICC_ReadCardSerial()) {}
}

void MakerRFID::ReadingMessage() {
  display_.clear();
  display_.print("Leyendo...");
  Serial.println("Tarjeta detectada.");
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
    Serial.println((buffer[i] < 0x10) ? " 0" : " ");
    Serial.println(buffer[i], HEX);
  }
}

void MakerRFID::ReadSector(byte* buffer, int blockAddr) {
  byte size = 32;
  status_ = rfid_.MIFARE_Read(blockAddr, buffer, &size);
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
    status_ = rfid_.MIFARE_Read(blockAddr, buffer, &size);
    if (status_ != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Read() failed: "));
      Serial.println(rfid_.GetStatusCodeName(status_));
    }
    Serial.print(F("Data in block ")); 
    Serial.print(blockAddr); 
    Serial.println(F(":"));
    DumpByteArray(buffer, 32); 
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
    display_.clear();
    display_.print("Permiso concedido.");
  } else {
    display_.clear();
    display_.print("Permisos insuficientes.");
  }
}

// Comunication with server
// send uid, password and locker to server for it to check if valid.
String MakerRFID::compareData(byte* buffer) {
  String serverName = "http://192.168.241.15:8080/paginas/reply_comparation.php?";
  // UID
  String uidString;
  for (uint8_t i = 0; i < rfid_.uid.size; i++) {
    uidString += String(rfid_.uid.uidByte[i], HEX);// std::string(rfid_.uid.uidByte[i], sizeof(rfid_.uid.uidByte[i]));
  }
  String user = "uid=" + uidString;
  // for (uint8_t i = 0; i < sizeof(buffer); i++) {
  //   pass += std::string(buffer[i], sizeof(buffer[i]));
  // }
  // PASSWORD
  //char* passData;
  //memcpy(passData, buffer, 16);
  String password = "psswd=" + String((char*)buffer);
  // LOCKER
  String locker = "locker=" + String(locker_);
  // COMBINE REQUEST
  String request = serverName + user + "&" + password + "&" + locker;
  // char* request = new char[Srequest.length() + 1];
  // strcpy(request, Srequest.c_str());

  // QUERY
  String output;
  HTTPClient http;
  http.begin(request.c_str());
  int httpCode = http.GET();
  if(httpCode > 0){
    output = http.getString();
    Serial.print("Nombre de usuario: "); 
    Serial.println(output);
  } else {
    display_.print("[ERROR] Server request failed. Aborting...");
  }
  
  // Si no se recibe nada devuelve una cadena vacia
  return output;
}

void MakerRFID::readLockerFromKeyboard(Keypad &keypad) {
  // char pressedKey = keypad.waitForKey();
  locker_ = 1;// uint8_t(pressedKey);
}

void MakerRFID::openLocker() {
  delay(1000);
  digitalWrite(boozerPin, HIGH);
  digitalWrite(relay1Pin, HIGH);
  delay(5000);
  digitalWrite(relay1Pin, LOW);
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

void MakerRFID::setKey(byte* buffer) {
  for (uint8_t i = 0; i < 16; ++i) {
    key_.keyByte[i] = buffer[i];
  }
}

byte* MakerRFID::generatePassword() {
  byte password[16];
  for (uint8_t i = 0; i < 16; ++i) {
    // TODO: true random
    password[i] = byte(random(0, 255));
  }
  Serial.println("Generada nueva contraseña aleatoria.");
  return password;
}

void MakerRFID::writePassword(byte* password, uint8_t block) {
  MFRC522::StatusCode status = rfid_.MIFARE_Write(block, password, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.println(F("Fallo de escritura de la contraseña en la tarjeta."));
    Serial.println(rfid_.GetStatusCodeName(status));
  } else {
    Serial.println(F("Contraseña escrita con éxito en la tarjeta."));
  }
}

void MakerRFID::sendPacket(std::string serverAddress, byte* passwordBuffer) {
  // std::string serverName = "http://127.0.0.1/getdata.php?";
  String uid = "uid=";
  for (uint8_t i = 0; i < rfid_.uid.size; i++) {
    uid += String(rfid_.uid.uidByte[i], HEX);
  }
  // 
  // char* passData;
  // memcpy(passData, passwordBuffer, sizeof(passwordBuffer));
  // std::string password = "psswd=" + std::string(passData);
// 
  // PASSWORD
  //char* passData;
  //memcpy(passData, buffer, 16);
  String password = "psswd=passx";//String((char*)passwordBuffer);
  for (uint8_t i = 0; i < 16; i++) {
    uid += String(passwordBuffer[i], HEX);
  }
  // LOCKER
  String locker = "locker=" + String(locker_);
  // COMBINE REQUEST
  // String request = serverName + user + "&" + password + "&" + locker;
  String request = "http://10.42.0.1:8080/paginas/reply_comparation.php?";//serverAddress + uid + "&" + password;
  request += uid;
  request += "&";
  request += password;
  request += "&";
  request += locker;

  Serial.println(request);
  String output;
  HTTPClient http;
  http.begin(request.c_str());
  int httpCode = http.GET();
  if(httpCode > 0) {
    Serial.print("Bienvenido ");
    Serial.println(http.getString());

  } else {
    Serial.println("Error en la subida, comprueba la conexión y contacta con el administrador.");
  }
}

void MakerRFID::connectDB(void) { // Se supone inicializado en el setup tras startWifi()
  Serial.println("voy a con");
  delay(1000);
  conn_ = new PGconnection(&client_, 0, 1024, dbBuffer_);
  if (conn_ == NULL) {
    Serial.println("conn_ is a nullptr");
  }
  delay(1000);
  Serial.println("hice con");
  delay(1000);
  IPAddress PGIP(10,159,5,105); // IP del servidor de la base de datos

  conn_->setDbLogin(PGIP, "postgres", "postgres", "makerspacecontrol", "utf8", 5432);
  if (conn_->status() == CONNECTION_BAD) {
    Serial.println("setDbLogin() failed");
    Serial.print("CODE: ");
    Serial.println(conn_->status());
  } else {
    Serial.println("Conexión a la base de datos establecida.");
  }
}

String MakerRFID::executeQuery(byte* Buffer) {
  // std::string serverName = "http://127.0.0.1/getdata.php?";
  String uid = "uid=";
  for (uint8_t i = 0; i < rfid_.uid.size; i++) {
    uid += (char*)(rfid_.uid.uidByte[i], sizeof(rfid_.uid.uidByte[i]));
  }
  
  char* passData;
  memcpy(passData, Buffer, sizeof(Buffer));
  String password = "psswd=" + String(passData);

  String query =  String("SELECT nombre FROM usuarios NATURAL JOIN permisos ");
         query += String("WHERE uid = '" + uid + "' AND passwd = '" + password + "' ");
         query += String("AND armario_" + char(locker_));
         query += String(" = 1;");

  String output;
  conn_->execute(query.c_str());
  if(conn_->getData() > 0) {
    Serial.println("¡Subida al servidor correcta!");
  } else {
    Serial.println("Error en la subida, comprueba la conexión y contacta con el administrador.");
  }
}
