#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <map>
#include <functional>
#include "FS.h"
#include <LittleFS.h>
#include <WiFi.h>
#include "mx.h"

extern String ver;

extern bool isMounted;

extern int terminalRows;
extern int terminalCols;

extern String currentPath;
extern int currentDisk; // 0 - LittleFS, 1 - SD card

extern String username; // Имя пользователя
extern String machineName; // Имя системы
extern String autoMountSD;
extern String autoMountPin;

extern const char* configFilePath;
extern const char* wifiKnownFilePath;
extern const char* wifiConfigFilePath;

extern bool isRoot;
extern String command;

extern std::map<String, String*> configMap;


struct helpCommand {
    const char* name;
    const char* description;
};

const helpCommand commands[] = {
    {"help", "Show list with helpful notes (this)"},
    {"sw", "Serial text editor, simple and fast"},
    {"mount", "Mount pin as SD Card"}, // mount [pin]
    {"unmount", "Unmount SD Card"},
    {"settings", "Change/look device settings"}, // settings <set/get/list/keys> <key> [value]
    {"pin", "Control any pin from terminal"}, // pin <num> mode/set [analog] <mode:[OUTPUT/INPUT/PULLUP/PULLDN]> <set:[digital:low/high/0/1 analog:0-255]>
    {"mx", "Small mx file editor"},
    {"neofetch", "System information"}
};

struct Command {
  const char* name;
  void (*handler)(String);
};

// Объявления функций
void helpFunc();
bool mount(String command);
bool unmount();
void updateConfig(const String& key, const String& value);
bool saveConfig();
String getConfig(const String& key);
void processSettingsCommand(const String& command);
void listAllSettings();
void listAllKeys();
void neofetch();
void writeUser();
void getTerminal();
void pinWorker(const String& command);
String progressBar(int used, int total, int width);