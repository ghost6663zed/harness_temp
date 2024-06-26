#include <Arduino.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "SerialCommands.h"

#define LCD_ROW_COUNT 2
#define LCD_COL_COUNT 16
#define PORT_SPEED 115200
#define PWR_PIN 2
#define SIM_PIN 3
#define LED_PIN 13

char serial_command_buffer_[256];
char* delim = " ";


LiquidCrystal_I2C *lcd;

SerialCommands serial_commands_(&Serial, serial_command_buffer_, sizeof(serial_command_buffer_), "\r", delim);

void cmd_unrecognized(SerialCommands* sender, const char* cmd)
{
  sender->GetSerial()->print("Unrecognized command [");
  sender->GetSerial()->print(cmd);
  sender->GetSerial()->println("]");
}

void cmd_pwr(SerialCommands* sender)
{
  char* val_str = sender->Next();
  if (val_str == NULL)
  {
    sender->GetSerial()->println("ERROR NO_VALUE");
    return;
  }

  int val = atoi(val_str);
  if(val<0 || val>1) {
    sender->GetSerial()->println("ERROR WRONG_VALUE(MUST BE 0-1)");
    return;    
  }

  switch(val) {
    case 0:
        digitalWrite(PWR_PIN, LOW);
        break;
    case 1:
        digitalWrite(PWR_PIN, HIGH);
        break;      
  }

  sender->GetSerial()->print("PWR");
  sender->GetSerial()->print(" = ");
  sender->GetSerial()->print(val_str);
  sender->GetSerial()->println();
}

void cmd_echo(SerialCommands* sender)
{
  int rownum = 0;
    
  char* rownum_str = sender->Next();
  if (rownum_str == NULL)
  {
    sender->GetSerial()->println("ERROR NO_ROWNUM");
    return;
  }

  rownum = atoi(rownum_str);
  if(rownum<=0 || rownum>LCD_ROW_COUNT) {
    sender->GetSerial()->print("ERROR WRONG_ROWNUM (Must be 1 - "); 
    sender->GetSerial()->print(LCD_ROW_COUNT);
    sender->GetSerial()->println(")");
    return;    
  }
  
  if (lcd == NULL) {
    sender->GetSerial()->println("ERROR NO_LCD (LCD is not found on I2C bus)");
    return;        
  }
  
  // Set cursor to default position
  lcd->setCursor(0, rownum-1);
  lcd->noBlink();

  String StrBuf = "";
  while (char* val_str = sender->Next()) {
    if (StrBuf.length() > 0) {
      StrBuf += " ";
    }
    StrBuf += String(val_str);           
  }

  if (StrBuf.length() == 0) {
    sender->GetSerial()->println("ERROR MSG_TOO_SHORT");
    return;        
  }

  if (StrBuf.length() > LCD_COL_COUNT) {
    sender->GetSerial()->print("ERROR MSG_TOO_LONG (MAX length with delimiters is ");
    sender->GetSerial()->print(LCD_COL_COUNT);
    sender->GetSerial()->println(")");
    return;        
  }
  sender->GetSerial()->print("ECHO \"");
  sender->GetSerial()->print(StrBuf);
  sender->GetSerial()->print("\" ");
  sender->GetSerial()->print(StrBuf.length());
  sender->GetSerial()->print(" symbols at row ");
  sender->GetSerial()->print(rownum);
  sender->GetSerial()->println(".");
  // Fill output string
  while(StrBuf.length() < LCD_COL_COUNT) {
    StrBuf += " ";
  }
  lcd->print(StrBuf);
}

void cmd_clear(SerialCommands* sender)
{
  if (lcd == NULL) {
    sender->GetSerial()->println("ERROR NO_LCD (LCD is not found on I2C bus)");
    return;        
  }

  char* rownum_str = sender->Next();
  if (rownum_str == NULL)
  {
    sender->GetSerial()->println("ERROR NO_ROWNUM");
    return;
  }

  int rownum = atoi(rownum_str);
  if(rownum>LCD_ROW_COUNT) {
    sender->GetSerial()->print("ERROR WRONG_ROWNUM (Must be 1 - "); 
    sender->GetSerial()->print(LCD_ROW_COUNT);
    sender->GetSerial()->println(")");
    return;    
  }

  String StrBuf = "";
  while(StrBuf.length() < LCD_COL_COUNT) {
    StrBuf += " ";
  }
  if(rownum == 0){
    lcd->clear();
    lcd->setCursor(0,0);
    lcd->noCursor();
    lcd->noBlink();    
    sender->GetSerial()->println("CLEAR ALL LCD"); 
  } else {
    lcd->setCursor(0,rownum - 1);
    lcd->print(StrBuf);
    sender->GetSerial()->print("CLEAR LCD ROW ");
    sender->GetSerial()->print(rownum);
  }
}

void cmd_sim(SerialCommands* sender)
{
  char* val_str = sender->Next();
  if (val_str == NULL)
  {
    sender->GetSerial()->println("ERROR NO_VALUE");
    return;
  }

  int val = atoi(val_str);
  if(val<0 || val>1) {
    sender->GetSerial()->println("ERROR WRONG_VALUE(MUST BE 0-1)");
    return;    
  }

  switch(val) {
    case 0:
        digitalWrite(SIM_PIN, LOW);
        break;
    case 1:
        digitalWrite(SIM_PIN, HIGH);
        break;      
  }

  sender->GetSerial()->print("SIM");
  sender->GetSerial()->print(" = ");
  sender->GetSerial()->print(val_str);
  sender->GetSerial()->println();
}

// Detect device on I2C bus and return address
byte i2detect()
{
  byte err, addr;
  for(addr = 0; addr < 127; addr++ )
  {
    digitalWrite(LED_PIN, HIGH);
    Wire.beginTransmission(addr);
    err = Wire.endTransmission();
    if (err == 0)
    {
      digitalWrite(LED_PIN, HIGH);
      return addr;
    }
    digitalWrite(LED_PIN, LOW);
  }
}

SerialCommand cmd_pwr_("PWR", cmd_pwr);
SerialCommand cmd_echo_("ECHO", cmd_echo);
SerialCommand cmd_clear_("CLEAR", cmd_clear);
SerialCommand cmd_sim_("SIM", cmd_sim);

void setup() 
{
  // Disable modem's board power by default
  pinMode(PWR_PIN, OUTPUT);
  digitalWrite(PWR_PIN, LOW);

  // Disable SIM data by default
  pinMode(SIM_PIN, OUTPUT);
  digitalWrite(SIM_PIN, LOW);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Serial init
  Serial.begin(PORT_SPEED);

  serial_commands_.SetDefaultHandler(cmd_unrecognized);
  serial_commands_.AddCommand(&cmd_pwr_);
  serial_commands_.AddCommand(&cmd_echo_);
  serial_commands_.AddCommand(&cmd_clear_);
  serial_commands_.AddCommand(&cmd_sim_);  
  
  // I2C LCD init
  Wire.begin();
  byte i2c_addr = i2detect();
  if (i2c_addr != 0) 
  {
    lcd = new LiquidCrystal_I2C(i2c_addr, 16, 2);  
    lcd->init();
    lcd->backlight();
    lcd->noCursor();
    lcd->clear();
    lcd->setCursor(0,0);
    lcd->print("Loading ...");
    lcd->setCursor(0,0);
    lcd->blink();
    Serial.print("I2C LCD display is found at ");
    Serial.println(i2c_addr);
  }
  else {
    Serial.println("I2C LCD display is not found");
  }
}

void loop() 
{
  serial_commands_.ReadSerial();
}
