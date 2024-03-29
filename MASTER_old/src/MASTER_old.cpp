#include <Wire.h>
#include <SoftwareSerial.h>
#include <stdio.h>
#include <stdlib.h>
#include "Arduino.h"

#define START_FRAME 0xABCD  // [-] Start frme definition for reliable serial communication
//#define ACTIVATEDELAY 500
#define MAXSPEED 1000
#define SPEEDSTEP 2
#define STEERSTEP 2

#define Contr2 11 //High level for controller 2 power on 
#define Contr1 10 //High level for controller 1 power on 
#define relay1 9 //number of which  attached to relay 
#define relay2 8 //number of which  attached to relay 
#define auxbutton 7 //button for reversing on corpus of master 
#define statusled 6 //Yellow Led on corpus of master 


unsigned long previousMillisbls = 0;  // blinkLED
int ledStatebls = LOW; //blinkLED

void blinkled(uint8_t ledname ,unsigned long interval);//1=always on , 0=shut down
void SetMotors(uint8_t IO);
void RecieveData();
void DataConstruct();
void SmoothSpeed();
void Send(int fSp, int sSp, int fSt, int sSt);
void Send();
void Debug();
void RelayCheck();

SoftwareSerial Controller1(2, 3);  //RX TX
SoftwareSerial Controller2(4, 5);  //RX TX 

typedef struct {
  uint16_t start;
  int16_t mysteer;
  int16_t myspeed;
  uint16_t checksum;
} SerialCommand;
SerialCommand Command;

typedef struct {
  uint16_t start;
  int16_t cmd1;
  int16_t cmd2;
  int16_t speedR_meas;
  int16_t speedL_meas;
  int16_t batVoltage;
  int16_t boardTemp;
  uint16_t cmdLed;
  uint16_t checksum;
} SerialFeedback;
SerialFeedback Feedback;
SerialFeedback NewFeedback;
unsigned long iTimeSend = 0;

uint8_t data[5];
unsigned long activateDelay;
int enabled;

int speed, steer, currSpeedS, currSpeedF, currSteer;
int defSpeed;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Controller1.begin(115200);
  Controller2.begin(115200);

  pinMode(auxbutton,INPUT_PULLUP);
  pinMode(statusled, OUTPUT);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(Contr1, OUTPUT);
  pinMode(Contr2, OUTPUT);
  digitalWrite(Contr1, 0);
  digitalWrite(Contr2, 0);
  digitalWrite(relay1, 0);
  digitalWrite(relay2, 0);
  blinkled(statusled,100);
  speed = 0;
  steer = 0;
  currSpeedS = 0;
  currSpeedF = 0;
  currSteer = 0;
  data[0] = 127;
  data[1] = 128;
  data[2] = 111;
  data[3] = 111;
  data[4] = 5;
  SetMotors(1);
  delay(500);
  Serial.print("AUXBUTTON : "); Serial.println(digitalRead(auxbutton)); Serial.println();

}

void loop() {
if (data[4]==0)
{blinkled(statusled,1);}
else 
{blinkled(statusled,100);}
  RecieveData();
  DataConstruct();
  SmoothSpeed();
    if (digitalRead(auxbutton)==0)
     {
      Send(currSpeedF, currSpeedS, currSteer, currSteer);
     }
     else 
     {Send(-currSpeedF,-currSpeedS, currSteer, currSteer);}

  RelayCheck();
  if (Serial.available())
  {
  Debug();
  }
}

void RecieveData() {
  Wire.requestFrom(10, 5);                 // Запрос на получение данных от Slave Arduino(9 number of slave , 8 number of bytes)
  if (Wire.available() >= 5) {  // Проверка наличия достаточного количества данных
    for (int i = 0; i <5; i++) {
      data[i] = Wire.read();  // Чтение принятых данных и сохранение их в массив
    }
  }
 
}

void SetMotors(uint8_t IO) {
  if (IO==1)
  {
  digitalWrite(Contr1, 1);
  digitalWrite(Contr2, 1);
  }
  else if (IO==0)
  {
  digitalWrite(Contr1, 0);
  digitalWrite(Contr2, 0);
  }
}

void RelayCheck() {       ///число == номер реле которое должно спаотать и скинуть одну или вторую мину 
  if (data[2] == 112) 
  {digitalWrite(relay1, 1);}
  else if (data[2] == 113) 
  {digitalWrite(relay2, 1);}
   else 
   {
    digitalWrite(relay1, 0);
    digitalWrite(relay2, 0);
   }
}

void DataConstruct() {
  defSpeed = MAXSPEED / 2;

  if(data[3] == 112 || data[3] == 114)
  {
    defSpeed = MAXSPEED;
  }

  if (data[0] == 127)
  {
    speed = 0;
  } 
  else
  {
    speed = map(data[0], 0, 255, -1 * defSpeed, defSpeed);
  }

  if (data[1] == 128)
  {
    steer = 0;
  }
  else
  {
    int defSteer = map(abs(currSpeedS), 0 ,(defSpeed / 100 * 80), defSpeed, defSpeed / 2);
    steer = map(data[1], 0, 255, -1 * defSteer, defSteer);
    if(currSpeedS > 0)
      steer = -1 * steer;
  }
}

void SmoothSpeed()
{
  if(data[3] == 113 || data[4] == 114)
  {
    currSpeedS = speed;
    currSpeedF = speed;
    currSteer = steer;
    return;
  }
//speed
  if(speed == 0)
  {
    currSpeedS = speed;
    currSpeedF = speed;
  }
  else if((currSpeedS > 0 && speed < 0) || (currSpeedS < 0 && speed > 0))
  {
    currSpeedS = 0;
    currSpeedF = 0;
  }
  else if (currSpeedS > 0 && currSpeedS < 75 && speed > 0 && speed > 75)
  {
    currSpeedS = 75;
    currSpeedF = 75;
  }
  else if (currSpeedS < 0 && currSpeedS > -75 && speed < 0 && speed > -75)
  {
    currSpeedS = -75;
    currSpeedF = -75;
  }
  else if(currSpeedS > speed)
  {
    currSpeedS -= SPEEDSTEP;
    currSpeedF -= (SPEEDSTEP + 1);
    if(currSpeedS > currSpeedF + 40)
      currSpeedS = currSpeedF;
    if(currSpeedS < speed)
    {
      currSpeedS = speed;
      currSpeedF = speed;

    }
  }
  else if(currSpeedS < speed)
  {
    currSpeedS += (SPEEDSTEP + 1);
    currSpeedF += SPEEDSTEP;
    if(currSpeedS > currSpeedF + 40)
      currSpeedF = currSpeedF;
    if(currSpeedS > speed)
    {
      currSpeedS = speed;
      currSpeedF = speed;
    }
  }
  //steer
  if(steer == 0)
  {
    currSteer = steer;
  }
  else if ((currSteer > 0 && steer < 0) || (currSteer < 0 && steer > 0))
  {
    currSteer = 0;
  }
  else if (currSteer > steer)
  {
    currSteer -= STEERSTEP;
    if(currSteer < steer)
      currSteer = steer;
  }
  else if (currSteer < steer)
  {
    currSteer += STEERSTEP;
    if(currSteer > steer)
      currSteer = steer;
  }
  
  
  
}

void Send(int fSp, int sSp, int fSt, int sSt) {
  Command.start = (uint16_t)START_FRAME;
  Command.mysteer = sSt;
  Command.myspeed = sSp;
  Command.checksum = (uint16_t)(Command.start ^ Command.mysteer ^ Command.myspeed);
  Controller1.write((uint8_t *)&Command, sizeof(Command));
  Command.mysteer = fSt;
  Command.myspeed = fSp;
  Command.checksum = (uint16_t)(Command.start ^ Command.mysteer ^ Command.myspeed);
  Controller2.write((uint8_t *)&Command, sizeof(Command));
}



void Debug() {
  Serial.print("data is: ");
  for (int i = 0; i <=4; i++) {
    Serial.print(data[i]);
    Serial.print(" ");
  }
  Serial.println();
}

  

  void blinkled(uint8_t ledname ,unsigned long interval)
 {
  if (interval==1)
  {digitalWrite(ledname,1);}
  else if  (interval==0)
  {digitalWrite(ledname,0);}
  else  
  {
  unsigned long currentMillis = millis();  // Получаем текущее время
  if (currentMillis - previousMillisbls >= interval) {
    previousMillisbls = currentMillis;
    if (ledStatebls == LOW) {
      ledStatebls = HIGH;
    } else {
      ledStatebls = LOW;
    }
    digitalWrite(ledname, ledStatebls);
  }
}
}