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

// Create MFRC522
MFRC522 rfid(10, 9);

// Allowed tags to unlock
const byte NO_TAGS = 1;
String tag_uids[NO_TAGS] = {"9A A5 D4 BF" /*, "75 14 69 21"*/};

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

  lcd.print("Scan tag");
  lcd.setCursor(0, 1);

  // Set pin for servo
  sg90.attach(SERVO_PIN);
  // Initial position to 0
  sg90.write(0);
  
  // Init SPI bus
  SPI.begin();

  // Init MFRC522
  rfid.PCD_Init();

}

byte searchTagUID(String tag) {
  for (uint8_t j = 0; j < NO_TAGS; j++)
  {
    if (tag.substring(1) == tag_uids[j])
      return 1;
  }
  return 0;
}


void deleteLastKey(int i) {
  // remove last typed key from LCD
  lcd.setCursor(i, 1);
  lcd.print(" ");
  lcd.setCursor(i, 1);
}

void loop() {
  // put your main code here, to run repeatedly:

    if (!rfid.PICC_IsNewCardPresent())
    {
      return;
    }
    // Select one of the cards
    if (!rfid.PICC_ReadCardSerial())
    {
      return;
    }
   //Reading from the card
   String tag = "";
   for (byte j = 0; j < rfid.uid.size; j++)
   {
     tag.concat(String(rfid.uid.uidByte[j] < 0x10 ? " 0" : " "));
     tag.concat(String(rfid.uid.uidByte[j], HEX));
   }
   tag.toUpperCase();
   Serial.print(tag);
   if (!searchTagUID(tag))
   {
    // Do not allow access
   }

    
  
  // Get key value if pressed
  char key = kp.getKey();

  
  if (key)
  {
    if (key == '*')
    {
      if (i > 0)
      {
        i--;
        deleteLastKey(i);
      }
    } else if (key == '#' && i == 4)
    {
      pass_done = 1;
    }
    else if (i < 4)
    {
      Serial.print(key);
      // Print character
      lcd.print(key);
      password[i] = key;
      i++;
    }
  }

  // action after password is entered
  if (i == 4 && pass_done)
  {
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
