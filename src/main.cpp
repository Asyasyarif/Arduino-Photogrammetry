

/*
 *
 *  Created by Arif
 *  Bandung, September, 15 2020
 *
 */




#include <Arduino.h>
#include <LiquidCrystal.h>
#include <TimeAlarms.h>
#include <EEPROM.h>
#include <TinyStepper.h>

const String VERSION = "0.0.2";

// Uncomment DEBUG_DONG line for debug
#define DEBUG_DONG

#define BAUDRATE 115200
#define MAX_DELAY 15
#define MAX_ROTATION 30
#define MAX_FRAME 200
#define DELAY_BEFORE_SHUTTER_RELEASE 50
#define DELAY_AFTER_SHUTTER_RELEASE 1000

#define TRIGGER_AUDIO_JACK 0
#define TRIGGER_BT_SERIAL 1 // bluetooth
String commandShutterBT = "#trigger#";
#define TRIGGER_MANUAL_BUTTON 2

AlarmId id;
AlarmId shutterId;
int readKey;
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

#define PIN_STEPPER_IN_1 3
#define PIN_STEPPER_IN_2 11
#define PIN_STEPPER_IN_3 12
#define PIN_STEPPER_IN_4 13
#define NYALA HIGH
#define MATI LOW
#define HALFSTEPS 4096
TinyStepper stepper(HALFSTEPS, PIN_STEPPER_IN_1, PIN_STEPPER_IN_2, PIN_STEPPER_IN_3, PIN_STEPPER_IN_4);

int menuPage = 0;
String menuItems[]    = {"START CAPTURE", "START SHOWCASE" ,"PRESETS", "SET TRIGGER", "SETTINGS", "ABOUT"};
int maxMenuPages = round(((sizeof(menuItems) / sizeof(String)) / 2) + .5);

int triggerMenu = 0;
String subTriggerMenu[]  = {"JACK 3.5mm", "Bluetooth", "Manual Button"};
int maxSubMenuTrigger = round(((sizeof(subTriggerMenu) / sizeof(String)) / 2) + .5);

int presetMenu = 0;
String subPresetMenu[] = {"Low", "Medium", "High"};
int maxSubMenuPresets = round(((sizeof(subPresetMenu) / sizeof(String)) / 2) + .5);

int settingsMenu = 0;
String subSettingsMenu[] = {"Preset Low", "Preset Medium", "Preset High"};
int maxSubMenuSettings = round(((sizeof(subSettingsMenu) / sizeof(String)) / 2) + .5);

int settingPresetMenu = 0;
String settingPresetMenuItems[] = {"Delay", "Rotation", "Frames"};
int maxsettingPresetMenuItems = round(((sizeof(settingPresetMenuItems) / sizeof(String)) / 2) + .5);

int cursorPosition,subCursorPosition = 0;
bool enteringSubMenu, isReadyCapture, nextFrame, doneStep = false;
bool shutterReleaseReady = true;
int selectMenu;
int result = 0;


byte addressPreset  = 11;
byte addressTrigger = 12;
byte addressPresetLowDelay    = 13;
byte addressPresetLowRotation     = 14;
byte addressPresetLowFrame    = 15;

byte addressPresetMedDelay    = 16;
byte adaddressPresetMedRotation     = 17;
byte addressPresetMedFrame    = 18;

byte addressPresetHighDelay   = 19;
byte addressPresetHighRotation    = 20;
byte addressPresetHighFrame   = 21;

unsigned int valueSelectedPreset, valueSelectedTrigger, currentFrame, 
          valuePresetLowDelay, valuePresetLowRotation, valuePresetLowFrame, 
          valuePresetMedDelay, valuePresetMedRotation, valuePresetMedFrame,
          valuePresetHighDelay, valuePresetHighRotation, valuePresetHighFrame;

int loadDelay, loadStep, loadFrame = 0;
int loadTriggerInput;

#define PIN_SHUTTER_JACK_AUDIO 2
#define PIN_FOCUS 8

// byte valuePresetLow[3] = {addressPresetLowDelay, addressPresetLowRotation, addressPresetLowFrame};

byte downArrow[8] = {
  B00100,
  B00100,
  B00100,
  B00100,
  B00100,
  B11111,
  B01110,
  B00100
};

byte upArrow[8] = {
  B00100,
  B01110,
  B11111,
  B00100,
  B00100,
  B00100,
  B00100,
  B00100
};

byte menuCursor[8] = {
  B01000,
  B00100,
  B00110,
  B01111,
  B01111,
  B00110,
  B00100,
  B01000
};

byte enterCursor[8] = {
  B10000,
  B10000,
  B10100,
  B10110,
  B11111,
  B00110,
  B00100,
  B00000
};

byte shutter[] = {
  B00000,
  B00000,
  B01000,
  B11111,
  B11001,
  B11001,
  B11111,
  B00000
};

byte check[8] = {
  0b00000,
  0b00001,
  0b00011,
  0b10110,
  0b11100,
  0b01000,
  0b00000,
  0b00000
};

byte heart[8] = {
  0b00000,
  0b01010,
  0b11111,
  0b11111,
  0b01110,
  0b00100,
  0b00000,
  0b00000
};

byte degrees[] = {
  B01000,
  B10100,
  B01000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

void software_Reset(){
  asm volatile ("  jmp 0");  
}


void readEEPROM(){
  Serial.println(F("Loading EEPROM"));
  valueSelectedPreset   = EEPROM.read(addressPreset);
  valueSelectedTrigger  = EEPROM.read(addressTrigger);
  valuePresetLowDelay = EEPROM.read(addressPresetLowDelay);
  valuePresetLowRotation = EEPROM.read(addressPresetLowRotation);
  valuePresetLowFrame = EEPROM.read(addressPresetLowFrame);


  valuePresetMedDelay = EEPROM.read(addressPresetMedDelay);
  valuePresetMedFrame = EEPROM.read(addressPresetMedFrame);
  valuePresetMedRotation  = EEPROM.read(adaddressPresetMedRotation);

  valuePresetHighDelay = EEPROM.read(addressPresetHighDelay);
  valuePresetHighFrame = EEPROM.read(addressPresetHighFrame);
  valuePresetHighRotation  = EEPROM.read(addressPresetHighRotation);
}

void initTimer(){

}



void setup() {
  Serial.begin(BAUDRATE);
  lcd.begin(16, 2);
  // EEPROM.begin();
  
  lcd.createChar(0, menuCursor);
  lcd.createChar(1, upArrow);
  lcd.createChar(2, downArrow);
  lcd.createChar(3, enterCursor);
  lcd.createChar(4, check);
  lcd.createChar(5, heart);
  lcd.createChar(6, degrees);
  lcd.createChar(7, shutter);

  lcd.setCursor(0,0);
  lcd.print("Starting...");
  Serial.println(F("Starting..."));
  lcd.setCursor(0,1);
  lcd.print("Please Wait");
  readEEPROM();
  initTimer();
  pinMode(PIN_SHUTTER_JACK_AUDIO, OUTPUT);
  pinMode(PIN_FOCUS, OUTPUT);
  delay(1000);
  Serial.println("Ready");
  lcd.clear();
  Serial.println(F(""));
  Serial.println(F(""));
  // homeMenu();

}


// This function is called whenever a button press is evaluated. The LCD shield works by observing a voltage drop across the buttons all hooked up to A0.
int evaluateButton(int x) {
  int result = 0;
  if (x < 50) {
    result = 1; // right
  } else if (x < 195) {
    result = 2; // up
  } else if (x < 380) {
    result = 3; // down
  } else if (x < 790) {
    result = 4; // left
  }
  return result;
}

// When called, this function will erase the current cursor and redraw it based on the cursorPosition and menuPage variables.
void drawCursor() {

  for (int x = 0; x < 2; x++) {     // Erases current cursor
    lcd.setCursor(0, x);
    lcd.print(" ");
  
  }

  // The menu is set up to be progressive (menuPage 0 = Item 1 & Item 2, menuPage 1 = Item 2 & Item 3, menuPage 2 = Item 3 & Item 4), so
  // in order to determine where the cursor should be you need to see if you are at an odd or even menu page and an odd or even cursor position.
  if (menuPage % 2 == 0) {
    if (cursorPosition % 2 == 0) {  // If the menu page is even and the cursor position is even that means the cursor should be on line 1
        lcd.setCursor(0, 0);
        lcd.write(byte(0));
      }

    if (cursorPosition % 2 != 0) {  // If the menu page is even and the cursor position is odd that means the cursor should be on line 2
      lcd.setCursor(0, 1);
      lcd.write(byte(0));
    }
  }

  if (menuPage % 2 != 0) {
    if (cursorPosition % 2 == 0) {  // If the menu page is odd and the cursor position is even that means the cursor should be on line 2
      lcd.setCursor(0, 1);
      lcd.write(byte(0));
    }

    if (cursorPosition % 2 != 0) {  // If the menu page is odd and the cursor position is odd that means the cursor should be on line 1
      lcd.setCursor(0, 0);
      lcd.write(byte(0));
    }
  }
}

// This function will generate the 2 menu items that can fit on the screen. They will change as you scroll through your menu. Up and down arrows will indicate your current menu position.
void mainMenuDraw() {
  // Serial.println("main menu");
  // Serial.print(menuPage);
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print(menuItems[menuPage]);
  lcd.setCursor(1, 1);
  lcd.print(menuItems[menuPage + 1]);
  if (menuPage == 0) {
    lcd.setCursor(15, 1);
    lcd.write(byte(2));
  } else if (menuPage > 0 and menuPage < maxMenuPages) {
    lcd.setCursor(15, 1);
    lcd.write(byte(2));
    lcd.setCursor(15, 0);
    lcd.write(byte(1));
  } else if (menuPage == maxMenuPages) {
    lcd.setCursor(15, 0);
    lcd.write(byte(1));
  }
}

// SUB Menu
void drawTriggerMenu(byte index){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.write(byte(3));
  lcd.print("Select Trigger:");
  lcd.setCursor(0, 1);
  lcd.print("[");
  String selected = subTriggerMenu[triggerMenu + index];
  lcd.print(selected);
  lcd.print("]");
  valueSelectedTrigger = EEPROM.read(addressTrigger);
  if(valueSelectedTrigger == index){
    lcd.write(byte(4)); // selected mark
  }
}

void drawPresetMenu(byte index){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write(byte(3));
  lcd.print("Select Preset:");
  lcd.setCursor(0, 1);
  lcd.print("[");
  String selected = subPresetMenu[presetMenu + index];
  lcd.print(selected);
  lcd.print("]");
  valueSelectedPreset = EEPROM.read(addressPreset);
  if(valueSelectedPreset == index){
    lcd.write(byte(4)); // selected mark
  }
}

void settingsMenuDraw(byte index){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write(byte(3));
  lcd.print("Settings:");
  lcd.setCursor(0, 1);
  String selected = subSettingsMenu[settingsMenu + index];
  lcd.print(selected);
}

void trigger(){
  currentFrame++;
  // nextFrame = true;
  shutterReleaseReady = true;
  #ifdef DEBUG_DONG
    Serial.println(F("[DEBUG] Next frame"));
  #endif
}

void menuCapture() { // Function executes when you select the 2nd item from main menu
  lcd.clear();
  int activeButton = 0;
  enteringSubMenu = true;
  while (activeButton == 0) {
    lcd.setCursor(0, 0);
    lcd.print("Intializing...");
    stepper.Enable();
    Serial.println(" ");
    #ifdef DEBUG_DONG
    Serial.println(F("[DEBUG] Intializing"));
    #endif
    lcd.setCursor(0,1);
    lcd.print("Preset: ");
    #ifdef DEBUG_DONG
    Serial.print(F("[DEBUG] Preset: "));
    #endif
    if(valueSelectedPreset == 0){
      lcd.print("LOW");
    #ifdef DEBUG_DONG
      Serial.println(F("LOW"));
    #endif
      loadDelay = valuePresetLowDelay;
      loadStep  = valuePresetLowRotation;
      loadFrame = valuePresetLowFrame;
    }else if(valueSelectedPreset == 1){
      lcd.print("MEDIUM");
      #ifdef DEBUG_DONG
      Serial.println(F("MEDIUM"));
      #endif
      loadDelay = valuePresetMedDelay;
      loadStep  = valuePresetMedRotation;
      loadFrame = valuePresetMedFrame;
    }else if(valueSelectedPreset == 2){
      lcd.print("HIGH");
      #ifdef DEBUG_DONG
      Serial.println(F("HIGH"));
      #endif
      loadDelay = valuePresetHighDelay;
      loadStep  = valuePresetHighRotation;
      loadFrame = valuePresetHighFrame;
    }
    Serial.println("");
    delay(1000);
    lcd.clear();

    id = Alarm.timerRepeat(loadDelay, trigger);
    isReadyCapture = true;
      //load by pin sensor
    if(valueSelectedTrigger == TRIGGER_AUDIO_JACK){
        loadTriggerInput = PIN_SHUTTER_JACK_AUDIO;
    }else if(valueSelectedTrigger == TRIGGER_BT_SERIAL){ // Bluetooth

    }else if(valueSelectedTrigger == TRIGGER_MANUAL_BUTTON){

    }
    while(isReadyCapture){
      // Serial.println(currentFrame);
      
      if(currentFrame < loadFrame){
        Alarm.delay(0);
        lcd.setCursor(0,0);
        lcd.print("Capturing...");
        lcd.setCursor(0, 1);
        lcd.print("Frame ");
        lcd.print(currentFrame);
        lcd.print(" of ");
        lcd.print(loadFrame);
        while(shutterReleaseReady){
          #ifdef DEBUG_DONG
          Serial.print(F("[DEBUG] Capturing "));
          Serial.print(currentFrame);
          Serial.print(F(" of "));
          Serial.println(loadFrame);
          Serial.println(F("[DEBUG] Shutter relase.. "));
          #endif
          if(valueSelectedTrigger == TRIGGER_MANUAL_BUTTON){
            int activeButton = 0;
            Serial.println("waiting trigger");
            int button;
            while (activeButton == 0){
              readKey = analogRead(0);
              if (readKey < 790) {
                delay(100);
                readKey = analogRead(0);
              }
              lcd.setCursor(0,0);
              lcd.print("[DEBUG] Press right btn");
              button = evaluateButton(readKey);
              switch (button) {
                case 1:
                  #ifdef DEBUG_DONG
                    Serial.println(F("[DEBUG] Pressed right button"));
                  #endif
                  shutterReleaseReady = false;
                  nextFrame = true;
                  activeButton = 1;
                break;
                case 4:  // This case will execute if the "back" button is pressed
                 #ifdef DEBUG_DONG
                    Serial.println(F("[DEBUG] Pressed Cancel button"));
                 #endif
                  shutterReleaseReady = false;
                  enteringSubMenu = false;
                  isReadyCapture = false;
                  button = 0;
                  activeButton = 1;
                break;
              }
            }
          
          }else if(valueSelectedTrigger == TRIGGER_BT_SERIAL){
            int activeButton = 0;
            int button;
            while (activeButton == 0){
              readKey = analogRead(0);
              if (readKey < 790) {
                delay(100);
                readKey = analogRead(0);
              }
              lcd.setCursor(0,0);
              lcd.print("[DEBUG] Bluetooth trigger");
              #ifdef DEBUG_DONG
                Serial.println(F("[DEBUG] Sent command"));
              #endif
              shutterReleaseReady = false;
              nextFrame = true;
              Serial.print(commandShutterBT);
              #ifdef DEBUG_DONG
                Serial.println(F(" "));
                Serial.println(F("[DEBUG] Done command"));
              #endif
              button = evaluateButton(readKey);
              activeButton = 1;
              switch (button) {
                case 4:  // This case will execute if the "back" button is pressed
                 #ifdef DEBUG_DONG
                    Serial.println(F("[DEBUG] Pressed Cancel button"));
                 #endif
                  shutterReleaseReady = false;
                  enteringSubMenu = false;
                  isReadyCapture = false;
                  button = 0;
                  activeButton = 1;
                break;
              }
            }
          }else{
            digitalWrite(loadTriggerInput, NYALA);
            delay(DELAY_BEFORE_SHUTTER_RELEASE); // shutter time needed
            digitalWrite(loadTriggerInput, MATI);
            delay(DELAY_AFTER_SHUTTER_RELEASE);
            shutterReleaseReady = false;
            nextFrame = true;
          }
        }

        while(nextFrame){
          nextFrame = false;
          #ifdef DEBUG_DONG
          Serial.print(F("[DEBUG] Stepper rotate "));
          Serial.print(loadStep);
          Serial.println(F(" degrees"));
          Serial.println("");
          #endif
          stepper.Move(loadStep);
        }
        
      } else {
        Serial.println("");
        #ifdef DEBUG_DONG
        Serial.print(F("[DEBUG] Job is done! "));
        #endif
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Job is done,");
        lcd.setCursor(0,1);
        lcd.print("Enjoy ");
        lcd.write(byte(5));
        stepper.Disable();
        delay(2000);
        Alarm.free(id); // free the memory
        id = dtINVALID_ALARM_ID;
        nextFrame = false;
        isReadyCapture = false;
        shutterReleaseReady = true;
        loadDelay = 0;
        loadStep = 0;
        loadFrame = 0;
        currentFrame = 0;
        activeButton = 1;
      }
      int button;
      readKey = analogRead(0);
      if (readKey < 790) {
          delay(100);
          readKey = analogRead(0);
      }
      button = evaluateButton(readKey);
      if(button == 4){
        #ifdef DEBUG_DONG
        Serial.println(F("[DEBUG] Canceled Job"));
        #endif
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Cancel Caputre.");
        Alarm.free(id); // free the memory
        id = dtINVALID_ALARM_ID;
        loadDelay = 0;
        loadStep = 0;
        loadFrame = 0;
        currentFrame = 0;
        stepper.Disable();
        delay(1000);
        isReadyCapture = false;
        nextFrame = false;
        activeButton = 1;
      }
    }
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
      enteringSubMenu = false;
      isReadyCapture = false;
      button = 0;
      activeButton = 1;
      break;
    }
  }
}

void subMenuPreset() { // PRESET
  byte x = 0;
  int activeButton = 0;
  enteringSubMenu = true;
  drawPresetMenu(x);
  
  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 0:
      break;
      case 1:
        button = 0;
        EEPROM.write(addressPreset, x);
        lcd.setCursor(0,1);
        lcd.print("-Saved         ");
        delay(1000);
        software_Reset() ;
      break;
      case 2:
        button = 0;
        x--;
        if(x <= presetMenu){
          x = presetMenu;
        }else if(x >= maxSubMenuPresets){
          x = maxSubMenuPresets;
        }
        drawPresetMenu(x);
      break;
      case 3:
        button = 0;
        x++;
        if(x <= presetMenu){
          x = presetMenu;
        }else if(x >= maxSubMenuPresets){
          x = maxSubMenuPresets;
        }
        drawPresetMenu(x);
      break;
      case 4:  // This case will execute if the "back" button is pressed
        enteringSubMenu = false;
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void subMenuInput() { // SET INPUT
  byte x = 0;
  int activeButton = 0;
  enteringSubMenu = true;
  drawTriggerMenu(0); // value yang nantinya disimpan di eeprom
  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 0:
      break;
      case 1: // save selected items
        button = 0;
        EEPROM.write(addressTrigger, x);
        lcd.setCursor(0,1);
        lcd.print("-Saved         ");
        delay(1000);
        software_Reset() ;
      break;
      case 2:
        button = 0;
        x--;
        if(x <= triggerMenu){
          x = triggerMenu;
        }else if(x >= maxSubMenuTrigger){
          x = maxSubMenuTrigger;
        }
      drawTriggerMenu(x);
      break;
      case 3:
        button = 0;
        x++;
        if(x <= triggerMenu){
          x = triggerMenu;
        }else if(x >= maxSubMenuTrigger){
          x = maxSubMenuTrigger;
        }
      drawTriggerMenu(x);
      break;
      case 4:  // This case will execute if the "back" button is pressed
        enteringSubMenu = false;
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

// SETING LOW
void childSettingLow(byte index,int value, String bracket1, String bracket2){
  //  Serial.println(index);
  lcd.clear();
  String selected = settingPresetMenuItems[settingPresetMenu + index];
  lcd.setCursor(0, 0);
  lcd.write(byte(3));
  lcd.print("Preset Low:");
  
  lcd.setCursor(0, 1);
  lcd.print(selected);

  if(value != NULL || value == 0){
    lcd.print(bracket1);
    lcd.print(value);
    if(index == 0){
      lcd.print(" sec");
    }else if(index == 1){
      lcd.write(byte(6));
    }
    lcd.print(bracket2);
  }
}


void subSettingLow(){
  
  int activeButton = 0;
  int activeButton2 = 0;
  byte x = 0;
  childSettingLow(x, valuePresetLowDelay, " ", "");
 
  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 0:
      break;
      case 1:
        button = 0;
        activeButton2 = 0;
       
        if(x == 0){
          childSettingLow(x, valuePresetLowDelay, "[", "]");
        }else if(x == 1){
          childSettingLow(x, valuePresetLowRotation, "[", "]");
        }else if(x == 2){
          childSettingLow(x, valuePresetLowFrame, "[", "]");
        }
        while (activeButton2 == 0){
          readKey = analogRead(0);
          if (readKey < 790) {
            delay(100);
            readKey = analogRead(0);
          }

          subCursorPosition = evaluateButton(readKey);
          switch (subCursorPosition) {
            case 0:
            break;
            case 1:
              button = 0;
              if(x == 0){
                EEPROM.write(addressPresetLowDelay, valuePresetLowDelay);
                delay(10);
                childSettingLow(x, valuePresetLowDelay, " ", "");
                lcd.setCursor(0,1);
                lcd.print("-Saved         ");
              }else if(x == 1){
                EEPROM.write(addressPresetLowRotation,  valuePresetLowRotation);
                delay(10);
                lcd.setCursor(0,1);
                lcd.print("-Saved         ");
              }else if(x == 2){ 
                EEPROM.write(addressPresetLowFrame, valuePresetLowFrame);
                delay(10);
                lcd.setCursor(0,1);
                lcd.print("-Saved         ");
              }

             delay(500);
            //  software_Reset();
             activeButton2 = 1;
            break;
            case 2:
              button = 0;
              if(x == 0){
                valuePresetLowDelay++;
              if(valuePresetLowDelay > MAX_DELAY){
                valuePresetLowDelay = MAX_DELAY;
              }
                childSettingLow(x, valuePresetLowDelay, "[", "]");
              }else if(x == 1){
                valuePresetLowRotation++;
              if(valuePresetLowRotation > MAX_ROTATION){
                valuePresetLowRotation = MAX_ROTATION;
              }
                childSettingLow(x, valuePresetLowRotation, "[", "]");
              }else if(x == 2){ 
                valuePresetLowFrame++;
              if(valuePresetLowFrame > MAX_FRAME){
                valuePresetLowFrame = MAX_FRAME;
              }
                childSettingLow(x, valuePresetLowFrame, "[", "]");
              }
            break;
            case 3:
              button = 0;
              if(x == 0){
                valuePresetLowDelay--;
                if(valuePresetLowDelay <= 0){
                    valuePresetLowDelay = 1;
                }
                childSettingLow(x, valuePresetLowDelay, "[", "]");
              }else if(x == 1){
                valuePresetLowRotation--;
                if(valuePresetLowRotation <= 0){
                  valuePresetLowRotation = 1;
                }

                childSettingLow(x, valuePresetLowRotation, "[", "]");
              }else if(x == 2){ 
                valuePresetLowFrame--;
                if(valuePresetLowFrame <= 0){
                  valuePresetLowFrame = 1;
                }
                childSettingLow(x, valuePresetLowFrame, "[", "]");
              }
            break;
            case 4:  // This case will execute if the "back" button is pressed
              readEEPROM();
              if(x == 0){
                childSettingLow(x, valuePresetLowDelay, " ", "");
              }else if(x == 1){
                childSettingLow(x, valuePresetLowRotation, " ", "");
              }else if(x == 2){ 
                childSettingLow(x, valuePresetLowFrame, " ", "");
              }
              button = 0;
              subCursorPosition = 0;
              activeButton2 = 1;
            break;
          }
        }
        
      break;
      case 2:
        x--;
        button = 0;
        if(x < settingPresetMenu){
          x = settingPresetMenu;
        }else if(x > maxsettingPresetMenuItems){
          x = maxsettingPresetMenuItems;
        }
        if(x == 0){
          childSettingLow(x, valuePresetLowDelay, " ", "");
        }else if(x == 1){
          childSettingLow(x, valuePresetLowRotation, " ", "");
        }else if(x == 2){ 
          childSettingLow(x, valuePresetLowFrame, " ", "");
        }
        
      break;
      case 3:
        x++;
        button = 0;
        if(x < settingPresetMenu){
          x = maxsettingPresetMenuItems;
        }else if(x > maxsettingPresetMenuItems){
          x = settingPresetMenu;
        }
        if(x == 0){
          childSettingLow(x, valuePresetLowDelay, " ", "");
        }else if(x == 1){
          childSettingLow(x, valuePresetLowRotation, " ", "");
        }else if(x == 2){ 
          childSettingLow(x, valuePresetLowFrame, " ", "");
        }
       
      break;
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
      break;
    }
  }
}
// END SETTING LOW

// SETTING MEDIUM
void childSettingMed(byte index,int value, String bracket1, String bracket2){

  lcd.clear();
  String selected = settingPresetMenuItems[settingPresetMenu + index];
  lcd.setCursor(0, 0);
  lcd.write(byte(3));
  lcd.print("Preset Medium:");
  
  lcd.setCursor(0, 1);
  lcd.print(selected);
  if(value != NULL || value == 0){
    lcd.print(bracket1);
    lcd.print(value);
      if(index == 0){
      lcd.print(" sec");
    }else if(index == 1){
      lcd.write(byte(6));
    }
    lcd.print(bracket2);
  }
}

void subSettingMed(){
  int activeButton = 0;
  int activeButton2 = 0;
  byte x = 0;
  childSettingMed(x, valuePresetMedDelay, " ", "");
 
  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 0:
      break;
      case 1:
        button = 0;
        activeButton2 = 0;
       
        if(x == 0){
          childSettingMed(x, valuePresetMedDelay, "[", "]");
        }else if(x == 1){
          childSettingMed(x, valuePresetMedRotation, "[", "]");
        }else if(x == 2){
          childSettingMed(x, valuePresetMedFrame, "[", "]");
        }
        while (activeButton2 == 0){
          readKey = analogRead(0);
          if (readKey < 790) {
            delay(100);
            readKey = analogRead(0);
          }

          subCursorPosition = evaluateButton(readKey);
          switch (subCursorPosition) {
            case 0:
            break;
            case 1:
              button = 0;
              if(x == 0){
                EEPROM.write(addressPresetMedDelay, valuePresetMedDelay);
                delay(10);
                childSettingMed(x, valuePresetLowDelay, " ", "");
                lcd.setCursor(0,1);
                lcd.print("-Saved         ");
              }else if(x == 1){
                EEPROM.write(adaddressPresetMedRotation,  valuePresetMedRotation);
                delay(10);
                childSettingMed(x, valuePresetMedRotation, " ", "");
                lcd.setCursor(0,1);
                lcd.print("-Saved         ");
              }else if(x == 2){ 
                EEPROM.write(addressPresetMedFrame, valuePresetMedFrame);
                delay(10);
                childSettingMed(x, valuePresetMedFrame, " ", "");
                lcd.setCursor(0,1);
                lcd.print("-Saved         ");
              }

             delay(500);
            //  software_Reset();
             activeButton2 = 1;
            break;
            case 2:
              button = 0;
              if(x == 0){
                valuePresetMedDelay++;
              if(valuePresetMedDelay > MAX_DELAY){
                valuePresetMedDelay = MAX_DELAY;
              }
                childSettingMed(x, valuePresetMedDelay, "[", "]");
              }else if(x == 1){
                valuePresetMedRotation++;
              if(valuePresetMedRotation > MAX_ROTATION){
                valuePresetMedRotation = MAX_ROTATION;
              }
                childSettingMed(x, valuePresetMedRotation, "[", "]");
              }else if(x == 2){ 
                valuePresetMedFrame++;
              if(valuePresetMedFrame > MAX_FRAME){
                valuePresetMedFrame = MAX_FRAME;
              }
                childSettingMed(x, valuePresetMedFrame, "[", "]");
              }
            break;
            case 3:
              button = 0;
              if(x == 0){
                valuePresetMedDelay--;
                if(valuePresetMedDelay <= 0){
                  valuePresetMedDelay = 1;
                }
                childSettingMed(x, valuePresetMedDelay, "[", "]");
              }else if(x == 1){
               valuePresetMedRotation--;
                if(valuePresetMedRotation <= 0){
                  valuePresetMedRotation = 1;
                }
                childSettingMed(x, valuePresetMedRotation, "[", "]");
              }else if(x == 2){ 
                valuePresetMedFrame--;
                if(valuePresetMedFrame <= 0){
                  valuePresetMedFrame = 1;
                }
                childSettingMed(x, valuePresetMedFrame, "[", "]");
              }
            break;
            case 4: 
              readEEPROM();
              if(x == 0){
                childSettingMed(x, valuePresetMedDelay, " ", "");
              }else if(x == 1){
                childSettingMed(x, valuePresetMedRotation, " ", "");
              }else if(x == 2){ 
                childSettingMed(x, valuePresetMedFrame, " ", "");
              }
              button = 0;
              subCursorPosition = 0;
              activeButton2 = 1;
            break;
          }
        }
        
      break;
      case 2:
        x--;
        button = 0;
        if(x < settingPresetMenu){
          x = settingPresetMenu;
        }else if(x > maxsettingPresetMenuItems){
          x = maxsettingPresetMenuItems;
        }
        if(x == 0){
          childSettingMed(x, valuePresetMedDelay, " ", "");
        }else if(x == 1){
          childSettingMed(x, valuePresetMedRotation, " ", "");
        }else if(x == 2){ 
          childSettingMed(x, valuePresetMedFrame, " ", "");
        }
        
      break;
      case 3:
        x++;
        button = 0;
        if(x < settingPresetMenu){
          x = maxsettingPresetMenuItems;
        }else if(x > maxsettingPresetMenuItems){
          x =  settingPresetMenu;
        }
        if(x == 0){
          childSettingMed(x, valuePresetMedDelay, " ", "");
        }else if(x == 1){
          childSettingMed(x, valuePresetMedRotation, " ", "");
        }else if(x == 2){ 
          childSettingMed(x, valuePresetMedFrame, " ", "");
        }
       
      break;
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
      break;
    }
  }
}
// END SETTING MEDIUM

// SETTING HIGH
void childSettingHigh(byte index,int value, String bracket1, String bracket2){
  lcd.clear();
  String selected = settingPresetMenuItems[settingPresetMenu + index];
  lcd.setCursor(0, 0);
  lcd.write(byte(3));
  lcd.print("Preset High:");
  
  lcd.setCursor(0, 1);
  lcd.print(selected);
  if(value != NULL || value == 0){
    lcd.print(bracket1);
    lcd.print(value);
    if(index == 0){
      lcd.print(" sec");
    }else if(index == 1){
      lcd.write(byte(6));
    }
    lcd.print(bracket2);
  }
}

void subSettingHigh(){
  int activeButton = 0;
  int activeButton2 = 0;
  byte x = 0;
  childSettingHigh(x, valuePresetHighDelay, " ", "");
 
  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 0:
      break;
      case 1:
        button = 0;
        activeButton2 = 0;
       
        if(x == 0){
          childSettingHigh(x, valuePresetHighDelay, "[", "]");
        }else if(x == 1){
          childSettingHigh(x, valuePresetHighRotation, "[", "]");
        }else if(x == 2){
          childSettingHigh(x, valuePresetHighFrame, "[", "]");
        }
        while (activeButton2 == 0){
          readKey = analogRead(0);
          if (readKey < 790) {
            delay(100);
            readKey = analogRead(0);
          }

          subCursorPosition = evaluateButton(readKey);
          switch (subCursorPosition) {
            case 0:
            break;
            case 1:
              button = 0;
              if(x == 0){
                EEPROM.write(addressPresetHighDelay, valuePresetHighDelay);
                delay(10);
                childSettingHigh(x, valuePresetHighDelay, " ", "");
                lcd.setCursor(0,1);
                lcd.print("-Saved         ");
              }else if(x == 1){
                EEPROM.write(addressPresetHighRotation,  valuePresetHighRotation);
                delay(10);
                childSettingHigh(x, valuePresetHighRotation, " ", "");
                lcd.setCursor(0,1);
                lcd.print("-Saved         ");
              }else if(x == 2){ 
                EEPROM.write(addressPresetHighFrame, valuePresetHighFrame);
                delay(10);
                childSettingHigh(x, valuePresetHighFrame, " ", "");
                lcd.setCursor(0,1);
                lcd.print("-Saved         ");
              }

             delay(500);
            //  software_Reset();
             activeButton2 = 1;
            break;
            case 2:
              button = 0;
              if(x == 0){
                valuePresetHighDelay++;
              if(valuePresetHighDelay > MAX_DELAY){
                valuePresetHighDelay = MAX_DELAY;
              }
                childSettingHigh(x, valuePresetHighDelay, "[", "]");
              }else if(x == 1){
                valuePresetHighRotation++;
              if(valuePresetHighRotation > MAX_ROTATION){
                valuePresetHighRotation = MAX_ROTATION;
              }
                childSettingHigh(x, valuePresetHighRotation, "[", "]");
              }else if(x == 2){ 
                valuePresetHighFrame++;
              if(valuePresetHighFrame > MAX_FRAME){
                valuePresetHighFrame = MAX_FRAME;
              }
                childSettingHigh(x, valuePresetHighFrame, "[", "]");
              }
            break;
            case 3:
              button = 0;
              if(x == 0){
                valuePresetHighDelay--;
                if(valuePresetHighDelay <= 0){
                  valuePresetHighDelay = 1;
                }
                childSettingHigh(x, valuePresetHighDelay, "[", "]");
              }else if(x == 1){
               valuePresetHighRotation--;
                if(valuePresetHighRotation <= 0){
                  valuePresetHighRotation = 1;
                }
                childSettingHigh(x, valuePresetHighRotation, "[", "]");
              }else if(x == 2){ 
                valuePresetHighFrame--;
                if(valuePresetHighFrame <= 0){
                  valuePresetHighFrame = 1;
                }
                childSettingHigh(x, valuePresetHighFrame, "[", "]");
              }
            break;
            case 4: 
              readEEPROM();
              if(x == 0){
                childSettingHigh(x, valuePresetHighDelay, " ", "");
              }else if(x == 1){
                childSettingHigh(x, valuePresetHighRotation, " ", "");
              }else if(x == 2){ 
                childSettingHigh(x, valuePresetHighFrame, " ", "");
              }
              button = 0;
              subCursorPosition = 0;
              activeButton2 = 1;
            break;
          }
        }
        
      break;
      case 2:
        x--;
        button = 0;
        if(x < settingPresetMenu){
          x = settingPresetMenu;
        }else if(x > maxsettingPresetMenuItems){
          x = maxsettingPresetMenuItems;
        }
        if(x == 0){
          childSettingHigh(x, valuePresetHighDelay, " ", "");
        }else if(x == 1){
          childSettingHigh(x, valuePresetHighRotation, " ", "");
        }else if(x == 2){ 
          childSettingHigh(x, valuePresetHighFrame, " ", "");
        }
        
      break;
      case 3:
        x++;
        button = 0;
        if(x < settingPresetMenu){
          x = maxsettingPresetMenuItems;
        }else if(x > maxsettingPresetMenuItems){
          x = settingPresetMenu;
        }
        if(x == 0){
          childSettingHigh(x, valuePresetHighDelay, " ", "");
        }else if(x == 1){
          childSettingHigh(x, valuePresetHighRotation, " ", "");
        }else if(x == 2){ 
          childSettingHigh(x, valuePresetHighFrame, " ", "");
        }
       
      break;
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
      break;
    }
  }
}
// END SETTING HIGH

void subMenuSettings() { //SETTINGS
  int activeButton = 0;
  byte x = 0;
  settingsMenuDraw(x);

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 0:
      break;
      case 1:
          button = 0;
          subCursorPosition = evaluateButton(readKey);
          switch (subCursorPosition){
          case 0:

          break;
          case 1: // enter right
              subCursorPosition = 0;
              switch (x){
                case 0:
                  subSettingLow();
                break;
                
                case 1:
                  subSettingMed();
                break;

                case 2:
                  subSettingHigh();
                break;
              }
          break;
          case 4:  // This case will execute if the "back" button is pressed
          subCursorPosition = 0;
          activeButton = 1;
          break;
        }
   
        settingsMenuDraw(x);
      break;
      case 2:
        button = 0;
        x--;
        if(x < settingsMenu){
          x = settingsMenu;
        }else if(x > maxSubMenuSettings){
          x = maxSubMenuSettings;
        }
        settingsMenuDraw(x);
      break;
      case 3:
        button = 0;
        x++;
        if(x < settingsMenu){
          x = maxSubMenuSettings;
        }else if(x > maxSubMenuSettings){
          x = settingsMenu;
        }
        settingsMenuDraw(x);
      break;
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
      break;
    }
  }
}

void aboutMenu(){
  int activeButton = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Version ");
  lcd.print(VERSION);

  lcd.setCursor(0, 1);
  lcd.print("by Arif ");
  lcd.write(byte(5));
  lcd.print(" 2020");

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void menuShowCase(){

}

void operateMainMenu() {
  int activeButton = 0;
  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 0: // When button returns as 0 there is no action taken
        break;
      case 1:  // This case will execute if the "forward" button is pressed
        button = 0;
        switch (cursorPosition) { // The case that is selected here is dependent on which menu page you are on and where the cursor is.
          case 0:
            menuCapture();
            break;
          case 1:
            // showcase menu
            menuShowCase();
          break;
          case 2:
            subMenuPreset();
            break;
          case 3:
            subMenuInput();
            break;
          case 4:
            subMenuSettings();
            break;
          case 5:
            aboutMenu();
            break;
        }
        activeButton = 1;
        mainMenuDraw();
        drawCursor();
        break;
      case 2:
        button = 0;
        if (menuPage == 0) {
          cursorPosition = cursorPosition - 1;
          cursorPosition = constrain(cursorPosition, 0, ((sizeof(menuItems) / sizeof(String)) - 1));
        }
        if (menuPage % 2 == 0 and cursorPosition % 2 == 0) {
          menuPage = menuPage - 1;
          menuPage = constrain(menuPage, 0, maxMenuPages);
        }

        if (menuPage % 2 != 0 and cursorPosition % 2 != 0) {
          menuPage = menuPage - 1;
          menuPage = constrain(menuPage, 0, maxMenuPages);
        }

        cursorPosition = cursorPosition - 1;
        cursorPosition = constrain(cursorPosition, 0, ((sizeof(menuItems) / sizeof(String)) - 1));

        mainMenuDraw();
        drawCursor();
        activeButton = 1;
        break;
      case 3:
        button = 0;
        if (menuPage % 2 == 0 and cursorPosition % 2 != 0) {
          menuPage = menuPage + 1;
          menuPage = constrain(menuPage, 0, maxMenuPages);
        }

        if (menuPage % 2 != 0 and cursorPosition % 2 == 0) {
          menuPage = menuPage + 1;
          menuPage = constrain(menuPage, 0, maxMenuPages);
        }

        cursorPosition = cursorPosition + 1;
        cursorPosition = constrain(cursorPosition, 0, ((sizeof(menuItems) / sizeof(String)) - 1));
        mainMenuDraw();
        drawCursor();
        activeButton = 1;
        break;
    }
  }
}

  // routine 
void loop() {

  mainMenuDraw();
  drawCursor();
  operateMainMenu();
 
}


