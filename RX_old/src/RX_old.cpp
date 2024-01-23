#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <stdio.h>
#include <stdlib.h>

#define SEND_DATA_SIZE  5
#define DISCONNECTDELAY  2000
#define START_FLAG 253
#define END_FLAG 254

SoftwareSerial E32Serial(3,2); //Rx TX
unsigned char data[5];
unsigned char ToSend[SEND_DATA_SIZE];
int losePackages;
unsigned long disconectTime;
bool waitSignal;
void setup()
{
    Serial.begin(115200); // Монитор порта (компьютер)
    E32Serial.begin(9600); // Инициализация порта, куда подключен модуль (только для Arduino MEGA)
    E32Serial.setTimeout(150);
    Wire.begin(10);
    Wire.onRequest(SendData);



  pinMode(3, INPUT);
  pinMode(2, OUTPUT);
  pinMode(6, OUTPUT); 
  pinMode(7, OUTPUT);
  digitalWrite(6, HIGH); // Логическая единица
  digitalWrite(7, HIGH); // Логическая единица
  
  delay(500);
  E32Serial.write(0xC0); // C0 - сохранить настройки, C2 - сбросить после отключения от питания
  E32Serial.write(0xFF);  // Верхний байт адреса. Если оба байта 0xFF - передача и прием по всем адресам на канале
  E32Serial.write(0xFF);  // Нижний байт адреса. Если оба байта 0xFF - передача и прием по всем адресам на канале
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
  
  Init();
}

void Init()
{
    losePackages= 0;
    disconectTime = millis();
    waitSignal = true;

    data[0] = 0;
    data[1] = 128;
    data[2] = 0;
    data[3] = 0;
    data[4] = 128;
}

void loop()
{
    unsigned char* rec;

    rec = ReceiveData();

    if(rec != NULL)
    {
        
        SetData(rec);
        free(rec);
        disconectTime = millis();
        waitSignal = false;
    }
    else
    {
        if(!waitSignal && millis() - disconectTime > DISCONNECTDELAY)
        {
            waitSignal = true;
            // data[0] = 1;
            data[1] = 128;
            data[2] = 0;
            data[3] = 0;
            data[4] = 128;
            Wire.write((unsigned char*)ToSend, sizeof(ToSend));
            Serial.println("DISCONNECTED");
            disconectTime = millis();
        }
    }

    if(data != NULL)
    {
        SendData();
    }
    delay(70);
}

void SetData(unsigned char* rec)
{
    for(int i = 0; i < 5; i++)
    {
        data[i] = rec[i];
    }
}

unsigned char* ReceiveData()
{
    unsigned char* res;
    unsigned char marker;
    int i = 0;

  //  Serial.print("Find START_FLAG\n");
    while (E32Serial.available())
    {
        marker = E32Serial.read();
        if(marker == START_FLAG)
        {
          //  Serial.print("START_FLAG FINDED\n");
            break;
        }
    }

    res = (unsigned char*)malloc(SEND_DATA_SIZE * sizeof(unsigned char));

    while (E32Serial.available() && i < SEND_DATA_SIZE)
    {
        res[i] = E32Serial.read();
        if((int)res[i] == START_FLAG || (int)res[i] == END_FLAG)
        {
            // Serial.print("Broken Data\n");
            break;
        }
        i++;
    }
    if(i != SEND_DATA_SIZE || !E32Serial.available() || (E32Serial.available() && E32Serial.read() != END_FLAG))
    {
      // Serial.print(i);
      // Serial.print("   zero\n");
      losePackages += 1;
      free(res);
      return NULL;
    }
   Serial.print("Lose package ");
   Serial.print(losePackages);
   Serial.print("\n");
   losePackages= 0;

    return res;
}

void SendData()
{
    ToSend[0]=data[0];//enable
    ToSend[1]=data[2];//mina
    ToSend[2]=data[1];//speed
    ToSend[3]=data[4];//steer
    ToSend[4]=data[3];//speed settings

    // Serial.println("Data Sended");
    Wire.write((unsigned char*)ToSend, sizeof(ToSend));
}




