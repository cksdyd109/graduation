#include <SoftwareSerial.h>
#include <Servo.h>

// submotor
Servo servo;

// bluetooth
#define BT_RXD A5
#define BT_TXD A4
SoftwareSerial bluetooth(BT_RXD, BT_TXD);


// motor port
int subMortor = 9;
int mortorRight = 5;
int mortorLeft = 6;

// Initialization
int angle;
char input = "";



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  bluetooth.begin(9600);
  
  //모터 포트 설정, 초기값
  pinMode(mortorRight, OUTPUT);
  pinMode(mortorLeft, OUTPUT);
  digitalWrite(mortorRight, LOW);
  digitalWrite(mortorLeft, LOW);
  //서보모터
  angle = 90;
  servo.attach(subMortor);
}

void loop() {
  // put your main code here, to run repeatedly:
  
   // bluetooth control
  if (bluetooth.available())
  {
    input = bluetooth.read();
    // move forward
    if (input == 'i')
    {
      analogWrite(mortorRight, 255);
      analogWrite(mortorLeft, 1);
    }
    else if (input == 'k')
    {
      analogWrite(mortorRight, 1);
      analogWrite(mortorLeft, 255);
    }
    else if (input == 'j')
    {
      for (int i = 0; i < 20; i++)
      {
        angle = angle + 1;
        if (angle >= 180)
          angle = 180;
        servo.write(angle);
        delay(10);
      }
    }
    else if (input == 'l')
    {
      for (int i = 0; i < 20; i++)
      {
        angle = angle - 1;
        if (angle >= 180)
          angle = 180;
        servo.write(angle);
        delay(10);
      }
    }
    else if (input == 'o')
    {
      analogWrite(mortorRight, 0);
      analogWrite(mortorLeft, 0);
      angle = 90;
      servo.write(angle);
    }
  }

}
