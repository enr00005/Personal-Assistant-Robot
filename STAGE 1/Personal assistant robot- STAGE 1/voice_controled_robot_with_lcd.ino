#include <SoftwareSerial.h>
SoftwareSerial bluetooth(0,1); // RX, TX
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);
char t;

#define MLa 8     //left motor 1st pin
#define MLb 9     //left motor 2nd pin
#define MRa 10    //right motor 1st pin
#define MRb 11    //right motor 2nd pin
 
void setup()
{
  Serial.begin(9600);
bluetooth.begin(9600);
pinMode(MLa,OUTPUT);   //left motors forward
pinMode(MLb,OUTPUT);   //left motors reverse
pinMode(MRa,OUTPUT);   //right motors forward
pinMode(MRb,OUTPUT);   //right motors reverse
lcd.init();
lcd.backlight();
}
 
void loop() 
{
  lcd.clear();
  lcd.setCursor(4,0);
  lcd.print("PERSONAL");
  lcd.setCursor(3,1);
  lcd.print("Assistant");
  
if(bluetooth.available()>0)
{
  t = bluetooth.read();
  Serial.println(t);
}
 switch(t)
 {
  case 'F':
        {            //move forward(all motors rotate in forward direction)
          digitalWrite(MLa,HIGH);
          digitalWrite(MLb,LOW);
          digitalWrite(MRa,HIGH);
          digitalWrite(MRb,LOW);
        }
   break;
   
  case 'B':
        {      //move reverse (all motors rotate in reverse direction)
          digitalWrite(MLa,LOW);
          digitalWrite(MLb,HIGH);
          digitalWrite(MRa,LOW);
          digitalWrite(MRb,HIGH);
        }
   break;
   
  case 'L':
        {      //turn right (left side motors rotate in forward direction, right side motors doesn't rotate)
          digitalWrite(MLa,HIGH); 
          digitalWrite(MLb,LOW); 
          digitalWrite(MRa,LOW);
          digitalWrite(MRb,LOW);
        }
   break;
   
  case 'R':
        {      //turn left (right side motors rotate in forward direction, left side motors doesn't rotate)
          digitalWrite(MLa,LOW); 
          digitalWrite(MLb,LOW); 
          digitalWrite(MRa,HIGH);
          digitalWrite(MRb,LOW);
        }
   break;
   
  case 'S':
        {           //STOP (all motors stop)
          digitalWrite(MLa,LOW);
          digitalWrite(MLb,LOW); 
          digitalWrite(MRa,LOW);
          digitalWrite(MRb,LOW);
        }
 }
}
