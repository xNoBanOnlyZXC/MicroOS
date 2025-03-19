// #include <ota.h>
#include "system.h"

TextEditor editor;

// const int CS_PIN = 5; // Пин CS для SDCARD

void processCommand();
bool saveConfig();
bool updateConfig();

String currentPath = "/";
int currentDisk = 0; // 0 - LittleFS, 1 - SD card

Command commandList[] = {
  {"help", [](String) { helpFunc(); }},
  {"mount", [](String cmd) { mount(cmd); }},
  {"unmount", [](String) { unmount(); }},
  {"settings", [](String cmd) { processSettingsCommand(cmd); }},
  {"neofetch", [](String) { neofetch(); }},
  {"tcheck", [](String) { getTerminal(); }},
  {"reboot", [](String) {
    Serial.println("Restarting ESP...");
    delay(200);
    for (int pin = 0; pin < 40; pin++) {
    pinMode(pin, INPUT);
    }
    ESP.restart();
  }},
  {"pin", [](String cmd) { pinWorker(cmd); }},
  {"mx", [](String cmd) {
    int spaceIndex = cmd.indexOf(' ');
    if (spaceIndex != -1) {
      String filePath = cmd.substring(spaceIndex + 1);
    if (filePath.startsWith("/")) {
        filePath = filePath;
    } else if (filePath.startsWith("./")) {
        filePath = currentPath + filePath;
    } else {
        filePath = currentPath + "/" + filePath;
    }
      filePath.replace("//", "/");
      editor.begin(filePath);
      editor.run();
    } else {
      Serial.println("No file path provided");
    }
  }},
  {"ls", [](String cmd) { listFiles(cmd);}},
  {"cd", [](String cmd) { changeDir(cmd);}},
  {"touch", [](String cmd) {touchFile(cmd);}},
  {"log", [](String cmd) { logPin(cmd); }},
  {"rm", [](String cmd) { removeFile(cmd); }},
  {"mv", [] (String cmd) { moveFile(cmd); }},
  {"cat", [](String cmd) { catFile(cmd); }},
  {"echo", [](String cmd) {
    String text = cmd.substring(4);
    text.trim();
    Serial.println(text);
  }}
};

void setup() {
  String ver = "0.02.27";

  Serial.begin(115200);
  Serial.print("\033[2J\033[H");
  
  // Включить светодиод
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);

  // LittleFS part
  Serial.println("Mounting LittleFS...");
  if (!LittleFS.begin()) {
        Serial.println("Error while mounting LittleFS. Formatting...");
        
        if (!LittleFS.format()) {
          Serial.println("Error while formatting LittleFS");
        } else {
          Serial.println("Successfully formated LittleFS");
        }
        
        if (!LittleFS.begin()) {
            Serial.println("Could not mount LittleFS even after formatting");
            return;
        } else {
            Serial.println("LittleFS successfully mounted");
        }
    } else {
        Serial.println("LittleFS successfully mounted");
    }
  if (!LittleFS.exists("/sys")) {
      if (!LittleFS.mkdir("/sys")) {
          Serial.println("Failed to create directory /sys");
      }
  }

  if (LittleFS.exists(configFilePath)) {
    Serial.println("Loading system configuration...");
    File configFile = LittleFS.open(configFilePath, "r");

    if (!configFile) {
      Serial.println("Error while reading configuration\nLoading default values");
      return;
    }

    while (configFile.available()) {
      String line = configFile.readStringUntil('\n');
      line.trim();
      int sep = line.indexOf('=');
      if (sep != -1) {
        String key = line.substring(0, sep);
        String val = line.substring(sep+1);
        if (configMap.find(key) != configMap.end()) {
            *(configMap[key]) = val;
        } else {
            Serial.println("Unknown key: " + key);
        }
      }
    }
    Serial.println("System configuration loaded successfully");
  } else {
    Serial.println("System configuration not exists. Saving defaults");
    saveConfig();
  }
  Serial.print("\nWelcome to MicroOS " + ver + "\r\n\nType \"help\" for more information.\r\n");
  writeUser();
}

void writeUser() {
  Serial.print(username + "@" + machineName + ":" + (isMounted ? (currentDisk ? "sd" : "fs") : "") + currentPath + (isRoot ? "#" : "$") + " ");
}

void loop() {

  if (Serial.available()) {
    char c = Serial.read();

    // Обработка Backspace (\b)
    if (c == '\b' || c == 127) {
      if (command.length() > 0) {
        command.remove(command.length() - 1);
        Serial.write('\b');
        Serial.write(' ');
        Serial.write('\b');
      }
    }
    // Обработка Enter (\r или \n)
    else if (c == '\r' || c == '\n') {
      Serial.println();
      processCommand();
      command = "";
      writeUser();
    } else if (c == 37 || c == 39) {

    } else if (isPrintable(c)) {
      command += c;
      Serial.write(c);
    }
  }
  if (Serial.available() == 0) {
    delay(30); 
  }
}

void processCommand() {
  command.trim();

  if (command.isEmpty()) {
    // Serial.println("Empty command.");
    return;
  }

  // Разделение команды на имя и аргументы
  int spaceIndex = command.indexOf(' ');
  String cmdName = (spaceIndex == -1) ? command : command.substring(0, spaceIndex);

  for (const auto& cmd : commandList) {
    if (cmdName == cmd.name) {
      cmd.handler(command);
      return;
    }
  }

  Serial.println("Unknown command. Type \"help\" for more information.");
}