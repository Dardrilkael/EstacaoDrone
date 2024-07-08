#pragma once
// --- Config data  ---
struct Config{
    char station_name[64]{0};
    char wifi_ssid[64]{0};
    char wifi_password[64]{0};
    char mqtt_server[64]{0};
    char mqtt_username[64]{0};
    char mqtt_password[64]{0};
    int mqtt_port;
    int interval;
};

// --- Metrics data  ---

struct Metrics {

  long timestamp=0;
  float humidity = 0;
  float temperature = 0;
  float pressure = 0;
  float co2 = 0;
  float co = 0;
  float ch4 = 0;
};

char* parseData(const Metrics& data, const Config& config) {
  // Static buffer to hold the JSON output
  static char jsonOutput[256];

  // Prepare the JSON template
  const char *json_template = "{\"timestamp\": %ld, \"slug\": \"%s\", \"humi\": %s, \"temp\": %s, \"pres\": %s, \"co2\": %s, \"co\": %s, \"ch4\": %s}";

  // Create the JSON string using snprintf
  snprintf(jsonOutput, sizeof(jsonOutput), json_template,
           data.timestamp,
           config.station_name,
           isnan(data.humidity) ? "null" : String(data.humidity, 2).c_str(),
           isnan(data.temperature) ? "null" : String(data.temperature, 2).c_str(),
           (data.pressure == -1) ? "null" : String(data.pressure, 2).c_str(),
           (data.co2 == -1) ? "null" : String(data.co2, 2).c_str(),
           (data.co == -1) ? "null" : String(data.co, 2).c_str(),
           (data.ch4 == -1) ? "null" : String(data.ch4, 2).c_str());

  return jsonOutput;
}


