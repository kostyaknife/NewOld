#include <Wire.h>
#include <SoftwareSerial.h>
#include <stdio.h>
#include <stdlib.h>
#include "Arduino.h"

#define ADDR 11 // 
#define UARTSPEED 115200 //9600 19200 38400 57600 115200 //
#define AIRSPEED 19200 // 300 1200 2400 4800 9600 19200 //
#define CHANNEL 920 // 862...931 //

#define ledstatus 11 
#define leddata 12
#define SEND_DATA_SIZE  6
#define START_FLAG 253
#define END_FLAG 254
#define DEADRANGE 10

uint32_t uartspeed=UARTSPEED;
uint32_t airspeed=AIRSPEED;
uint32_t channel=CHANNEL;
uint8_t addr=ADDR;


SoftwareSerial E32Serial(3,2); //RX TX 
uint8_t RawData[19];
uint8_t PrevRawData[19];
uint8_t finalData[SEND_DATA_SIZE];
unsigned long time;
int enabled;
bool checkClick;
bool clicked;
unsigned long clickDelay;

uint8_t mina=111;
uint8_t spdset=111;
uint8_t code=100;
uint8_t crc;



unsigned long previousMillisbls = 0;  // blinkLED
int ledStatebls = LOW; //blinkLED
unsigned long lastSend=0;

void init(uint32_t uartspeed, uint32_t airspeed, uint32_t channel,uint8_t addr); //9600 19200 38400 57600 115200 // 300 1200 2400 4800 9600 19200 // 862...931 //
void blinkled(uint8_t ledname ,unsigned long interval);//1=always shin, 0=shut down
void AddAditionalData();
void ReceiveRawData();
void GetFinalData();
//void CheckEnabled();
void SendData(unsigned char data[SEND_DATA_SIZE]);
void SavePrevData();
bool Check();

void setup()
{
    Wire.begin();
    Serial.begin(115200);
    E32Serial.begin(9600);
    E32Serial.setTimeout(1);

  pinMode(leddata, OUTPUT);//should blink while data transmitted successfully
  pinMode(ledstatus, OUTPUT);//should be 1 after 'all is ready'
  pinMode(4, INPUT);
  pinMode(5, OUTPUT); 
  pinMode(6, OUTPUT);

  init(uartspeed,airspeed,channel,addr);
  blinkled(ledstatus,1);

  time = millis();
  // finalData = (unsigned char*)malloc(SEND_DATA_SIZE  * sizeof(unsigned char));
  checkClick = false;
  // finalData[0] = 127;
  // finalData[1] = 128;
  // finalData[2] = mina;
  // finalData[3] = spdset;
  // finalData[4] = code;
  // finalData[5] = crc;
  //////
}

void loop()
{
    ReceiveRawData();
    GetFinalData();
   // CheckEnabled();

     if(digitalRead(4)==1)
    {
      code++;
      if (code>=253)
      {code=111;}
       SendData(finalData);
      if (Serial.available())
      {
      unsigned long currentTime = millis();
    unsigned long interval = currentTime - lastSend;
    Serial.println(interval);
     for(int i = 0; i < 19; i++)
        {   
         Serial.print(RawData[i]);
         Serial.print("  ");
        }
        Serial.println("  ");
        
    lastSend = currentTime;
      }
    }
      if (RawData[0]==111)
      {blinkled(leddata,0);}
      else if (RawData[0]==223)
      {blinkled(leddata,100);}
      else
      {blinkled(leddata,1);}
    
    SavePrevData();

}

void SavePrevData()
{
  for(int i = 0; i < 19; i++)
  {
    PrevRawData[i] = RawData[i];
    
  }
}

void ReceiveRawData()
{
    Wire.requestFrom(9, 19);       // Запрос на получение данных от Slave Arduino(9 number of slave , 8 number of bytes)
    if (Wire.available() >= 19)
    { 
        for (int i = 0; i < 19; i++) 
        {
        RawData[i] = Wire.read();  // Чтение принятых данных и сохранение их в массив
        }
        // if (Serial.read()=='r')
        // {
        // for(int i = 0; i < 19; i++)
        // {   
        //  Serial.print(RawData[i]);
        //  Serial.print("  ");
        // }
        // Serial.println("  ");
        // }
    }
}

// void CheckEnabled()
// {
//     if(RawData[7] == 1)
//     {
//       if(!clicked)
//       {
//         if(!checkClick)
//         {
//           checkClick = true;
//           clickDelay = millis();
//         }
//         else
//         {
//           if(millis() - clickDelay > BUTTONDELAY)
//           {
//             enabled = enabled == 1 ? 0 : 1;        
//             clicked = true;
//           }
//         }
//       }
//     }
//     else
//     {
//       checkClick = false;
//       clicked = false;
//     }

// // //enabled = enabled == 1 ? 0 : 1;
// //     if(finalData[0] == 0 || enabled == 0)
// //     {
// //       finalData[1] = 128;
// //       finalData[4] = 128;
// //     }
// //     finalData[0] = enabled;
// }

void GetFinalData()
{
  
  
  if(RawData[17]!=0)
  {mina=112;}
  else if (RawData[15]!=0)
  {mina=113;}
  else 
  {mina=111;}
  if (RawData[18]!=0||RawData[16]!=0)
  {
  if (RawData[18]!=0)
  {spdset=112;}
   if (RawData[16]!=0)
  {spdset=113;}
   if (RawData[16]!=0 && RawData[18]!=0)
  {spdset=114;}
  }
  else 
  {spdset=111;}
     
    
    finalData[0] = RawData[4];//speed
    finalData[1] = RawData[5];//steer
    finalData[2] = mina;//mina
    finalData[3] = spdset;//speed settings
    finalData[4] = code;
    AddAditionalData();
    crc = ((finalData[0]+finalData[1]+finalData[2]+finalData[3])/8)+finalData[3];
    finalData[5] = crc;

}

void AddAditionalData()
{
  if(RawData[9] !=0)
  {
    finalData[0] = 48;
    finalData[1] = 128;
  }
  else if(RawData[7]!=0)
  {
    finalData[0] = 30;
    finalData[1] = 255;
  }
  else if(RawData[10]!=0)
  {
    finalData[0] = 208;
    finalData[1] = 128;
  }
    else if(RawData[8]!=0)
  {
    finalData[0] = 30;
    finalData[1] = 0;
  }
  else if(RawData[9] !=0 && RawData[7]!=0)
  {
    finalData[0] = 32;
    finalData[1] = 192;
  }
  else if(RawData[10] !=0 && RawData[7]!=0)
  {
    finalData[0] = 224;
    finalData[1] = 192;
  }
  else if(RawData[8]!=0 && RawData[10] !=0)
  {
    finalData[0] = 224;
    finalData[1] = 64;
  }
  else if(RawData[8]!=0 && RawData[9] !=0)
  {
    finalData[0] = 32;
    finalData[1] = 64;
  }
}

bool Check()
{
  for(int i = 0; i < 19; i++)
  {
    if(RawData[i] != PrevRawData[i])
      return true;
  }
  return false;
}

void SendData(unsigned char data[SEND_DATA_SIZE])
{
    unsigned char toSend[SEND_DATA_SIZE + 2];

    toSend[0] = START_FLAG;
    toSend[SEND_DATA_SIZE + 1] = END_FLAG;
    for(int i = 0; i < SEND_DATA_SIZE; i++)
    {
        toSend[i+ 1] = data[i];
    }
    if (Serial.available())
    {
   for(int i = 0; i < sizeof(toSend); i++)
        {   
         Serial.print(toSend[i]);
         Serial.print("  ");
        }
        Serial.println("  ");
    }
    E32Serial.write(toSend, sizeof(toSend));
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


void init(uint32_t uartspeed,uint32_t airspeed,uint32_t channel,uint8_t addr)
{
  unsigned int ch;
  unsigned int fb;
  unsigned int sb;
  
  if (uartspeed!=0)
  {
  if (uartspeed==9600)
  {fb=0b011;}
  if (uartspeed==19200)
  {fb=0b100;}
  if (uartspeed==38400)
  {fb=0b101;}
  if (uartspeed==57606)
  {fb=0b110;}
  if (uartspeed==115200)
  {fb=0b111;}
  }
  else 
  {uartspeed=9600;}
  if (airspeed!=0)
  {
  if (airspeed==300)
  {sb=0b000;}
  if (airspeed==1200)
  {sb=0b001;}
  if (airspeed==2400)
  {sb=0b010;}
  if (airspeed==4800)
  {sb=0b011;}
  if (airspeed==9600)
  {sb=0b100;}
  if (airspeed==19200)
  {sb=0b101;}
  }
  else 
  {airspeed=2400;}
 
  if (channel>=862 && channel<=931)
  {ch=channel-862;}
  else 
  {ch=0;}
  
  unsigned int sp=(fb<<3)|sb;

  digitalWrite(5, HIGH); // Логическая единица
  digitalWrite(6, HIGH); // Логическая единица
  delay(50);
  E32Serial.write(0xC0); // C0 - сохранить настройки, C2 - сбросить после отключения от питания
  E32Serial.write(0xFF);  // Верхний байт адреса. Если оба байта 0xFF - передача и прием по всем адресам на канале
  E32Serial.write(addr);  // Нижний байт адреса. Если оба байта 0xFF - передача и прием по всем адресам на канале
  E32Serial.write(sp); // Параметры скорости
  E32Serial.write(ch); // Канал (частота), 0x06 - 868 МГц, шаг частоты - 1 МГц (0x07 - 869 МГц)
  E32Serial.write(0x44); // Служебные опции
  delay(50);
  digitalWrite(5, LOW); // Логическая единица
  digitalWrite(6, LOW);
  delay(50);
  E32Serial.write(0xC1); // Передаём 3 байта, чтобы модуль вернул текущие параметры
  E32Serial.write(0xC1);
  E32Serial.write(0xC1);
  while(E32Serial.available()) // Получаем параметры и выводим их в монитор порта
  {
  int inByte = E32Serial.read();
  Serial.print(inByte, HEX);
  Serial.print(" ");
  }
  Serial.println(); // Переносим каретку на новую строку, чтобы данные не сливались друг с другом
  delay(50);
  E32Serial.end();
  delay(50);
  E32Serial.begin(uartspeed);
  delay(50);
  Serial.print("E32_900T30D was started "); Serial.println();
  Serial.print("LOW ADDRESS is: ");Serial.print(addr); Serial.println();
  Serial.print("MCU to E32 speed: "); Serial.print(uartspeed); Serial.println();
  Serial.print("Air Data Rate: ");  Serial.print(airspeed); Serial.println();
  Serial.print("Channel (Freqency): "); Serial.print(ch); Serial.print(" ("); Serial.print(channel); Serial.print(" MHz"); Serial.print(")"); Serial.println();
  Serial.println("For Debug send any symbol...");
}