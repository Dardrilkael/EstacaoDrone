//https://www.youtube.com/watch?v=fBo3Yq9LK1U&t=486s
//https://www.youtube.com/watch?v=AbwQwxi0qaw
//https://components101.com/sites/default/files/component_datasheet/MQ-4%20Methane%20Gas%20Sensor%20Datasheet.pdf
//https://davidegironi.blogspot.com/2017/05/mq-gas-sensor-correlation-function.html#.XyxLkIgzbb0
//https://automeris.io/wpd/
//https://www.youtube.com/watch?v=HjY2CcHYR8E
/*
198.99754597957264; 1.7595138102004522
298.15162005803353; 1.5167067975538893
394.60853506402566; 1.3855604396994872
500.2594854576871; 1.2496512917783915
597.6326350373234; 1.194339770479718
695.0089567490493; 1.1269292998582316
791.0003471609897; 1.077122568315866
885.8197927685817; 1.0295403718944587
1008.1499165586224; 0.9903874801643513
1988.4640853097526; 0.769935941147508
2995.1615842002175; 0.6809786566394687
3964.2645230672047; 0.6141447331299836
4944.789183067113; 0.5683516447879128
5971.517832054689; 0.5328067561539367
6907.193792728543; 0.5027380619540954
7946.802645643756; 0.46830680288281074
8946.66381132412; 0.46522740204279434
9965.455842977808; 0.4418282743929923

*/
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_AHTX0.h>
#include <rtc_wdt.h>
#include "data.h"
#include "sd-repository.h"
#include "integration.h"
//#include "mqtt.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "GasSensor.h"

// Pin definitions
const int MG811_PIN = 34;   // Analog input pin for MG811 sensor
const int MQ4_PIN = 35;     // Analog input pin for MQ-4 sensor
const int MQ7_PIN = 36;     // Analog input pin for MQ-7 sensor
const int I2C_SDA = 21;     // Common SDA pin for BMP180 and AHT10
const int I2C_SCL = 22;     // Common SCL pin for BMP180 and AHT10

Adafruit_BMP085 bmp;    // Create BMP180 instance
Adafruit_AHTX0 aht;     // Create AHT10 instance

sensors_event_t humidity, temp; // Sensor data structures for AHT10
String formatedDateString = "";
std::string jsonConfig = "{}";
Metrics data;
Config config;
const char *configFileName = "/config.txt";
void convertTimeToLocaleDate(long timestamp);

#define INTERVAL 4000

void setup() {
  Serial.begin(9600); 
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  delay(3000);
  bool loadedSD =false;
  while(!loadedSD) loadedSD = loadConfiguration(SD, configFileName, config, jsonConfig);
  setupWifi("  - Wifi", config.wifi_ssid, config.wifi_password);
  connectNtp("  - NTP");

  Wire.begin(I2C_SDA, I2C_SCL);  // Initialize I2C communication for both sensors

  int count =0;
  if (!bmp.begin()) {
    Serial.println("BMP180 not found. Check wiring or I2C address.");
  }

  if (!aht.begin()) {
    Serial.println("AHT10 not found. Check wiring or I2C address.");
  }
}

void loop() {
  int begin = millis();
  timeClient.update();
  int timestamp = timeClient.getEpochTime();
  convertTimeToLocaleDate(timestamp);

  float divisor_tensao = 1.0;
  float conversionVoltage = 3.3/4095 * divisor_tensao;


  int mg811Value = analogRead(MG811_PIN);  // Read analog value from MG811 sensor
  float mg811Voltage = mg811Value*conversionVoltage;  // Convert analog value to voltage

  int mq4Value = analogRead(MQ4_PIN);  // Read analog value from MQ-4 sensor
  float mq4Voltage = mq4Value*conversionVoltage;  // Convert analog value to voltage

  int mq7Value = analogRead(MQ7_PIN);  // Read analog value from MQ-7 sensor
  float mq7Voltage = mq7Value*conversionVoltage;  // Convert analog value to voltage

  // Convert voltages to gas concentrations (example conversions, adjust as per sensor calibration)
  float CO2_ppm = mg811Voltage * 1000;  // Example conversion factor for MG811
  float MQ7_ppm = mq7Voltage * 100;     // Example conversion factor for MQ-7

  float VC = 3.3;
  float RL_VALUE = 20000.0f;
  float Rs = RL_VALUE * ((VC / mq4Voltage) - 1);
  //RO = Rs * ratio;
  Serial.printf("Voltage: %.2f",mq4Voltage);
   Serial.printf("RS: %.2f",Rs);
  float RO=41763;
  float ratio = Rs / RO;
  Serial.println(ratio);
  float a= 997.361;
  float b=-2.81457;
  float MQ4_ppm = a*pow(ratio,b);

  float temperature = bmp.readTemperature();  // Read temperature from BMP180 sensor in Celsius
  // Read humidity and temperature from AHT10
  if (!aht.getEvent(&humidity, &temp)) {
    Serial.println("Error reading AHT10 data.");
  }

  data.timestamp = timestamp;
  data.humidity = humidity.relative_humidity;
  data.temperature = temp.temperature;
  data.pressure =  bmp.readPressure() / 100.0F;
  data.co2=CO2_ppm;
  data.co = MQ7_ppm;
  data.ch4 = MQ4_ppm;

  const char* trying = parseData(data,config);
  
  storeMeasurement("/metricas", formatedDateString, trying);
  storeMeasurement("/metricas", formatedDateString, "\n");
  Serial.println(trying);
  int tempo = millis() - begin;
  delay((config.interval>tempo)*(config.interval-tempo));

}


void convertTimeToLocaleDate(long timestamp) {
  struct tm *ptm = gmtime((time_t *)&timestamp);
  int day = ptm->tm_mday;
  int month = ptm->tm_mon + 1;
  int year = ptm->tm_year + 1900;
  formatedDateString = String(day) + "-" + String(month) + "-" + String(year);
}
