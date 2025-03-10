/* #include <WiFi.h>
#include <ArduinoOTA.h>

// Настройки Wi-Fi
const char* ssid = "netis";       // Имя вашей Wi-Fi сети
const char* password = "password"; // Пароль вашей Wi-Fi сети

// Флаг для отслеживания состояния подключения
bool wifiConnected = false;

// Задача для подключения к Wi-Fi
void connectToWiFi(void *parameter) {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  wifiConnected = true; // Устанавливаем флаг подключения

  // После подключения настраиваем OTA
  ArduinoOTA.begin();
  Serial.println("OTA update ready.");

  // Бесконечный цикл для обработки OTA-обновлений
  while (true) {
    ArduinoOTA.handle(); // Обрабатываем OTA-запросы
    delay(10);           // Небольшая задержка для снижения нагрузки
  }
} */