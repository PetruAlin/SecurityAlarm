// Keypad
#include <I2CKeyPad.h>
#include <Keypad.h>
// SD card
#include <SPI.h>
#include <SD.h>
// LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
// Bluetooth
#include <SoftwareSerial.h>

#define FILE_WRITE (O_WRITE | O_READ | O_CREAT) // Pentru scriere de la inceput
#define PIR 5 // 5
#define BUTTON PD2 // 2 
#define BUZZER PD3 // 4 -> 3

#define I2CPadModule 0x20
#define PIR_RESET 3000

int mode = 0;
SoftwareSerial App(6, 7); // RX | TX
LiquidCrystal_I2C lcd(0x27, 16, 2);
I2CKeyPad keypad(I2CPadModule);
char keys[] = "123A456B789C*0#DNF";
uint32_t start, stop;
uint32_t lastKeyPressed = 0;

int lcd_reprint = 0;

const int chipSelect = 10;
File myFile;
char password[5];
int passLength = 4;

void setPassword() {
  Serial.print("Please set a 4 digit password: ");
  lcd.clear();
  int current = 0;
  while (current < passLength) {
    uint32_t now = millis();
    // Create a variable named key of type char to hold the characters pressed
    if (now - lastKeyPressed >= 200)
    {
      lastKeyPressed = now;

      start = micros();
      uint8_t ind;
      ind = keypad.getKey();
      stop = micros();
      if (ind >= 0 && ind < 16) {// if the key variable contains
        password[current] = keys[ind]; // output characters from Serial Monitor
        lcd.setCursor(current, 0);
        lcd.print(keys[ind]);
        current++;
        if (current != 4) {
          Serial.print(keys[ind]);
        } else {
          Serial.println(keys[ind]);
        }
      }
    }
  }
  password[passLength] = '\0';
  Serial.print("Password set: ");
  Serial.println(password);
  lcd.setCursor(0, 1);
  lcd.print("Set");
}

void getPassword() {
  if (!SD.begin(chipSelect)) {
    Serial.println("Password intialization failed");
  }
  // LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Reading");
  lcd.setCursor(0, 1);
  lcd.print("SD card");
  //
  myFile = SD.open("ALARMT~1.TXT");
  if (myFile) {
    // read from the file until there's nothing else in it:
    int i = 0;
    while (myFile.available()) {
      char c = myFile.read();
      password[i] = c;
      i++;
    }
    password[5] = '\0';
    // close the file:
    myFile.close();
    Serial.println(password);
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening Alarm.txt");
  }
}

void setButton() {
  DDRD &= ~(1 << BUTTON);
  PORTD |= (1 << BUTTON);
}


void setPins() {
  pinMode(PIR, INPUT);
  pinMode(BUZZER, OUTPUT);
  //BUTTON setup
  setButton();
}

void setInterupts() {
  cli();
  //Interrupt setup
  EIMSK |= (1 << INT0);
  // On the rising edge

  EICRA |= (1 << ISC01) | (1 << ISC00);
  //
  sei();
}

void writeToCard() {
  Serial.println("Saving new password...");
  myFile = SD.open("ALARMT~1.TXT", FILE_WRITE);
  int ret = myFile.seek(0);
  if (ret == 1) {
    myFile.write(password, 4);
  }
  myFile.close();
}

void reset() {
  Serial.println("Reset your password?");
  Serial.println("Yes(A), No(D)");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Reset?");
  lcd.setCursor(0, 1);
  lcd.print("A or D");
  //int p = 0;
  int current = 0;
  while (current == 0) {
    // Create a variable named key of type char to hold the characters pressed
    uint32_t now = millis();
    if (now - lastKeyPressed >= 200)
    {
      lastKeyPressed = now;

      start = micros();
      uint8_t ind;
      ind = keypad.getKey();
      stop = micros();
      if (ind >= 0 && ind < 16) {
        Serial.println(keys[ind]);
        if (keys[ind] == 'A') {
          //p = 1;
          setPassword();
          writeToCard();
          current = 1;
        } else if (keys[ind] == 'D') {
          //p = 0;
          current = 1;
        }
      }
    }
  }
}

void initLCD() {
  lcd.init();  //initialize the lcd
  lcd.backlight();  //open the backlight
}

long ts = 0;
long pt = 0;

void setup()
{
  setPins();
  setInterupts();
  initLCD();
  Serial.begin(9600);
  App.begin(9600);
  getPassword();
  Wire.begin();
  Wire.setClock(400000);
  if (keypad.begin() == false)
  {
    Serial.println("\nERROR: cannot communicate to keypad.\nPlease reboot.\n");
    while (1);
  }
  reset();
  while (millis() - ts < 60000) {
    //ts = millis();
  }
  ts = millis();
  Serial.println("Alarm starting");
  // lcd
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Alarm");
  lcd.setCursor(0, 1);
  lcd.print("active");
  mode = 0;
}

ISR(INT0_vect)
{
  if (millis() - ts > 300) {
    ts = millis();
    if (mode == 1) {
      mode = 2;
    } else if (mode == 0) {
      mode = 3;
    }
  }
}

void introducePassword() {
  Serial.print("Password: ");
  int current = 0;
  char introduced[5];
  while (current < passLength) {
    uint32_t now = millis();
    // Create a variable named key of type char to hold the characters pressed
    if (now - lastKeyPressed >= 200) {
      lastKeyPressed = now;

      start = micros();
      uint8_t ind;
      ind = keypad.getKey();
      stop = micros();
      if (ind >= 0 && ind < 16) {// if the key variable contains
        introduced[current] = keys[ind]; // output characters from Serial Monitor
        current++;
        if (current != 4)
          Serial.print(keys[ind]);
        else
          Serial.println(keys[ind]);
      }
    }
  }
  password[passLength] = '\0';
  int p = 1;
  for (int i = 0; i < 4; i++) {
    if (introduced[i] != password[i]) {
      p = 0;
    }
  }
  if (p == 1) {
    Serial.println("Correct password");
    int lcd_reprint = 0;
    lcd.clear();
    mode = 0;
    //ts = millis();
    while (millis() - ts < 60000) {

    }
    lcd.setCursor(0, 0);
    lcd.print("Alarm");
    lcd.setCursor(0, 1);
    lcd.print("active");
  } else {
    Serial.println("Wrong password");
    mode = 1;
  }
}

void loop()
{
  //Serial.println(mode);
  if (mode == 0) {
    int p = 0;
    //if (millis() - pt > PIR_RESET) {
    //pt = millis();
    p = digitalRead(PIR);
    //}
    if (p == HIGH) {
      mode = 1;
      Serial.println("Motion detected!");
      if (lcd_reprint == 0) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Motion");
        lcd.setCursor(1, 0);
        lcd.print("Detected");
        lcd_reprint = 1;
      }
      App.write("Motion Detected!");
    }
  } else if (mode == 1) {
    pt = millis();
    digitalWrite(BUZZER, HIGH);
    delay(100);
    digitalWrite(BUZZER, LOW);
    delay(100);
    // Search for bluetooth incoming data;
    if (App.available()) {
      char a = App.read();
      if(a - 48 == 0){
        mode = 2;
      }
    }
    //
  } else if (mode == 2) {
    introducePassword();
    pt = millis();
  } else if (mode == 3) {
    reset();
    mode = 0;
  }

}
