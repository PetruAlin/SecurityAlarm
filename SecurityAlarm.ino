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

#define FILE_WRITE (O_WRITE | O_READ | O_CREAT) // Scriere la inceput de fisier
#define PIR PD5
#define BUTTON PD2
#define BUZZER PD3
#define chipSelect 10
#define I2CPadModule 0x20
#define I2CLcdModule 0x27

// Alarm system mode
int mode = 0;

// Password
int passLength = 4;
char password[5];

// Bluetooth device
SoftwareSerial App(PD6, PD7);

// Lcd device
LiquidCrystal_I2C lcd(I2CLcdModule, 16, 2);
int lcd_reprint = 0;

// Keypad module
I2CKeyPad keypad(I2CPadModule);
char keys[] = "123A456B789C*0#DNF";
uint32_t start, stop;
uint32_t lastPressed = 0;

// Micro sd card variabiles;
//const int chipSelect = 10;
File myFile;

long ts = 0;

/* Function recalibrates PIR sensor, by waiting 1 minute */
void calibratePir(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Wait 1");
  lcd.setCursor(0, 1);
  lcd.print("minute");
  while (millis() - ts < 60000) {
    
  }
  ts = millis();
  Serial.println("Alarm active");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Alarm");
  lcd.setCursor(0, 1);
  lcd.print("active");
}

/* Function sets a 4 digit password in memory */
void setPassword() {
  Serial.print("Please set a 4 digit password: ");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter 4 digits:");
  int current = 0;
  while (current < passLength) {
    uint32_t now = millis();

    // Debouncing
    if (now - lastPressed >= 200)
    {
      lastPressed = now;

      start = micros();
      uint8_t ind;
      ind = keypad.getKey();
      stop = micros();
      if (ind >= 0 && ind < 16) { // If key is on the keypad
        password[current] = keys[ind];  // Save digit of password
        lcd.setCursor(current, 1);
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
  lcd.setCursor(0, 0);
  lcd.print("Set:");
}

/* Function reads password from micro-sd and saves it in memory */
void getPassword() {
  if (!SD.begin(chipSelect)) {
    Serial.println("Password intialization failed");
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Reading from");
  lcd.setCursor(0, 1);
  lcd.print("SD card");

  // open file
  myFile = SD.open("ALARMT~1.TXT");
  if (myFile) {
    // Read from the file until eof
    int i = 0;
    while (myFile.available()) {
      char c = myFile.read();
      password[i] = c;
      i++;
    }
    password[5] = '\0';
    
    // close the file
    myFile.close();
    Serial.println(password);
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening Alarm.txt");
  }
}

/* Function sets button */
void setButton() {
  DDRD &= ~(1 << BUTTON);
  PORTD |= (1 << BUTTON);
}

/* Function sets pins of PIR, Button and buzzer */
void setPins() {
  pinMode(PIR, INPUT);
  pinMode(BUZZER, OUTPUT);
  //BUTTON setup
  setButton();
}

/* Functions sets external interupt on rising edge for button press */
void setInterupts() {
  cli();
  // Interrupt setup
  EIMSK |= (1 << INT0);
  // On rising edge
  EICRA |= (1 << ISC01) | (1 << ISC00);
  //
  sei();
}

/* Function initializes LCD */
void initLCD() {
  lcd.init();  // initialize the lcd
  lcd.backlight();  // open the backlight
}

/* Functions writes password from memory to micro-sd card */
void writeToCard() {
  Serial.println("Saving new password...");
  myFile = SD.open("ALARMT~1.TXT", FILE_WRITE);
  int ret = myFile.seek(0);
  if (ret == 1) {
    myFile.write(password, 4);
  }
  myFile.close();
}

/* Functions resets the system password
 * If user choses to change password, new password
 * is saved both in memory and on the micro-sd card
 */
void reset() {
  Serial.println("Reset your password?");
  Serial.println("Yes(A), No(D)");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Reset password?");
  lcd.setCursor(0, 1);
  lcd.print("A(Y) or D(N)");
  int current = 0;
  while (current == 0) {
    uint32_t now = millis();
    if (now - lastPressed >= 200)
    {
      lastPressed = now;

      start = micros();
      uint8_t ind;
      ind = keypad.getKey();
      stop = micros();
      if (ind >= 0 && ind < 16) { // If key is available on the keypad
        Serial.println(keys[ind]);
        if (keys[ind] == 'A') { // Accept reset
          setPassword();  // Get new password and set in memory
          writeToCard();  // Save new password on micro-sd card 
          current = 1;
        } else if (keys[ind] == 'D') { // Decline reset
          current = 1;
        }
      }
    }
  }
}

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
  ts = millis();
  calibratePir();
  mode = 0;
  lcd_reprint = 0;
}

/* Interupt code for button press */
ISR(INT0_vect)
{
  if (millis() - ts > 300) {
    ts = millis();
    if (mode == 1) {  // Let user introduce password
      mode = 2;
    } else if (mode == 0) { // Let user reset password
      mode = 3;
    }
  }
}

/* Function accepts a 4 digit password from keypad, checks if is correct or not */
void introducePassword() {
  Serial.print("Password: ");
  int current = 0;
  char introduced[5];
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Enter password:");
  while (current < passLength) {
    uint32_t now = millis();
    // Debouncing
    if (now - lastPressed >= 200) {
      lastPressed = now;

      start = micros();
      uint8_t ind;
      ind = keypad.getKey();
      stop = micros();
      if (ind >= 0 && ind < 16) {// If the key is on the keypad
        introduced[current] = keys[ind]; // Saves introduced digit
        lcd.setCursor(current, 1);
        lcd.print(keys[ind]);
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
  // Check introduced password
  for (int i = 0; i < 4; i++) {
    if (introduced[i] != password[i]) {
      p = 0;
    }
  }
  if (p == 1) { // Password is corect
    Serial.println("Correct password");
    lcd.setCursor(0,1);
    lcd.print("Correct");
    while (millis() - ts < 1000) {

    }
    lcd_reprint = 0;
    mode = 0;
    calibratePir();
  } else {  // Password is wrong
    Serial.println("Wrong password");
    lcd.setCursor(0,1);
    lcd.print("Wrong ");
    App.write("Motion Detected!");
    while (millis() - ts < 1000) {

    }
    ts = millis();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Motion");
    lcd.setCursor(0, 1);
    lcd.print("Detected");
    mode = 1;
  }
}

void loop()
{
  if (mode == 0) {  // Active mode
    int p = 0;
    p = digitalRead(PIR);
    if (p == HIGH) {
      mode = 1;
      Serial.println("Motion detected!");
      if (lcd_reprint == 0) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Motion");
        lcd.setCursor(0, 1);
        lcd.print("Detected");
        lcd_reprint = 1;
      }
      App.write("Motion Detected!");
    }
  } else if (mode == 1) { // Motion detected mode
    tone(BUZZER, 100);
    delay(100);
    noTone(BUZZER);
    delay(100);
    // Search for bluetooth incoming data;
    if (App.available()) {
      char a = App.read();
      if(a - 48 == 0){
        mode = 0;
        lcd_reprint = 0;
        calibratePir();
      }
    }
    //
  } else if (mode == 2) { // Introduce password from console
    App.write("WAIT");
    introducePassword();
  } else if (mode == 3) { // Reset password
    App.write("WAIT");
    reset();
    mode = 0;
    lcd_reprint = 0;
    Serial.println("Alarm active");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Alarm");
    lcd.setCursor(0, 1);
    lcd.print("active");
  }
}
