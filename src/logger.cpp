#include "logger.h"

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::begin(unsigned long baudRate) {
    Serial.begin(baudRate);
     while (!Serial) { ; } // Czekaj na port 
    if (!LittleFS.begin()) 
        fsExist = false;
    else 
        fsExist = true;
        clearLog();
}

void Logger::log(LogLevel level, const char* message) {
    Serial.print("[");
    Serial.print(levelToString(level));
    Serial.print("] ");
    Serial.println(message);
    if(fsExist) 
        writeLogFS(message);
}

void Logger::clearLogIfSize() {
    if (LittleFS.exists(fileLOGPath)) {
    File file = LittleFS.open(fileLOGPath, "r");
    if (file) {
      size_t fileSize = file.size();
      file.close();
      if (fileSize > maxFileSize) { 
        Serial.println("LOG File exceeds, clearing"); 
        file = LittleFS.open(fileLOGPath, "w");
        if (!file) {
          Serial.println("LOG clear failed");
          return;
        }
        file.print("");
        file.close();
      }
    }
  }
}

void Logger::clearLog() {
    if ((LittleFS.exists(fileLOGPath)) && (fsExist)) {
    File file = LittleFS.open(fileLOGPath, "w");
    if (file) {
        file.print("");
        file.close();
      }
    }
  }


void Logger::writeLogFS(const String& content) {
  if(!fsExist) return;
  clearLogIfSize();  // Check is LOG file is grather an > maxFileSize. If true, clear.
  File file = LittleFS.open(fileLOGPath, "a");
  if (!file) {
    Serial.println("Failed to open LOG file for writing");
    return;
  } 
  file.print("");  // truncate
  file.println(content); 
  file.close();
}

void Logger::logInfo(const char* message) {
    log(LOG_INFO, message);
}

void Logger::logWarning(const char* message) {
    log(LOG_WARNING, message);
}

void Logger::logError(const char* message) {
    log(LOG_ERROR, message);
}

const char *Logger::levelToString(LogLevel level) {
    switch (level) {
        case LOG_INFO:
            return "INFO";
        case LOG_WARNING:
            return "WARNING";
        case LOG_ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}
