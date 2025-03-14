#include "system.h"
#include <stdio.h>

bool isMounted = false;
String username = "root"; // Имя пользователя
String machineName = "microos"; // Имя системы
String autoMountSD = "0";
String autoMountPin = "5";
String lastMountedPin = "-1";

int terminalRows = 0;
int terminalCols = 0;

const char* configFilePath = "/sys/config.init";
const char* wifiKnownFilePath = "/sys/wifi.known";
const char* wifiConfigFilePath = "/sys/wifi.init";

bool isRoot = true;
String command = "";

std::map<String, String*> configMap = {
    {"username", &username},
    {"machineName", &machineName},
    {"autoMountSD", &autoMountSD},
    {"autoMountPin", &autoMountPin},
    {"lastMountedPin", &lastMountedPin}
};

const int PWM_FREQUENCY = 5000;
const int PWM_RESOLUTION = 8;
const int MAX_PWM_CHANNELS = 16;

bool pwmChannelsUsed[MAX_PWM_CHANNELS] = {false};
int pinToChannelMap[40] = {-1};

int findFreePWMChannel() {
    for (int i = 0; i < MAX_PWM_CHANNELS; i++) {
        if (!pwmChannelsUsed[i]) {
            pwmChannelsUsed[i] = true;
            return i;
        }
    }
    return -1;
}

String progressBar(int used, int total, int width = 10) {
    float percentage = (float)used / total * 100;
    int filledWidth = (int)(percentage / 100 * width);

    String bar = "[";
    for (int i = 0; i < width; i++) {
        if (i < filledWidth) {
            bar += "=";
        } else {
            bar += " ";
        }
    }
    bar += "]";

    return bar + " " + String(percentage) + "%";
}

void helpFunc() {
    printf("List of known commands:\r\n");
    
    size_t maxCommandLength = 0;
    for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); ++i) {
        if (strlen(commands[i].name) > maxCommandLength) {
            maxCommandLength = strlen(commands[i].name);
        }
    }

    for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); ++i) {
        printf(" %-*s  -  %s\r\n", maxCommandLength, commands[i].name, commands[i].description);
    }
}

// Функция для монтирования SD-карты
bool mount(String command) {

    int pin;
    int spc = command.indexOf(' ');
    if (spc != -1) {
        pin = command.substring(spc + 1).toInt();
    } else {
        Serial.println("Error: No pin specified for \"mount\" command");
        return false;
    }
    
    if (isMounted) {
        Serial.println("SD card is already mounted");
        return true;
    }

    if (!SD.begin(pin)) {
        Serial.println("Failed to mount SD card");
        return false;
    }

    Serial.printf("SD card successfully mounted on pin %d\r\n", pin);
    isMounted = true;
    return true;
}

bool unmount() {
    if (!isMounted) {
        Serial.println("SD card is not mounted");
        return false;
    } else {
        SD.end();
        isMounted = false;
        Serial.println("SD card unmounted");
        return true;
    }
}

bool saveConfig() {
    File configFile = LittleFS.open(configFilePath, "w");
    if (!configFile) {
        Serial.println("Error while opening settings file");
        return false;
    }

    for (const auto& entry : configMap) {
        configFile.println(entry.first + "=" + *(entry.second));
    }

    configFile.close();
    Serial.println("Settings configuration saved successfully");
    return true;
}

void updateConfig(const String& key, const String& value) {
    if (configMap.find(key) != configMap.end()) {
        *(configMap[key]) = value;

        saveConfig();
    } else {
        Serial.println("Unknown key: " + key);
    }
}

String getConfig(const String& key) {
    if (configMap.find(key) != configMap.end()) {
        return *(configMap[key]);
    } else {
        return "Unknown key: " + key;
    }
}

void processSettingsCommand(const String& command) {
    int firstSpace = command.indexOf(' ');
    int secondSpace = command.indexOf(' ', firstSpace + 1);
    int thirdSpace = command.indexOf(' ', secondSpace + 1);

    if (firstSpace == -1) {
        Serial.println("Invalid command format. Use 'settings <set/get/list/keys> <key> [value]'");
        return;
    }

    String action = command.substring(firstSpace + 1, secondSpace == -1 ? command.length() : secondSpace);

    if (action == "set") {
        if (secondSpace == -1 || thirdSpace == -1) {
            Serial.println("Missing key or value for 'set' command");
            return;
        }
        String key = command.substring(secondSpace + 1, thirdSpace);
        String value = command.substring(thirdSpace + 1);
        updateConfig(key, value);
        Serial.println("Updated key '" + key + "' with value '" + value + "'");
    } else if (action == "get") {
        if (secondSpace == -1) {
            Serial.println("Missing key for 'get' command");
            return;
        }
        String key = command.substring(secondSpace + 1);
        String value = getConfig(key);
        Serial.println("Key '" + key + "' has value: " + value);
    } else if (action == "list") {
        listAllSettings();
    } else if (action == "keys") {
        listAllKeys();
    } else {
        Serial.println("Unknown action: " + action);
    }
}

void listAllSettings() {
    Serial.println("Current settings:");
    for (const auto& entry : configMap) {
        Serial.println(entry.first + ": " + *(entry.second));
    }
}

void listAllKeys() {
    Serial.println("Available keys:");
    for (const auto& entry : configMap) {
        Serial.println(entry.first);
    }
}

const char* microOScharset[] = {
    " @@@@@@               @@@@@@              ",
    " @@@@@@@             @@@@@@@              ",
    " @@@@@@@@           @@@@@@@@              ",
    " @@@@@@@@@         @@@@@@@@@              ",
    " @@@@@@@@@@       @@@@@@@@@@              ",
    " @@@@@@@@@@@     @@@@@@@@@@@              ",
    " @@@@@%-@@@@@   @@@@@@ @@@@@              ",
    " @@@@@% %@@@@@ -@@@@@  @@@@@              ",
    " @@@@@%  @@@@@+@@@@@   @@@@@              ",
    " @@@@@#   @@@@@@@@@    @@@@@              ",
    " @@@@@*    @@@@@@@     @@@@@              ",
    " @@@@@*     @@@@@      @@@@@              ",
    " @@@@@*                @@@@@              ",
    " @@@@@*                @@@@@              ",
    "                                          ",
    "                     =@@@@@       @@@@@%  ",
    "                   @@@@@@@@@@   @@@@@@@@@ ",
    "                  @@@@    #@@@  @@@@:     ",
    "                  @@@@     @@@   @@@@@@@@ ",
    "                  @@@@    +@@@        @@@@",
    "                   @@@@@@@@@@   @@@@%@@@@%",
    "                     @@@@@@      @@@@@@@  ",
    "                                          "
};

const int microOScharsetWidth = 43;

void neofetch() {

    int ramUsed = ESP.getHeapSize() - ESP.getFreeHeap();
    int ramTotal = ESP.getHeapSize();
    int flashUsed = ESP.getSketchSize();
    int flashTotal = ESP.getFlashChipSize();

    String info[] = {
        "Chip Model:      " + String(ESP.getChipModel()),
        "Chip Cores:      " + String(ESP.getChipCores()),
        "Chip Revision:   " + String(ESP.getChipRevision()),
        "CPU Frequency:   " + String(ESP.getCpuFreqMHz()) + " MHz",
        "Flash Size:      " + String(ESP.getFlashChipSize() / (1024 * 1024)) + " MB",
        "SDK Version:     " + String(ESP.getSdkVersion()),
        "Free Heap:       " + String(ESP.getFreeHeap() / 1024) + " KB", // Свободная память кучи
        "Sketch Size:     " + String(ESP.getSketchSize() / 1024) + " KB", // Размер PSRAM
        "MAC Address:     " + WiFi.macAddress(), // MAC-адрес устройства
        "Reset Reason:    " + String(esp_reset_reason()), // Причина последнего сброса
        "RAM Usage:       " + progressBar(ramUsed, ramTotal),
        "Flash Usage:     " + progressBar(flashUsed, flashTotal)
    };

    for (int i = 0; i < sizeof(microOScharset) / sizeof(microOScharset[0]); i++) {
        Serial.print("\033[31m");
        Serial.print(microOScharset[i]);
        Serial.print("\033[0m");
        int padding = microOScharsetWidth - strlen(microOScharset[i]);
        
        for (int j = 0; j < padding; j++) {
            Serial.print(" ");
        }

        if (i >= 1 && i - 1 < sizeof(info) / sizeof(info[0])) {
            Serial.print(info[i - 1]);
        }

        Serial.println();
    }

}

void getTerminal() {
    Serial.print("\033[18t");
    Serial.println("Checking terminal size");
    String response = Serial.readString();
    if (response.startsWith("\033[8;")) {
        response.remove(0, 4);

        // Разделение строки на части
        int semicolonIndex = response.indexOf(';');
        int terminalRows = response.substring(0, semicolonIndex).toInt();
        response.remove(0, semicolonIndex + 1);
        int tIndex = response.indexOf('t');
        int terminalCols = response.substring(0, tIndex).toInt();

        Serial.print("Size: ");
        Serial.print(terminalRows);
        Serial.print(" rows x ");
        Serial.print(terminalCols);
        Serial.println(" columns");

    } else {
        Serial.println("Incorrect response from terminal");
    }
}

void pinWorker(const String& command) {
    // pin <num> mode/set [analog] <mode:[OUTPUT/INPUT/PULLUP/PULLDN]> <set:[digital:low/high/0/1 analog:0-255]>
    // ex: pin 20 mode output
    // ex: pin 20 set analog 200
    // ex: pin 20 set 1
    String comm = command.substring(4);
    int firstSpace = comm.indexOf(' ');
    if (firstSpace == -1) {
        Serial.println("A \"pin\" command is incorrect\r\nUse: pin <num> mode/set [analog] <mode:[OUTPUT/INPUT/PULLUP/PULLDN]> <set:[digital:low/high/0/1 analog:0-255]>");
        return;
    } // Проверка на корректность команды

    String strpin = comm.substring(0, firstSpace);
    strpin.trim();
    int pin = strpin.toInt();
    String cmd = comm.substring(firstSpace + 1);

    if (cmd.isEmpty()) {
        Serial.println("Empty command after pin number.");
        return;
    }

    if (cmd.startsWith("mode")) {
        // Установка режима пина
        int modeIndex = cmd.indexOf(' ', 4); // Индекс после "mode"
        if (modeIndex == -1) {
            Serial.println("Invalid 'mode' command format.");
            Serial.println(cmd);
            return;
        }

        String modeStr = cmd.substring(modeIndex + 1);
        modeStr.toLowerCase();

        Serial.println("Setting pin " + String(pin) + " to mode " + modeStr);

        int i = 0;
        if (modeStr == "output") {
            pinMode(pin, OUTPUT);
            i = 1;
        } else if (modeStr == "input") {
            pinMode(pin, INPUT);
            i = 1;
        } else if (modeStr == "pullup") {
            pinMode(pin, INPUT_PULLUP);
            i = 1;
        } else if (modeStr == "pulldn") {
            pinMode(pin, INPUT_PULLDOWN);
            i = 1;
        }
        
        if (i == 1) {
            modeStr.toUpperCase();
            Serial.println("Pin " + String(pin) + " in " + modeStr + " mode now");
        }

    } else if (cmd.startsWith("set")) {
        // Установка значения пина
        String valueStr = cmd.substring(cmd.lastIndexOf(' ') + 1);
        if (cmd.indexOf("analog") != -1) {
            // Аналоговый выход
            int analogValue = valueStr.toInt();
            analogWrite(pin, analogValue);
        } else {
            // Цифровое значение
            int digitalValue = (valueStr == "high" || valueStr == "1") ? HIGH : LOW;
            digitalWrite(pin, digitalValue);
        }
    } else if (cmd.startsWith("pwm")) {
        int pwmValue = cmd.substring(cmd.indexOf(' ') + 1).toInt();
        if (pwmValue < 0 || pwmValue > ((1 << PWM_RESOLUTION) - 1)) {
            Serial.println("PWM value out of range (0-" + String((1 << PWM_RESOLUTION) - 1) + ").");
            return;
        }

        int channel = pinToChannelMap[pin];
        if (channel == -1) {
            channel = findFreePWMChannel();
            if (channel == -1) {
                Serial.println("No free PWM channels available.");
                return;
            }

            ledcSetup(channel, PWM_FREQUENCY, PWM_RESOLUTION);
            ledcAttachPin(pin, channel);
            pinToChannelMap[pin] = channel;

            Serial.println("PWM initialized on pin " + String(pin) + " (channel " + String(channel) + ")");
        }

        if (channel == -1 || !pwmChannelsUsed[channel]) {
            Serial.println("PWM channel is not initialized for pin " + String(pin));
            return;
        }

        ledcWrite(channel, pwmValue);
        Serial.println("PWM value " + String(pwmValue) + " set on pin " + String(pin) + " (channel " + String(channel) + ")");
    } else {
        Serial.println("Unknown command: " + cmd);
    }
}