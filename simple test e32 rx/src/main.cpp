#include <Wire.h>
#include <SoftwareSerial.h>
#include <stdio.h>
#include <stdlib.h>
#include "Arduino.h"

#define RECARRAY_SIZE 10
#define ADDR 11 // 
#define UARTSPEED 115200 //9600 19200 38400 57600 115200 //
#define AIRSPEED 19200 // 300 1200 2400 4800 9600 19200 //
#define CHANNEL 920 // 862...931 // 
uint32_t uartspeed=UARTSPEED;
uint32_t airspeed=AIRSPEED;
uint32_t channel=CHANNEL;
uint8_t addr=ADDR;


unsigned long lastReceiveTime = 0;
String input;

unsigned long startTime = 0;
int waitTime = 7; 

uint8_t RECARRAY[RECARRAY_SIZE];

SoftwareSerial E32Serial(3,2); //RX TX 

void init(uint32_t uartspeed, uint32_t airspeed, uint32_t channel,uint8_t addr); //9600 19200 38400 57600 115200 // 300 1200 2400 4800 9600 19200 // 862...931 //
void setup()
{
    Wire.begin();
    Serial.begin(115200);
    E32Serial.begin(9600);
    E32Serial.setTimeout(2);

  pinMode(4,INPUT);
  pinMode(5, OUTPUT); 
  pinMode(6, OUTPUT);
  init(uartspeed,airspeed,channel,addr);
}



void loop() 
{
if (digitalRead(4)==0)
   {
    // unsigned long currentTime = millis();
    // unsigned long interval = currentTime - lastReceiveTime;
    // lastReceiveTime = currentTime;
    while (E32Serial.available())
    {
    for (int i=0;i<RECARRAY_SIZE;i++)
    {
      RECARRAY[i]=E32Serial.read();
      Serial.print(RECARRAY[i]);
      Serial.print(" ");
    }
    Serial.println();
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