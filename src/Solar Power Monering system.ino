/* IoT based Solar Power Monitoring System with ESP32 over cloud
  Author: https://www.circuitschools.com */

#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <BH1750.h>
#include <LiquidCrystal_I2C.h>

// Define voltage sensor analog input
#define SIGNAL_PIN 27
float adc_voltage = 0.0;

float R1 = 30000.0;
float R2 = 7500.0;
float ref_voltage = 5;
float in_voltage = 0.0;
int adc_value = 0;

//ACS712
const int sensorIn = 34;      // pin where the OUT pin from sensor is connected on Arduino
int mVperAmp = 185;           // this the 5A version of the ACS712 -use 100 for 20A Module and 66 for 30A Module
int Watt = 0;
double Voltage = 0;
double VRMS = 0;
double AmpsRMS = 0;

//DS18B20
#define ONE_WIRE_BUS 15
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

//BH1750
BH1750 lightMeter(0x23);

//LCD display
LiquidCrystal_I2C lcd (0x27, 16, 2);

void setup() {
  // Initialize serial and wait for port to open:
  Serial.begin(9600);
  // This delay gives the chance to wait for a Serial Monitor without blocking if none is found
  delay(1500);

  lcd. init ();
  lcd. backlight ();
  lcd. print ( "Solar monitor" );

  sensors.begin();//Dallas sensors
  Wire.begin(); //bh1750
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println(F("BH1750 Advanced begin"));
  }
  else {
    Serial.println(F("Error initialising BH1750"));
  }

  delay(3000);
  lcd. clear();
}

void loop() {
  //voltage sensor
  adc_value = analogRead(SIGNAL_PIN);
  adc_voltage  = (adc_value * ref_voltage) / 4096.0;
  in_voltage = adc_voltage / (R2 / (R1 + R2)) ;
  Serial.print("Input Voltage = ");
  Serial.print(in_voltage, 2);
  Serial.print(" V |");

  //acs712
  Voltage = getVPP();
  VRMS = (Voltage / 2.0) * 0.707; //root 2 is 0.707
  AmpsRMS = ((VRMS * 1000) / mVperAmp) - 0.3;
  Serial.print("Current = ");
  Serial.print(AmpsRMS, 2);
  Serial.print(" A |");

  //temp
  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(0);
  Serial.print("temperature = ");
  Serial.print(temp);
  Serial.print(" c |");

  //bh1750
  int lux = lightMeter.readLightLevel();
  Serial.print("light = ");
  Serial.print(lux);
  Serial.println(" lx");

  //print on lcd
  lcd.setCursor (0, 0);
  lcd.print("V=");
  lcd.print(in_voltage, 2);
  lcd.print("v|");

  lcd.setCursor (8, 0);
  lcd.print("C=");
  lcd.print(AmpsRMS, 2);
  lcd.print("A");

  lcd.setCursor (0, 1);
  lcd.print("T=");
  lcd.print(temp, 1);
  lcd.print("c|");

  lcd.setCursor (8, 1);
  lcd.print("L=");
  lcd.print(lux);
}

float getVPP()
{
  float result;
  int readValue;                // value read from the sensor
  int maxValue = 0;             // store max value here
  int minValue = 4096;          // store min value here ESP32 ADC resolution

  uint32_t start_time = millis();
  while ((millis() - start_time) < 1000) //sample for 1 Sec
  {
    readValue = analogRead(sensorIn);
    // see if you have a new maxValue
    if (readValue > maxValue)
    {
      /*record the maximum sensor value*/
      maxValue = readValue;
    }
    if (readValue < minValue)
    {
      /*record the minimum sensor value*/
      minValue = readValue;
    }
  }

  // Subtract min from max
  result = ((maxValue - minValue) * 3.3) / 4096.0; //ESP32 ADC resolution 4096

  return result;
}
