#include <usbhid.h>
#include <hiduniversal.h>
#include <usbhub.h>
#include <Wire.h>
#include "Arduino.h"

// Satisfy IDE, which only needs to see the include statment in the ino.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

#include "hidjoystickrptparser.h"

#define I2C_SLAVE_ADDRESS 9

USB Usb;
USBHub Hub(&Usb);
JoystickEvents JoyEvents;
JoystickReportParser Joy(&JoyEvents);
HIDUniversal Hid(&Usb);
uint8_t data[19];
uint8_t defaultdata[19]={111,0,15,128,127,128,127,0,0,0,0,0,0,0,0,0,0,0,0};
uint8_t errordata[19]={223,0,15,128,127,128,127,0,0,0,0,0,0,0,0,0,0,0,0};
uint8_t analogy;
bool error=1;
bool analog=0;
bool start=0;

void DataChanged(uint8_t *newData);
void sendData();
void setup() 
{
        Wire.begin(I2C_SLAVE_ADDRESS);
        Wire.onRequest(sendData);
        JoyEvents.DataChangedCallback = &DataChanged;
        Serial.begin(115200);
#if !defined(__MIPSEL__)
        while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
        Serial.println("Start");
        start=1;
        if (Usb.Init() == -1)
                Serial.println("OSC did not start.");
        error=1;
        delay(200);

        if (!Hid.SetReportParser(0, &Joy))
                ErrorMessage<uint8_t > (PSTR("SetReportParser"), 1);

        for(int i = 0; i < 19; i++)
        {
                data[i] = defaultdata[i];   
        }

}

void loop() {
        Usb.Task();      
}

void DataChanged(uint8_t *newData)
{
        error=0;
        if (start==1 && data[1]==16)
        {analogy++;}
        if (analogy==2 )
        {     
                analog=!analog;
                analogy=0;
        }
        for(int i = 0; i < 19; i++)
        {
                data[i] = newData[i];     
        }
        //  if (Serial.available())
        //  {  for(int i = 0; i < 19; i++)
        // {
        //         Serial.print(data[i]);
        //         Serial.print("  ");
        // }
        // Serial.print(analogy);Serial.print(analog);Serial.print(error);
        // Serial.println();
        // }
}

void sendData() 
{  
         if (error)
        {
        Wire.write((byte*)errordata, sizeof(errordata));
        if (Serial.available())
        {Serial.println("223no_data");}
        }
        else  if (!analog)
        {
        Wire.write((byte*)defaultdata, sizeof(defaultdata));
        if (Serial.available())
        {Serial.println("111not_analog_data");}
        } 
        else 
        {
        Wire.write((byte*)data, sizeof(data));
        if (Serial.available())
        {Serial.println("0good_data");}
        }

}

