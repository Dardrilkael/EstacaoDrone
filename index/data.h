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
//MQ4
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

//MQ7
/*
49.93873074261034; 1.615598098439873
61.30557921498208; 1.3768571648527583
80.29481466318589; 1.154781984689457
90.4169805923451; 1.066050498984791
99.64086750810823; 0.9841369898850127
198.80966796973556; 0.6091471578453569
302.8656361143965; 0.46415888336127775
396.6774383514887; 0.3892940605763557
502.9936472466874; 0.3317671127842855
604.2963902381334; 0.29663488391777315
702.8699885226708; 0.26522295600204077
791.475543941116; 0.2528004122200023
891.250938133745; 0.23337565805823296
1003.604269020064; 0.22244478173594331
2002.4537771176244; 0.14216011881646262
2953.3335566049964; 0.11006941712522092
3995.4205589498865; 0.09085175756516871

*/
