#include <Wire.h>
#include <E32Uart.h>
#include <SoftwareSerial.h>
#include <stdio.h>
#include <stdlib.h>

#define ledstatus 11 
#define leddata 10
#define SEND_DATA_SIZE  5
#define START_FLAG 253
#define END_FLAG 254
#define DEADRANGE 10
#define BUTTONDELAY 500

SoftwareSerial E32Serial(3,2); //RX TX 
unsigned char RawData[19];
unsigned char PrevRawData[19];
unsigned char finalData[SEND_DATA_SIZE];
unsigned long time;
int enabled;
bool checkClick;
bool clicked;
unsigned long clickDelay;

uint8_t mina=0;
uint8_t spdset;
uint8_t code=100;

unsigned long previousMillisbls = 0;  // blinkLED
int ledStatebls = LOW; //blinkLED

void blinkledstatus(unsigned long interval);//1=not blink
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
    E32Serial.setTimeout(100);

  pinMode(leddata, OUTPUT);//should blink while data transmitted successfully
  pinMode(ledstatus, OUTPUT);//should be 1 after 'all is ready'
  pinMode(3, INPUT);
  pinMode(2, OUTPUT);
  pinMode(6, OUTPUT); 
  pinMode(7, OUTPUT);
  digitalWrite(6, HIGH); // Логическая единица
  digitalWrite(7, HIGH); // Логическая единица
  
  delay(500);
  E32Serial.write(0xC0); // C0 - сохранить настройки, C2 - сбросить после отключения от питания
  E32Serial.write(0xFF);  // Верхний байт адреса. Если оба байта 0xFF - передача и прием по всем адресам на канале
  E32Serial.write(0x01);  // Нижний байт адреса. Если оба байта 0xFF - передача и прием по всем адресам на канале
  E32Serial.write(0x2D); // Параметры скорости
  E32Serial.write(0x08); // Канал (частота), 0x06 - 868 МГц, шаг частоты - 1 МГц (0x07 - 869 МГц)
  E32Serial.write(0x44); // Служебные опции
  delay(500);
  digitalWrite(6, LOW); // Логическая единица
  digitalWrite(7, LOW);
  delay(500);
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
  delay(500);
  E32Serial.end();
  delay(500);
  E32Serial.begin(38400);
  delay(500);
  blinkledstatus(1);

  time = millis();
  // finalData = (unsigned char*)malloc(SEND_DATA_SIZE  * sizeof(unsigned char));
  enabled = 0;
  checkClick = false;
  finalData[0] = 127;
  finalData[1] = 128;
  finalData[2] = mina;
  finalData[3] = spdset;
  finalData[4] = code;
}

void loop()
{
    ReceiveRawData();
    GetFinalData();
   // CheckEnabled();

    if(Check())
    {
      SendData(finalData);
      time = millis(); 
    }
    else
    {
        if(millis() - time > 100)
        {
          time = millis();
          SendData(finalData);
        }
    }
    SavePrevData();
}

void SavePrevData()
{
  for(int i = 0; i < 13; i++)
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
        // for(int i = 0; i < 19; i++)
        // {   
        //  Serial.print(RawData[i]);
        //  Serial.print("  ");
        // }
        // Serial.println("  ");
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
  if (RawData[17]!=0||RawData[15]!=0)
  {
  if(RawData[17]!=0)
  {mina=1;}
  if (RawData[15]!=0)
  {mina=2;}
  }
  else 
  {mina=0;}
  if (RawData[18]!=0||RawData[16]!=0)
  {
  if (RawData[18]!=0)
  {spdset=1;}
   if (RawData[16]!=0)
  {spdset=2;}
   if (RawData[16]!=0 && RawData[18]!=0)
  {spdset=3;}
  }
  else 
  {spdset=0;}

    finalData[2] = mina;//mina
    finalData[3] = spdset;//speed settings
    finalData[4]=code;
    finalData[0] = RawData[4];//speed
    finalData[1] = RawData[5];//steer
   
      AddAditionalData();
 

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
  for(int i = 0; i < 13; i++)
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
for(int i = 0; i < sizeof(toSend); i++)
        {   
         Serial.print(toSend[i]);
         Serial.print("  ");
        }
        Serial.println("  ");
    E32Serial.write(toSend, sizeof(toSend));
}

void blinkledstatus(unsigned long interval)
 {
  if (interval==1)
  {digitalWrite(ledstatus,1);}
  else 
  {
  unsigned long currentMillis = millis();  // Получаем текущее время
  if (currentMillis - previousMillisbls >= interval) {
    previousMillisbls = currentMillis;
    if (ledstatus == LOW) {
      ledStatebls = HIGH;
    } else {
      ledStatebls = LOW;
    }
    digitalWrite(ledstatus, ledStatebls);
  }
}
}