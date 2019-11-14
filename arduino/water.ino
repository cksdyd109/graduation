#include <string.h>
#include <SoftwareSerial.h>
#include <OneWire.h>
#include <avr/pgmspace.h>
#include <EEPROM.h>

// wifi
SoftwareSerial mySerial(9, 8);
// temperature
OneWire ds(2);

// Do
#define DoSensorPin  A2    //dissolved oxygen sensor analog output pin to arduino mainboard
#define VREF 5000    //for arduino uno, the ADC reference is the AVCC, that is 5000mV(TYP)
float doValue;      //current dissolved oxygen value, unit; mg/L
#define EEPROM_write(address, p) {int i = 0; byte *pp = (byte*)&(p);for(; i < sizeof(p); i++) EEPROM.write(address+i, pp[i]);}
#define EEPROM_read(address, p)  {int i = 0; byte *pp = (byte*)&(p);for(; i < sizeof(p); i++) pp[i]=EEPROM.read(address+i);}
#define ReceivedBufferLength 20
char receivedBuffer[ReceivedBufferLength+1];    // store the serial command
byte receivedBufferIndex = 0;
bool check = true;

#define SCOUNT  30           // sum of sample point
int analogBuffer[SCOUNT];    //store the analog value in the array, readed from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0,copyIndex = 0;

#define SaturationDoVoltageAddress 12          //the address of the Saturation Oxygen voltage stored in the EEPROM
#define SaturationDoTemperatureAddress 16      //the address of the Saturation Oxygen temperature stored in the EEPROM
float SaturationDoVoltage,SaturationDoTemperature;
float averageVoltage;
const float SaturationValueTab[41] PROGMEM = {      //saturation dissolved oxygen concentrations at various temperatures
14.62, 14.21, 13.82, 13.46, 13.10,
12.77, 12.44, 12.14, 11.84, 11.56,
11.28, 11.02, 10.77, 10.53, 10.30,
10.08, 9.87,  9.66,  9.46,  9.27,
9.09,  8.91,  8.74,  8.57,  8.41,
8.26,  8.11,  7.96,  7.82,  7.69,
7.56,  7.43,  7.30,  7.18,  7.06,
6.95,  6.83,  6.72,  6.62,  6.51,
6.41,
};

char* FloatToString(char*, double, byte, byte);
String sendData(String, const int, boolean);
// 온도
float getTemp();
// DO
boolean serialDataAvailable(void);
byte uartParse();
void doCalibration(byte mode, float temperature);
int getMedianNum(int bArray[], int iFilterLen);


// air pump
int airPump = 13;

// Initialization
char input = "";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  mySerial.begin(9600);

  delay(2000);

  // air pump initialization
  pinMode(airPump, OUTPUT);
  digitalWrite(airPump, HIGH);

  // DO
  pinMode(DoSensorPin,INPUT);
  readDoCharacteristicValues();
  
  mySerial.listen();
  sendData("AT+RST\r\n", 10000, true);  // restart the module
  sendData("AT+CWMODE=3\r\n", 5000, true); // wifi mode, 3: ap+station mode
  sendData("AT+CWJAP=\"AndroidHotspot2877\",\"19836171\"\r\n", 10000, true); // connect wifi
  sendData("AT+CIFSR\r\n", 10000, true);  // get ip address, and ready to access network
  sendData("AT+CIPMUX=1\r\n", 10000, true); // configure for multiple connections
}

void loop() {
  // put your main code here, to run repeatedly:
  float temperature = getTemp();
  
  // start TCP connection
  int buf[10];
  int swap_temp;
  unsigned long int avgValue = 0;
  int turbiVol = analogRead(A0);
  int phVol;
  

  
  double turbiValue, phValue;
  for(int i = 0; i < 10; i++)
  {
    buf[i]=analogRead(A1);
    delay(10);
  }

  // To find average value of center values
  for(int i = 0; i < 9; i++)
  {
    for(int j = i + 1; j < 10; j++)
    {
      if (buf[i] > buf[j])
      {
        swap_temp = buf[i];
        buf[i] = buf[j];
        buf[j] = swap_temp;
      }
    }
  }
  
  for(int i = 2; i < 8; i++)
  {
    avgValue += buf[i];
  }

  //DO calib
  if (!(SaturationDoVoltage > 0.0))
  {
	  doCalibration(1, temperature);
	doCalibration(2, temperature);
	doCalibration(3, temperature);
  }
  


  // DO
  static unsigned long analogSampleTimepoint = millis();
   if(millis()-analogSampleTimepoint > 30U)     //every 30 milliseconds,read the analog value from the ADC
   {
     analogSampleTimepoint = millis();
     analogBuffer[analogBufferIndex] = analogRead(DoSensorPin);    //read the analog value and store into the buffer
     analogBufferIndex++;
     if(analogBufferIndex == SCOUNT)
         analogBufferIndex = 0;
   }

   static unsigned long tempSampleTimepoint = millis();
   if(millis()-tempSampleTimepoint > 500U)  // every 500 milliseconds, read the temperature
   {
      tempSampleTimepoint = millis();
      //temperature = readTemperature();  // add your temperature codes here to read the temperature, unit:^C
   }

   static unsigned long printTimepoint = millis();
   if(millis()-printTimepoint > 1000U)
   {
      printTimepoint = millis();
      for(copyIndex=0;copyIndex<SCOUNT;copyIndex++)
      {
        analogBufferTemp[copyIndex]= analogBuffer[copyIndex];
      }
      Serial.println(averageVoltage);
      averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 1024.0; // read the value more stable by the median filtering algorithm
      doValue = pgm_read_float_near( &SaturationValueTab[0] + (int)round(temperature) ) * averageVoltage / SaturationDoVoltage;  //calculate the do value, doValue = Voltage / SaturationDoVoltage * SaturationDoValue(with temperature compensation)
      Serial.println(temperature);
      Serial.println(pgm_read_float_near( &SaturationValueTab[0] + (int)round(temperature) ));
      Serial.println(SaturationDoVoltage);
   }
   
  float tempdo = pgm_read_float_near( &SaturationValueTab[0] + (int)round(temperature) ) * 0.75;
   if (doValue <= tempdo)
  {
    digitalWrite(airPump, HIGH);
  }
  else
  {
    digitalWrite(airPump, LOW);
  }
  
  phVol = (double)avgValue * (5.0 / 1024.0 / 6.0);
  float turran = random(-2, 2) / (float)100;
  doValue = doValue + turran;
  turbiValue = turbiVol * (5.0 / 1024.0) + 4.8 + turran;
  phValue = -7.78 * phVol + 38.16;

  char ph_temp[20], turbidity_temp[20], temperature_temp[20], dovalue_temp[20];
  String ph(FloatToString(ph_temp, phValue, 2, 0));
  String turbidity(FloatToString(turbidity_temp, turbiValue, 2, 0));

  String web = "GET /insert_data.php?tur="+turbidity+"&ph="+ph+"&temp="+temperature+"&do="+doValue+" ";

  web += "HTTP/1.0\r\n";
  web += "Host: localhost\r\n";
  web += "\r\n";
  web += "\r\n";
  
  String cipsend = "AT+CIPSEND=";
  cipsend += 4;
  cipsend += ",";
  cipsend += String(web.length());
  cipsend += "\r\n";
  sendData("AT+CIPSTART=4, \"TCP\",\"52.78.54.131\", 80\r\n", 2000, true);
  sendData(cipsend, 2000, true);
  sendData(web, 10000, true);
}

String sendData(String command, const int timeout, boolean debug)
{
  String response = "";

  mySerial.print(command);

  long int time = millis();

  while((time + timeout) > millis())
  {
    while(mySerial.available())
    {
      char c = mySerial.read();
      response += c;
    }
  }

  if (debug)
    Serial.print(response);

  return response;  
}

char* FloatToString(char* outstr, double value, byte precision, byte widthp)
{
  char temp[20];
  byte i;

  temp[0] = '\0';
  outstr[0] = '\0';

  if(value < 0.0)
  {
    strcpy(outstr, "-\0");
    value *= -1;
  }

  if(precision == 0)
    strcpy(outstr, ltoa(round(value), temp, 10));
  else
  {
    unsigned long frac, mult = 1;
    byte padding = precision - 1;

    while (precision--)
      mult *= 10;

    value += 0.5/(float)mult;

    strcat(outstr, ltoa(floor(value), temp, 10));
    strcat(outstr, ".\0");

    frac = (value - floor(value)) * mult;

    unsigned long frac1 = frac;

    while (frac1 /= 10)
      padding--;

    while (padding--)
      strcat(outstr, "0\0");
    
    strcat(outstr, ltoa(frac, temp, 10));
  }

  if ((widthp != 0) && (widthp >= strlen(outstr)))
  {
    byte j = 0;
    j = widthp - strlen(outstr);

    for(i = 0; i < j; i++)
    {
      temp[i] = ' ';
    }

    temp[i++] = '\0';
    
    strcat(temp, outstr);
    strcpy(outstr, temp);
  }

  return outstr;
}

// get Temperature
float getTemp()
{
  byte data[12];
  byte addr[8];

  if (!ds.search(addr))
  {
    ds.reset_search();
    return -1000;
  }

  if (OneWire::crc8(addr, 7) != addr[7])
  {
    Serial.println("CRC is not valid!");
    return -1000;
  }

  if (addr[0] != 0x10 && addr[0] != 0x28)
  {
    Serial.print("Device is not recognized");
    return -1000;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);

  byte present = ds.reset();
  ds.select(addr);
  ds.write(0xBE);

  for (int i = 0; i < 9; i++)
  {
    data[i] = ds.read();
  }

  ds.reset_search();

  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB);
  float TemperatureSum = tempRead / 16;

  return TemperatureSum;
}

// DO
boolean serialDataAvailable(void)
{
  char receivedChar;
  static unsigned long receivedTimeOut = millis();
  while ( Serial.available() > 0 )
  {
    if (millis() - receivedTimeOut > 500U)
    {
      receivedBufferIndex = 0;
      memset(receivedBuffer,0,(ReceivedBufferLength+1));
    }
    receivedTimeOut = millis();
    receivedChar = Serial.read();
    if (receivedChar == '\n' || receivedBufferIndex == ReceivedBufferLength)
    {
    receivedBufferIndex = 0;
    strupr(receivedBuffer);
    return true;
    }else{
        receivedBuffer[receivedBufferIndex] = receivedChar;
        receivedBufferIndex++;
    }
  }
  return false;
}

byte uartParse()
{
    byte modeIndex = 0;
    if(strstr(receivedBuffer, "CALIBRATION") != NULL)
        modeIndex = 1;
    else if(strstr(receivedBuffer, "EXIT") != NULL)
        modeIndex = 3;
    else if(strstr(receivedBuffer, "SATCAL") != NULL)
        modeIndex = 2;
    return modeIndex;
}

void doCalibration(byte mode, float temperature)
{
    char *receivedBufferPtr;
    static boolean doCalibrationFinishFlag = 0,enterCalibrationFlag = 0;
    float voltageValueStore;
    switch(mode)
    {
      case 0:
      if(enterCalibrationFlag)
         Serial.println(F("Command Error"));
      break;

      case 1:
      enterCalibrationFlag = 1;
      doCalibrationFinishFlag = 0;
      break;

     case 2:
      if(enterCalibrationFlag)
      {
         EEPROM_write(SaturationDoVoltageAddress, averageVoltage);
         EEPROM_write(SaturationDoTemperatureAddress, temperature);
         SaturationDoVoltage = averageVoltage;
         SaturationDoTemperature = temperature;
         doCalibrationFinishFlag = 1;
      }
      break;

        case 3:
        if(enterCalibrationFlag)
        {
            doCalibrationFinishFlag = 0;
            enterCalibrationFlag = 0;
        }
        break;
    }
}

int getMedianNum(int bArray[], int iFilterLen)
{
      int bTab[iFilterLen];
      for (byte i = 0; i<iFilterLen; i++)
      {
      bTab[i] = bArray[i];
      }
      int i, j, bTemp;
      for (j = 0; j < iFilterLen - 1; j++)
      {
      for (i = 0; i < iFilterLen - j - 1; i++)
          {
        if (bTab[i] > bTab[i + 1])
            {
        bTemp = bTab[i];
            bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
         }
      }
      }
      if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
      else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
      return bTemp;
}

void readDoCharacteristicValues(void)
{
    EEPROM_read(SaturationDoVoltageAddress, SaturationDoVoltage);
    EEPROM_read(SaturationDoTemperatureAddress, SaturationDoTemperature);
    if(EEPROM.read(SaturationDoVoltageAddress)==0xFF && EEPROM.read(SaturationDoVoltageAddress+1)==0xFF && EEPROM.read(SaturationDoVoltageAddress+2)==0xFF && EEPROM.read(SaturationDoVoltageAddress+3)==0xFF)
    {
      SaturationDoVoltage = 1127.6;   //default voltage:1127.6mv
      EEPROM_write(SaturationDoVoltageAddress, SaturationDoVoltage);
    }
    if(EEPROM.read(SaturationDoTemperatureAddress)==0xFF && EEPROM.read(SaturationDoTemperatureAddress+1)==0xFF && EEPROM.read(SaturationDoTemperatureAddress+2)==0xFF && EEPROM.read(SaturationDoTemperatureAddress+3)==0xFF)
    {
      SaturationDoTemperature = 25.0;   //default temperature is 25^C
      EEPROM_write(SaturationDoTemperatureAddress, SaturationDoTemperature);
    }
}