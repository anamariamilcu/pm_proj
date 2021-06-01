// Include required libraries
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>
#include <SPI.h>

#define GREEN_LED 7
#define RED_LED 6
#define SERVO_PIN 8
#define BUZZ_PIN 5

// Constants for row and column sizes
const byte ROWS = 4;
const byte COLS = 4;
 
// Array to represent keys on keypad
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// * -> delete a key
// # -> enter password
 
// Connections to Arduino
byte rowPins[ROWS] = {A0, A1, A2, A3};
byte colPins[COLS] = {4, 3, 2, 1};
 
// Create keypad object
Keypad kp = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// Create LCD object
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Create Servo object
Servo sg90;

char correct_pass[4] = {'2', '7', '4', '0'};
char password[4];
uint8_t i;
uint8_t pass_done;


void setup() {
  // put your setup code here, to run once:
  // Setup serial monitor
  Serial.begin(9600);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZ_PIN, OUTPUT);

  lcd.init();
  lcd.backlight();
  lcd.clear();

  lcd.print("Enter Password");
  lcd.setCursor(0, 1);

  // Set pin for servo
  sg90.attach(SERVO_PIN);
  // Initial position to 0
  sg90.write(0);

}

void deleteLastKey(int i) {

  lcd.setCursor(i, 1);
  lcd.print(" ");
  lcd.setCursor(i, 1);
}

void loop() {
  // put your main code here, to run repeatedly:
  // Get key value if pressed
  char key = kp.getKey();

  
  if (key){
    if (key == '*')
    {
      if (i > 0) {
        i--;
        deleteLastKey(i);
      }
    } else if (key == '#') {
      pass_done = 1;
    } else if (i < 4) {
      Serial.print(key);
      // Clear LCD display and print character
      lcd.print(key);
      password[i] = key;
      i++;
    }
  }
  if (i == 4 && pass_done){
    // reset counter
    i = 0;
    // reset flag
    pass_done = 0;
    delay(1000);
    // If password is matched
    if (!(strncmp(password, correct_pass, 4))) 
      {
        lcd.clear();
        lcd.print("Access permitted");
        // Door opens
        sg90.write(90);
        digitalWrite(GREEN_LED, HIGH);
        delay(5000);
        digitalWrite(GREEN_LED, LOW);
        lcd.clear();
        // Door closes
        sg90.write(0);
      }
      // If password is not matched
      else
      {
        lcd.clear();
        lcd.print("Access denied");
        digitalWrite(RED_LED, HIGH);
        tone(BUZZ_PIN, 1000);
        delay(5000);
        digitalWrite(RED_LED, LOW);
        noTone(BUZZ_PIN);
        lcd.clear();
      }
     lcd.print("Enter Password");
     lcd.setCursor(0, 1);
  }

}
