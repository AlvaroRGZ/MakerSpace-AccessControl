// Salvador Pérez del Pino
// ESP32 Debugger
#include <MakerRFID.hpp>

const unsigned dataBlock = 10;
const std::string serverAddress = "http://127.0.0.1/paginas/reply_comparation.php?";

// TODO: set wifi credentials
// TODO: set server address

MakerRFID makerspace;
byte default_key[16] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
byte alternative_key[16] = {0x4D, 0x6F, 0x72, 0x67, 0x61, 0x6E, 0xFF, 0x07, 0x80, 0x69, 0x4D, 0x6F, 0x72, 0x67, 0x61, 0x6E};
byte buffer[34];
U8X8_SSD1306_128X32_UNIVISION_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);   // Adafruit ESP8266/32u4/ARM Boards + FeatherWing OLED

void setup() {
    String ssid = "makerspace-controlserver";
    String password = "meikspeis";

    makerspace.StartDisplay();
    makerspace.ShowLogos();
    
    pinMode(greenPin, OUTPUT);
    pinMode(redPin, OUTPUT);
    pinMode(relay1Pin, OUTPUT);
    pinMode(relay2Pin, OUTPUT);
    pinMode(relay3Pin, OUTPUT);
    pinMode(boozerPin, OUTPUT);
    digitalWrite(boozerPin, HIGH);
    // digitalWrite(greenPin, LOW);
    // digitalWrite(redPin, LOW);
    // digitalWrite(relayPin, LOW);

    makerspace.StartSerial();
    makerspace.StartSPI();
    makerspace.StartRFID();
    makerspace.StartWiFi(ssid, password);
    makerspace.SetKey(default_key);
    u8x8.begin(); // Arduboy 10 (Produ
}

void loop() {
  
  // makerspace.waitForKeyboard();
  // makerspace.displayRequest();
  makerspace.DetectCard();
  makerspace.ReadingMessage();
  digitalWrite(greenPin, HIGH);
  digitalWrite(redPin, HIGH);
  makerspace.PrintCardDetails();

  makerspace.validateCard();

  makerspace.AuthenticateCard();

  byte newPassword[16];

  if (makerspace.doesUserExist()) {
    makerspace.displayMessage("Ya esiste ese usuario");
     u8x8.setFont(u8x8_font_chroma48medium8_r);
    String d = "Hellafghrehrehnrehrhrh";
    u8x8.drawString(0,0,d.c_str());
    u8x8.refreshDisplay();		// only required for SSD1606/7  
    delay(2000);
  } else {
    makerspace.displayMessage("No existe ese usuario");
  }
  

  /*
  if (!makerspace.doesUserExist()) {
    makerspace.generatePassword(newPassword);
    makerspace.writePassword(newPassword, dataBlock);
    if ((makerspace.registerNewCard(newPassword)) != "") {
      // makerspace.PermissionMessage(true);
      makerspace.displayMessage("Nuevo usuario registrado con exito");
      digitalWrite(greenPin, HIGH);
      digitalWrite(boozerPin, LOW);
      //makerspace.openLocker();
    } else {
      makerspace.displayMessage("Se ha detectado duplicacion en la BBDD");
      digitalWrite(redPin, HIGH);
      delay(1000);
    }
  } else {
    makerspace.displayMessage("Usuario ya registrado. Abortando");
    digitalWrite(redPin, HIGH);
    delay(1000);
  }
  */
  makerspace.StopRFID();
}

