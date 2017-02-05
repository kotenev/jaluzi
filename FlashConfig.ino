bool loadConfig() {
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return false;
  }

  // загружаем файл конфигурации в глобальную переменную
  jsonConfig = configFile.readString();
  Serial.println(jsonConfig);
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(jsonConfig);
  ssidApName = root["ssidAPName"].as<String>();
  ssidApPass = root["ssidApPass"].as<String>();
  timeZone = root["timeZone"];
  ssdpName = root["ssdpName"].as<String>();
  ssidName = root["ssidName"].as<String>();
  ssidPass = root["ssidPass"].as<String>();
  Language = root["Lang"].as<String>();
  ddns = root["ddns"].as<String>();
  ddnsName = root["ddnsName"].as<String>();
  ddnsPort = root["ddnsPort"];
  speed = root["speed"];
  calibration = root["calibration"];
  turn = root["turn"];
  timeUp = root["timeUp"].as<String>();
  timeDown = root["timeDown"].as<String>();
  timeServo1 = root["timeServo1"];
  timeServo2 = root["timeServo2"];
  return true;
}





bool saveConfig() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(jsonConfig);
  json["ssdpName"] = ssdpName;
  json["ssidAPName"] = ssidApName;
  json["ssidApPass"] = ssidApPass;
  json["ssidName"] = ssidName;
  json["ssidPass"] = ssidPass;
  json["timeZone"] = timeZone;
  json["Lang"] = Language;
  json["ddns"] = ddns;
  json["ddnsName"] = ddnsName;
  json["ddnsPort"] = ddnsPort;
  json["calibration"] = calibration;
  json["speed"] = speed;
  json["turn"] = turn;
  json["timeUp"] = timeUp;
  json["timeDown"] = timeDown;
  json["timeServo1"] = timeServo1;
  json["timeServo2"] = timeServo2;
  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    //Serial.println("Failed to open config file for writing");
    return false;
  }

  json.printTo(configFile);
  return true;
}






