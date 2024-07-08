#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_AHTX0.h>
#include "GasSensor.h" // Include your GasSensor class header file here
#include "data.h"
#include "sd-repository.h"
#include "integration.h"
//#include "mqtt.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// Pin definitions
const int MG811_PIN = 34;   // Analog input pin for MG811 sensor
const int MQ4_PIN = 35;     // Analog input pin for MQ-4 sensor
const int MQ7_PIN = 36;     // Analog input pin for MQ-7 sensor
const int I2C_SDA = 21;     // Common SDA pin for BMP180 and AHT10
const int I2C_SCL = 22;     // Common SCL pin for BMP180 and AHT10

Adafruit_BMP085 bmp;    // Create BMP180 instance
Adafruit_AHTX0 aht;     // Create AHT10 instance

GasSensor mq4Sensor(MQ4_PIN, 997.361, -2.81457, 20000.0, 1, 3.3); // Example parameters for MQ-4 sensor
GasSensor mq7Sensor(MQ7_PIN, 89.295, -1.58996, 20000.0, 17940.3421723, 3.3); // Example parameters for MQ-7 sensor


sensors_event_t humidity, temp; // Sensor data structures for AHT10
String formatedDateString = "";
std::string jsonConfig = "{}";
Metrics data;
Config config;
const char *configFileName = "/config.txt";
int before = 0;


void setup() {
  Serial.begin(9600); 
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  delay(3000);
  bool loadedSD = false;
  while(!loadedSD) loadedSD = loadConfiguration(SD, configFileName, config, jsonConfig);
  setupWifi("  - Wifi", config.wifi_ssid, config.wifi_password);
  connectNtp("  - NTP");

  Wire.begin(I2C_SDA, I2C_SCL);  // Initialize I2C communication for both sensors

  if (!bmp.begin()) {
    Serial.println("BMP180 not found. Check wiring or I2C address.");
  }

  if (!aht.begin()) {
    Serial.println("AHT10 not found. Check wiring or I2C address.");
  }
}

void loop() {
  timeClient.update();
  int timestamp = timeClient.getEpochTime();
  convertTimeToLocaleDate(timestamp);
  int now = millis();
  
  if (now - before > config.interval) {
    before = now;

    // Read MG-811 sensor values
    float mg811Voltage = analogRead(MG811_PIN) * 3.3 / 4095;  // Convert analog value to voltage
    float CO2_ppm = mg811Voltage * 1000;  // Example conversion factor for MG811


    float temperature = bmp.readTemperature();  // Read temperature from BMP180 sensor in Celsius

    // Read humidity and temperature from AHT10
    if (!aht.getEvent(&humidity, &temp)) {
      Serial.println("Error reading AHT10 data.");
    }

    // Populate data structure with sensor readings
    data.timestamp = timestamp;
    data.humidity = humidity.relative_humidity;
    data.temperature = temp.temperature;
    data.pressure =  bmp.readPressure() / 100.0F;
    data.co2 = CO2_ppm;
    data.co = mq7Sensor.readConcentration();
    data.ch4 = mq4Sensor.readConcentration();

    // Process data and store or transmit
    const char* parsedData = parseData(data, config);
    storeMeasurement("/metrics", formatedDateString, parsedData);
    storeMeasurement("/metrics", formatedDateString, "\n");
    Serial.println(parsedData);
  }
}

void convertTimeToLocaleDate(long timestamp) {
  struct tm *ptm = gmtime((time_t *)&timestamp);
  int day = ptm->tm_mday;
  int month = ptm->tm_mon + 1;
  int year = ptm->tm_year + 1900;
  formatedDateString = String(day) + "-" + String(month) + "-" + String(year);
}
