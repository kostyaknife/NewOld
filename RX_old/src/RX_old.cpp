#include <Wire.h>
#include <SoftwareSerial.h>
#include <stdio.h>
#include <stdlib.h>
#include "Arduino.h"


#define RXTIMEOUT 4000 //if no new data is received during the “RXTIMEOUT” period, the data for the "mastercontrolled" will be changed to the default data
#define NOTPARSED 20// if received data will not be successfully parsed for "NOTPARSED" the data for the "mastercontrolled" will be changed to the default data
#define ADDR 11 // 
#define UARTSPEED 115200 //9600 19200 38400 57600 115200 //
#define AIRSPEED 19200 // 300 1200 2400 4800 9600 19200 //
#define CHANNEL 920 // 862...931 // 
uint32_t uartspeed=UARTSPEED;
uint32_t airspeed=AIRSPEED;
uint32_t channel=CHANNEL;
uint8_t addr=ADDR;

uint32_t lasttime=0;
uint32_t curtime;

uint32_t starttime=0;
uint32_t currenttime;
uint32_t interval;

uint8_t RECARRAY[5];
uint8_t DEFARRAY[5]={127,128,111,111,5};


bool ac=0;
bool gooddata=0;
uint8_t notparsed=0;

SoftwareSerial E32Serial(3,2); //RX TX 


void SendData();
void pars ();
void init(uint32_t uartspeed, uint32_t airspeed, uint32_t channel,uint8_t addr); //9600 19200 38400 57600 115200 // 300 1200 2400 4800 9600 19200 // 862...931 //


void setup()
{
 
  Serial.begin(115200);
  E32Serial.begin(9600);
  E32Serial.setTimeout(1);

  pinMode(4,INPUT);
  pinMode(5, OUTPUT); 
  pinMode(6, OUTPUT);
  init(uartspeed,airspeed,channel,addr);
  Wire.begin(10);
  Wire.onRequest(SendData);
}



void loop() 
{   
  if (digitalRead(4)==1 && !ac)
  {
    ac=1;
  }
    if (digitalRead(4)==0 && ac) 
  {
    pars();
    starttime=millis();
  }
   if ( digitalRead(4)==1 && ac)
  {
    interval=millis()-starttime;
  }
  if (interval>=RXTIMEOUT || notparsed>=NOTPARSED)
  {
    gooddata=0;
  }
  if (Serial.available())
  {
    curtime=millis();
    if (curtime-lasttime>=100)
    {
    lasttime=curtime;
    Serial.print("notparsed  ");Serial.print(notparsed); Serial.println();
    Serial.print("interval  ");Serial.print(interval); Serial.println();
    Serial.print("RECARRAY : ");
    for(int i = 0; i < 8; i++)
        {   
         Serial.print(RECARRAY[i]);
         Serial.print("  ");
        }
        Serial.println("  ");
    }
  }

}
    
void pars()
{
    uint8_t RECRAW[8];
        while (E32Serial.available())
        { 
            for (int i = 0; i < 8; i++) 
            {
                RECRAW[i] = E32Serial.read();
            }
         if (RECRAW[0] == 253 && RECRAW[7] == 254 && RECRAW[6] ==(((RECRAW[1]+RECRAW[2]+RECRAW[3]+RECRAW[4])/8)+RECRAW[4]))
          { 
            gooddata=1;
            notparsed=0;
            RECARRAY[0]=RECRAW[1];
            RECARRAY[1]=RECRAW[2];
            RECARRAY[2]=RECRAW[3];
            RECARRAY[3]=RECRAW[4];
            RECARRAY[4]=0;
            memset(RECRAW, 0, sizeof(RECRAW));
          }
          else  
          {
           notparsed++;
           memset(RECRAW, 0, sizeof(RECRAW));
          }
          ac=0;
        }
}
  void SendData()
{
    if (!gooddata)
    { Wire.write(DEFARRAY, sizeof(DEFARRAY));}
    else if (gooddata)
    { Wire.write(RECARRAY, sizeof(RECARRAY));}
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


  delay(500);
  E32Serial.write(0xC0); // C0 - сохранить настройки, C2 - сбросить после отключения от питания
  E32Serial.write(0xFF);  // Верхний байт адреса. Если оба байта 0xFF - передача и прием по всем адресам на канале
  E32Serial.write(addr);  // Нижний байт адреса. Если оба байта 0xFF - передача и прием по всем адресам на канале
  E32Serial.write(sp); // Параметры скорости
  E32Serial.write(ch); // Канал (частота), 0x06 - 868 МГц, шаг частоты - 1 МГц (0x07 - 869 МГц)
  E32Serial.write(0x44); // Служебные опции
  delay(500);
  digitalWrite(5, LOW); // Логическая единица
  digitalWrite(6, LOW);
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
  E32Serial.begin(uartspeed);
  delay(500);
  Serial.print("E32_900T30D was started "); Serial.println();
  Serial.print("LOW ADDRESS is: ");Serial.print(addr); Serial.println();
  Serial.print("MCU to E32 speed: "); Serial.print(uartspeed); Serial.println();
  Serial.print("Air Data Rate: ");  Serial.print(airspeed); Serial.println();
  Serial.print("Channel (Freqency): "); Serial.print(ch); Serial.print(" ("); Serial.print(channel); Serial.print(")"); Serial.println();
  Serial.println("For Debug send any symbol...");
}