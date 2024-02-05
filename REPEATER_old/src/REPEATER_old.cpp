#include <Wire.h>
#include <SoftwareSerial.h>
#include <stdio.h>
#include <stdlib.h>
#include "Arduino.h"
#define RECARRAY_SIZE 10
#define PWR_LED A3
#define LNK_LED A4
//RX
#define RXM0 7
#define RXM1 8
#define RXAUX 11
#define RXADDR 1 // 
#define RXUARTSPEED 115200 //9600 19200 38400 57600 115200 //
#define RXAIRSPEED 19200 // 300 1200 2400 4800 9600 19200 //
#define RXCHANNEL 890 // 862...931 // 
//TX
#define TXM0 2
#define TXM1 3
#define TXAUX 6
#define TXADDR 2 // 
#define TXUARTSPEED 115200 //9600 19200 38400 57600 115200 //
#define TXAIRSPEED 19200 // 300 1200 2400 4800 9600 19200 //
#define TXCHANNEL 890 // 862...931 // 
//
uint32_t RXuartspeed=RXUARTSPEED;
uint32_t RXairspeed=RXAIRSPEED;
uint32_t RXchannel=RXCHANNEL;
uint8_t  RXaddr=RXADDR;
//
uint32_t TXuartspeed=TXUARTSPEED;
uint32_t TXairspeed=TXAIRSPEED;
uint32_t TXchannel=TXCHANNEL;
uint8_t  TXaddr=TXADDR;
//
unsigned long previousMillisbls = 0;  // blinkLED
int ledStatebls = LOW; //blinkLED
//
bool acc=0;
//

SoftwareSerial RXE32(9,10); //RX TX 
SoftwareSerial TXE32(4,5); //RX TX 

void pars();
void blinkled(uint8_t ledname ,unsigned long interval);
void RXinit(uint32_t RXuartspeed, uint32_t RXairspeed, uint32_t RXchannel,uint8_t RXaddr); //9600 19200 38400 57600 115200 // 300 1200 2400 4800 9600 19200 // 862...931 //
void TXinit(uint32_t TXuartspeed, uint32_t TXairspeed, uint32_t TXchannel,uint8_t TXaddr); //9600 19200 38400 57600 115200 // 300 1200 2400 4800 9600 19200 // 862...931 //
void setup()
{
    Serial.begin(115200); 
  pinMode(RXAUX,INPUT);
  pinMode(TXAUX,INPUT);
  pinMode(RXM0, OUTPUT); 
  pinMode(RXM1, OUTPUT);
  pinMode(TXM0, OUTPUT); 
  pinMode(TXM1, OUTPUT);
  pinMode(PWR_LED, OUTPUT); 
  pinMode(LNK_LED, OUTPUT); 
  //
  TXE32.begin(9600);
  TXE32.setTimeout(1);
  TXinit(TXuartspeed,TXairspeed,TXchannel,TXaddr);
  //
  RXE32.begin(9600);
  RXE32.setTimeout(1);
  RXinit(RXuartspeed,RXairspeed,RXchannel,RXaddr);
  RXE32.listen();
 //
  digitalWrite(PWR_LED,1);
}



void loop() 
{
if (digitalRead(RXAUX)==1 && !acc)
{
  acc=1;
}
 if (digitalRead(RXAUX)==0 && acc) 
  {
    pars();
  }

}
   
    
void pars ()
{
    uint8_t RECRAW[8];
        while (RXE32.available())
        { 
            for (int i = 0; i < 8; i++) 
            {
                RECRAW[i] = RXE32.read();
            }
         if (RECRAW[0] == 253 && RECRAW[7] == 254)
          { 
          TXE32.write(RECRAW, sizeof(RECRAW));
          blinkled(LNK_LED,50);
           if ( Serial.available())
           {
           for(int i = 0; i <8; i++)
        {   
         Serial.print(RECRAW[i]);
         Serial.print("  ");
        }
        Serial.println("  ");
           }
            
            memset(RECRAW, 0, sizeof(RECRAW));
          }
          else  
          {
            blinkled(LNK_LED,0);
           memset(RECRAW, 0, sizeof(RECRAW));
          }
          acc=0;
          
        }

}    
   
   
   
    










void RXinit(uint32_t RXuartspeed, uint32_t RXairspeed, uint32_t RXchannel,uint8_t RXaddr)
{
  unsigned int ch;
  unsigned int fb;
  unsigned int sb;
  
  if (RXuartspeed!=0)
  {
  if (RXuartspeed==9600)
  {fb=0b011;}
  if (RXuartspeed==19200)
  {fb=0b100;}
  if (RXuartspeed==38400)
  {fb=0b101;}
  if (RXuartspeed==57606)
  {fb=0b110;}
  if (RXuartspeed==115200)
  {fb=0b111;}
  }
  else 
  {RXuartspeed=9600;}
  if (RXairspeed!=0)
  {
  if (RXairspeed==300)
  {sb=0b000;}
  if (RXairspeed==1200)
  {sb=0b001;}
  if (RXairspeed==2400)
  {sb=0b010;}
  if (RXairspeed==4800)
  {sb=0b011;}
  if (RXairspeed==9600)
  {sb=0b100;}
  if (RXairspeed==19200)
  {sb=0b101;}
  }
  else 
  {RXairspeed=2400;}
 
  if (RXchannel>=862 && RXchannel<=931)
  {ch=RXchannel-862;}
  else 
  {ch=0;}
  
  unsigned int sp=(fb<<3)|sb;

  delay(500);
  digitalWrite(RXM0, 1); // Логическая единица
  digitalWrite(RXM1, 1);
  delay(500);
  RXE32.write(0xC0); // C0 - сохранить настройки, C2 - сбросить после отключения от питания
  RXE32.write(0xFF);  // Верхний байт адреса. Если оба байта 0xFF - передача и прием по всем адресам на канале
  RXE32.write(RXaddr);  // Нижний байт адреса. Если оба байта 0xFF - передача и прием по всем адресам на канале
  RXE32.write(sp); // Параметры скорости
  RXE32.write(ch); // Канал (частота), 0x06 - 868 МГц, шаг частоты - 1 МГц (0x07 - 869 МГц)
  RXE32.write(0x44); // Служебные опции
  delay(500);
  digitalWrite(RXM0, 0); // Логическая единица
  digitalWrite(RXM1, 0);
  delay(500);
  RXE32.write(0xC1); // Передаём 3 байта, чтобы модуль вернул текущие параметры
  RXE32.write(0xC1);
  RXE32.write(0xC1);
  delay(500);
  while(RXE32.available()) // Получаем параметры и выводим их в монитор порта
  {
  int inByte = RXE32.read();
  Serial.print(inByte, DEC);
  Serial.print(" ");
  }
  Serial.println(); // Переносим каретку на новую строку, чтобы данные не сливались друг с другом
  delay(500);
  RXE32.end();
  delay(500);
  RXE32.begin(RXuartspeed);
  RXE32.setTimeout(1);
  delay(500);
  Serial.print("RX was started "); Serial.println();
  Serial.print("LOW ADDRESS is: ");Serial.print(RXaddr); Serial.println();
  Serial.print("MCU to E32 speed: "); Serial.print(RXuartspeed); Serial.println();
  Serial.print("Air Data Rate: ");  Serial.print(RXairspeed); Serial.println();
  Serial.print("Channel (Freqency): "); Serial.print(ch); Serial.print(" ("); Serial.print(RXchannel); Serial.print(")"); Serial.println();
  
}

void TXinit(uint32_t TXuartspeed, uint32_t TXairspeed, uint32_t TXchannel,uint8_t TXaddr)
{
  unsigned int ch;
  unsigned int fb;
  unsigned int sb;
  
  if (TXuartspeed!=0)
  {
  if (TXuartspeed==9600)
  {fb=0b011;}
  if (TXuartspeed==19200)
  {fb=0b100;}
  if (TXuartspeed==38400)
  {fb=0b101;}
  if (TXuartspeed==57606)
  {fb=0b110;}
  if (TXuartspeed==115200)
  {fb=0b111;}
  }
  else 
  {TXuartspeed=9600;}
  if (TXairspeed!=0)
  {
  if (TXairspeed==300)
  {sb=0b000;}
  if (TXairspeed==1200)
  {sb=0b001;}
  if (TXairspeed==2400)
  {sb=0b010;}
  if (TXairspeed==4800)
  {sb=0b011;}
  if (TXairspeed==9600)
  {sb=0b100;}
  if (TXairspeed==19200)
  {sb=0b101;}
  }
  else 
  {TXairspeed=2400;}
 
  if (TXchannel>=862 && TXchannel<=931)
  {ch=TXchannel-862;}
  else 
  {ch=0;}
  
  unsigned int sp=(fb<<3)|sb;

  delay(500);
  digitalWrite(TXM0, 1); // Логическая единица
  digitalWrite(TXM1, 1);
  delay(500);
  TXE32.write(0xC0); // C0 - сохранить настройки, C2 - сбросить после отключения от питания
  TXE32.write(0xFF);  // Верхний байт адреса. Если оба байта 0xFF - передача и прием по всем адресам на канале
  TXE32.write(TXaddr);  // Нижний байт адреса. Если оба байта 0xFF - передача и прием по всем адресам на канале
  TXE32.write(sp); // Параметры скорости
  TXE32.write(ch); // Канал (частота), 0x06 - 868 МГц, шаг частоты - 1 МГц (0x07 - 869 МГц)
  TXE32.write(0x44); // Служебные опции
  delay(500);
  digitalWrite(TXM0, 0); // Логическая единица
  digitalWrite(TXM1, 0);
  delay(500);
  TXE32.write(0xC1); // Передаём 3 байта, чтобы модуль вернул текущие параметры
  TXE32.write(0xC1);
  TXE32.write(0xC1);
  delay(500);
  while(TXE32.available()) // Получаем параметры и выводим их в монитор порта
  {
  int inByte = TXE32.read();
  Serial.print(inByte, DEC);
  Serial.print(" ");
  }
  Serial.println(); // Переносим каретку на новую строку, чтобы данные не сливались друг с другом
  delay(500);
  TXE32.end();
  delay(500);
  TXE32.begin(TXuartspeed);
  TXE32.setTimeout(1);
  delay(500);
  Serial.print("TX was started "); Serial.println();
  Serial.print("LOW ADDRESS is: ");Serial.print(TXaddr); Serial.println();
  Serial.print("MCU to E32 speed: "); Serial.print(TXuartspeed); Serial.println();
  Serial.print("Air Data Rate: ");  Serial.print(TXairspeed); Serial.println();
  Serial.print("Channel (Freqency): "); Serial.print(ch); Serial.print(" ("); Serial.print(TXchannel); Serial.print(")"); Serial.println();
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
  if (currentMillis - previousMillisbls >= interval) 
  {
    previousMillisbls = currentMillis;
    if (ledStatebls == LOW) 
    {
      ledStatebls = HIGH;
    } else
    {
      ledStatebls = LOW;
    }
    digitalWrite(ledname, ledStatebls);
  }
}
}