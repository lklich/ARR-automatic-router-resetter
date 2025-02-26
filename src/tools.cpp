#include <tools.h>
#include <version.h>

Tools::Tools() {}

void Tools::displayWelcome(){
  Logger::getInstance().log(LOG_INFO, DEVICENAME);
  String ver = "VERSION " + String(VERSION); 
  Logger::getInstance().log(LOG_INFO, ver.c_str());
}

void Tools::getChipType(){
  String chipID = "CHIP MODEL: " + String(ESP.getChipModel());
  Logger::getInstance().log(LOG_INFO, chipID.c_str());
}

double Tools::CalculateSquare(int number) {
     return pow(number, 2);
}

void Tools::checkFlash() {
  #ifdef ESP32
    uint32_t chipSize = ESP.getFlashChipSize(); 
    uint32_t ideSize = ESP.getFlashChipSize();
    FlashMode_t ideMode = ESP.getFlashChipMode();
    #else
    uint32_t realSize = ESP.getFlashChipRealSize(); 
    uint32_t ideSize = ESP.getFlashChipSize();
    FlashMode_t ideMode = ESP.getFlashChipMode();
    uint32_t flashId = ESP.getFlashChipId();
  #endif

   String flashSize = "FLASH SIZE: " + String(ideSize / (1024 * 1024)) + "MB";
   Logger::getInstance().log(LOG_INFO, flashSize.c_str());

   String flashSpeed = "FLASH SPEED: " + String(ESP.getFlashChipSpeed() / 1000000) + "MHz";
   Logger::getInstance().log(LOG_INFO, flashSpeed.c_str());

  String flashMode = "FLASH MODE: " + String(ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" :
    ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN");
  Logger::getInstance().log(LOG_INFO, flashMode.c_str());
}
