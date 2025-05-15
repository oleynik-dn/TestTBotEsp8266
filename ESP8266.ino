#include <ESP8266WiFi.h>           // Подключение библиотеки для работы с Wi-Fi
#include <ESP8266WebServer.h>      // Библиотека для создания веб-сервера на ESP8266
#include <WiFiClientSecure.h>      // Библиотека для HTTPS-клиента

// Данные для подключения к Wi-Fi
const char* WIFI_SSID = "название_сети";
const char* WIFI_PASSWORD = "пароль_от_сети";

// Данные Telegram-бота
const char* BOT_TOKEN = "токен_вашего_бота"; // Токен бота
const char* CHAT_ID = "ид_чата_или_пользователя_телеграм"; // ID чата, куда будут отправляться сообщения

ESP8266WebServer server(80); // Создание веб-сервера на 80 порту
const int LED_PIN = LED_BUILTIN; // Встроенный светодиод платы (обычно D0)

// Функция setup() вызывается один раз при старте устройства
void setup() {
  Serial.begin(115200); // Запуск последовательного монитора на скорости 115200 бод
  pinMode(LED_PIN, OUTPUT); // Установка пина светодиода в режим вывода
  digitalWrite(LED_PIN, HIGH); // Выключение светодиода по умолчанию (активный LOW)

  // Подключение к Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Подключение к Wi-Fi");

  // Ожидание подключения
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Подключение успешно
  Serial.println("\nWiFi подключен. IP адрес: ");
  Serial.println(WiFi.localIP());

  // Обработка HTTP-запросов
  server.on("/on", handleOn);   // Обработка запроса для включения светодиода
  server.on("/off", handleOff); // Обработка запроса для выключения светодиода
  server.begin();               // Запуск HTTP-сервера
  Serial.println("HTTP сервер запущен");
}

// Главный цикл программы — постоянная обработка входящих запросов
void loop() {
  server.handleClient(); // Обработка HTTP-запросов от клиентов
}

// Функция обработки запроса /on (включить свет)
void handleOn() {
  server.send(200, "text/plain", "LED ON");  // Отправка ответа клиенту
  digitalWrite(LED_PIN, LOW); // Включение светодиода (LOW — активный уровень)
  sendTelegramMessage("Плата: ✅ Свет включён"); // Отправка уведомления в Telegram
}

// Функция обработки запроса /off (выключить свет)
void handleOff() {
  server.send(200, "text/plain", "LED OFF"); // Ответ клиенту
  digitalWrite(LED_PIN, HIGH); // Выключение светодиода
  sendTelegramMessage("Плата: ❌ Свет выключен"); // Уведомление в Telegram
}

// Функция отправки сообщения в Telegram
void sendTelegramMessage(String message) {
  WiFiClientSecure client;
  client.setInsecure(); // Отключение проверки SSL-сертификатов

  // Составление URL и POST-данных для Telegram API
  String url = "/bot" + String(BOT_TOKEN) + "/sendMessage";
  String data = "chat_id=" + String(CHAT_ID) + "&text=" + urlencode(message);

  // Подключение к Telegram API
  if (!client.connect("api.telegram.org", 443)) {
    Serial.println("Ошибка подключения к Telegram API");
    return;
  }

  // Отправка POST-запроса
  client.println("POST " + url + " HTTP/1.1");
  client.println("Host: api.telegram.org");
  client.println("Content-Type: application/x-www-form-urlencoded");
  client.print("Content-Length: ");
  client.println(data.length());
  client.println();
  client.print(data);

  // Чтение заголовков ответа
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") break; // Конец заголовков
  }

  // Вывод ответа Telegram API
  String response = client.readString();
  Serial.println("Ответ от Telegram:");
  Serial.println(response);
}

// Функция кодирования строки в формат URL (для корректной передачи в запросе)
String urlencode(String str) {
  String encoded = "";
  char c;
  char code0, code1;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (isalnum(c)) {
      encoded += c; // Буквы и цифры передаём как есть
    } else {
      code0 = (c >> 4) & 0xF; // Старшие 4 бита
      code1 = c & 0xF;        // Младшие 4 бита
      encoded += '%';
      encoded += "0123456789ABCDEF"[code0];
      encoded += "0123456789ABCDEF"[code1];
    }
  }
  return encoded;
}
