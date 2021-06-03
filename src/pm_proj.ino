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
#define SS_PIN 10
#define RST_PIN 9

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
MFRC522 rfid(SS_PIN, RST_PIN);

// Allowed tags to unlock
byte state;
const byte NO_MAX_TAGS = 5;
byte NO_TAGS = 1;
String tag_uids[NO_MAX_TAGS] = {"9A A5 D4 BF" /* , "75 14 69 21"*/};
// Used for reading current tag UID
String tag;
// Register new tag
boolean reg_new_tag = false;

char correct_pass[4] = {'2', '7', '4', '0'};
char password[4];
uint8_t count;
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

  lcd.print("Scan Tag");

  // Set pin for servo
  sg90.attach(SERVO_PIN);
  // Initial position to 0
  sg90.write(0);
  
  // Init SPI bus
  SPI.begin();

  // Init MFRC522
  rfid.PCD_Init();
  // search for tag firstly
  state = 1;

}

byte searchTagUID() {
  Serial.print("search:");
  for (uint8_t j = 0; j < NO_TAGS; j++)
  {
    Serial.print(tag_uids[j]);
    Serial.print("\n");
    if (tag == tag_uids[j])
      return 1;
  }
  return 0;
}


void deleteLastKey() {
  // remove last typed key from LCD
  count--;
  lcd.setCursor(count, 1);
  lcd.print(" ");
  lcd.setCursor(count, 1);
}


void allowTag() {
  lcd.clear();
  lcd.print("Tag is allowed");
  digitalWrite(GREEN_LED, HIGH);
  delay(3000);
  digitalWrite(GREEN_LED, LOW);
  lcd.clear();

  lcd.print("Enter Password");
  lcd.setCursor(0, 1);
  state = 2;
}

void denyTag() {
  lcd.clear();
  lcd.print("Tag unregistered");
  digitalWrite(RED_LED, HIGH);
  tone(BUZZ_PIN, 1000);
  delay(3000);
  digitalWrite(RED_LED, LOW);
  noTone(BUZZ_PIN);
  lcd.clear();
  lcd.print("Register new tag?");
  lcd.setCursor(0, 1);
  lcd.print("1-yes,other-no");
  state = 3;
}

void permitAccess() {
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

void denyAccess() {
  lcd.clear();
  lcd.print("Access denied");
  digitalWrite(RED_LED, HIGH);
  tone(BUZZ_PIN, 1000);
  delay(5000);
  digitalWrite(RED_LED, LOW);
  noTone(BUZZ_PIN);
  lcd.clear();
}

void readPassword() {
  // Get key value if pressed
  char key = kp.getKey();
  
  if (key)
  {
    if (key == '*' && count > 0)
    {
      deleteLastKey();
    }
    else if (key == '#' && count == 4)
    {
      pass_done = 1;
    }
    else if (count < 4)
    {
      Serial.print(key);
      // Print character
      lcd.print(key);
      password[count] = key;
      count++;
    }
  }
}

void registerNewTag() {
  lcd.clear();
  lcd.print("Correct password");
  lcd.setCursor(0, 1);
  lcd.print("Tag registered");
  // save new tag
  tag_uids[NO_TAGS] = tag;
  NO_TAGS++;
  // Allowed tags now:
  Serial.print("Allowed tags now:");
  for (uint8_t j = 0; j < NO_TAGS; j++)
  {
    Serial.print(tag_uids[j]);
  }
  Serial.print("\n");
  
  digitalWrite(GREEN_LED, HIGH);
  delay(3000);
  digitalWrite(GREEN_LED, LOW);
  lcd.clear();
}

void denyRegisteringNewTag() {
  lcd.clear();
  lcd.print("Wrong password");
  lcd.setCursor(0, 1);
  lcd.print("Tag not registered");
  digitalWrite(RED_LED, HIGH);
  tone(BUZZ_PIN, 1000);
  delay(3000);
  digitalWrite(RED_LED, LOW);
  noTone(BUZZ_PIN);
  lcd.clear();
}

void loop() {
  // put your main code here, to run repeatedly:

    // search for rfid tag
    if (state == 1) 
    {
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
      tag = "";
      for (byte j = 0; j < rfid.uid.size; j++)
      {
        tag.concat(String(rfid.uid.uidByte[j] < 0x10 ? " 0" : " "));
        tag.concat(String(rfid.uid.uidByte[j], HEX));
      }
      tag.toUpperCase();
      tag = tag.substring(1);
      Serial.print(tag);
      if (!searchTagUID())
      {
        // Do not allow access
        denyTag();
      } else
      {
        // Allow access
        allowTag();
      }
    }

  // verify password for registering new card
  if (state == 3)
  {
    // option not chosen yet
    if (!reg_new_tag)
    {
      char key = kp.getKey();
  
      if (key)
      {
        // 1 - register new tag
        if (key == '1')
        {
          reg_new_tag = true;
          lcd.clear();
          lcd.print("Enter Password");
          lcd.setCursor(0, 1);
        }
        // other key - don't register
        else
        {
          state = 1;
          lcd.clear();
          lcd.print("Scan Tag");
        }
      }
    }
    // option chosen - register the new tag
    else
    {
      readPassword();
      if (count == 4 && pass_done)
      {
        // reset counter
        count = 0;
        // reset flag
        pass_done = 0;
        delay(1000);
        // If password is matched
        if (!(strncmp(password, correct_pass, 4))) 
        {
          registerNewTag();
        }
        else
        {
          denyRegisteringNewTag();
        }
        state = 1;
        lcd.clear();
        lcd.print("Scan Tag");
      }
    }
  }

  // verify password
  if (state == 2)
  {
    readPassword();

    // action after password is entered
    if (count == 4 && pass_done)
    {
      // reset counter
      count = 0;
      // reset flag
      pass_done = 0;
      delay(1000);
      // If password is matched
      if (!(strncmp(password, correct_pass, 4))) 
      {
        permitAccess();
      }
      // If password is not matched
      else
      {
        denyAccess();
      }
      lcd.print("Scan tag");
      state = 1;
    }
  } 
}
