#include <Arduino.h>
#include "SerialCommands.h"

char serial_command_buffer_[32];

int modem_cnt = 8;
byte pwr_pins[] = {2, 3, 4, 5, 6, 7, 8, 9};
byte sim_pins[] = {A3, A2, A1, A0, 13, 12, 11, 10};

SerialCommands serial_commands_(&Serial, serial_command_buffer_, sizeof(serial_command_buffer_), "\r", " ");

void cmd_unrecognized(SerialCommands* sender, const char* cmd)
{
  sender->GetSerial()->print("Unrecognized command [");
  sender->GetSerial()->print(cmd);
  sender->GetSerial()->println("]");
}

void cmd_pwr(SerialCommands* sender)
{
  char* mdm_str = sender->Next();
  if (mdm_str == NULL)
  {
    sender->GetSerial()->println("ERROR NO_MODEM");
    return;
  }

  char* val_str = sender->Next();
  if (val_str == NULL)
  {
    sender->GetSerial()->println("ERROR NO_VALUE");
    return;
  }

  int mdm = atoi(mdm_str);
  if(mdm<1 || mdm>modem_cnt) {
    sender->GetSerial()->println("ERROR WRONG_MODEM(MUST BE 1-8)");
    return;    
  }
  
  int val = atoi(val_str);
  if(val<0 || val>1) {
    sender->GetSerial()->println("ERROR WRONG_VALUE(MUST BE 0-1)");
    return;    
  }

  switch(val) {
    case 0:
        digitalWrite(pwr_pins[mdm-1], HIGH);
        break;
    case 1:
        digitalWrite(pwr_pins[mdm-1], LOW);
        break;      
  }

  sender->GetSerial()->print("PWR_SET [");
  sender->GetSerial()->print(mdm_str);
  sender->GetSerial()->print("] = ");
  sender->GetSerial()->print(val_str);
  sender->GetSerial()->println();
}

void cmd_sim(SerialCommands* sender)
{
  char* mdm_str = sender->Next();
  if (mdm_str == NULL)
  {
    sender->GetSerial()->println("ERROR NO_MODEM");
    return;
  }

  char* val_str = sender->Next();
  if (val_str == NULL)
  {
    sender->GetSerial()->println("ERROR NO_VALUE");
    return;
  }

  int mdm = atoi(mdm_str);
  if(mdm<1 || mdm>modem_cnt) {
    sender->GetSerial()->println("ERROR WRONG_MODEM(MUST BE 1-8)");
    return;    
  }
  
  int val = atoi(val_str);
  if(val<1 || val>2) {
    sender->GetSerial()->println("ERROR WRONG_VALUE(MUST BE 1-2)");
    return;    
  }

  switch(val) {
    case 1:
        digitalWrite(sim_pins[mdm-1], LOW);
        break;
    case 2:
        digitalWrite(sim_pins[mdm-1], HIGH);
        break;      
  }

  sender->GetSerial()->print("SIM_SET [");
  sender->GetSerial()->print(mdm_str);
  sender->GetSerial()->print("] = ");
  sender->GetSerial()->print(val_str);
  sender->GetSerial()->println();
}


SerialCommand cmd_pwr_("PWR", cmd_pwr);
SerialCommand cmd_sim_("SIM", cmd_sim);

void setup() 
{
  for(int i=0; i<modem_cnt; i++) {
    // Disable modem's power by default
    pinMode(pwr_pins[i], OUTPUT);
    digitalWrite(pwr_pins[i], HIGH);
    // Set SIM 0 by default
    pinMode(sim_pins[i], OUTPUT);
    digitalWrite(sim_pins[i], LOW);    
  }
  
  Serial.begin(9600);

  serial_commands_.SetDefaultHandler(cmd_unrecognized);
  serial_commands_.AddCommand(&cmd_pwr_);
  serial_commands_.AddCommand(&cmd_sim_);

  Serial.println("Default values loaded");
}

void loop() 
{
  serial_commands_.ReadSerial();
}
