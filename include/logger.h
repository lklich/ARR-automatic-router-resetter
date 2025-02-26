#ifndef LOGGER_H
#define LOGGER_H

/* 
  Singleton Logger
  w klasie cpp: #include "Logger.h"
   Logger::getInstance().logInfo("MyClass object created.");
*/
#include <Arduino.h>
#include <LittleFS.h>
#include <FS.h>

enum LogLevel {
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
};

class Logger {
public:
    static Logger &getInstance();
    void begin(unsigned long baudRate);
    void log(LogLevel level, const char* message);
    void logInfo(const char* message);
    void logWarning(const char* message);
    void logError(const char* message);
    void clearLog();

private:
    String fileLOGPath = "/log.txt";
    Logger() {} // Konstruktor prywatny
    Logger(const Logger&) = delete;
    Logger &operator=(const Logger&) = delete;
    void writeLogFS(const String& content);
    void clearLogIfSize();
    bool fsExist = false;  // FS istnieje
    size_t maxFileSize = 200 * 1024; // 200KB
    const char *levelToString(LogLevel level);
};

#endif
