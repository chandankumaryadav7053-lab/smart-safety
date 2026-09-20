#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// ---------- WiFi ----------
const char* ssid = "Sarfaraj";
const char* password = "12345678";

// ---------- Telegram ----------
#define BOT_TOKEN "8902286321:AAEOst-3D9ifblsD1AsCyryYwk-cIi38onU"
#define CHAT_ID "6960024511"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// ---------- Pins ----------
#define RAIN_SENSOR 27
#define LED_PIN 26
#define BUZZER_PIN 25

bool rainState = false;

void setup() {
  Serial.begin(115200);

  pinMode(RAIN_SENSOR, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  WiFi.begin(ssid, password);

  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected!");

  client.setInsecure();

  bot.sendMessage(CHAT_ID, "🌧️ Rain Detector Started", "");
}

void loop() {

  int sensorValue = digitalRead(RAIN_SENSOR);

  // LOW = Rain detected
  if (sensorValue == LOW && rainState == false) {

    rainState = true;

    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);

    Serial.println("🌧️ RAIN DETECTED!");

    bot.sendMessage(
      CHAT_ID,
      "🌧️ ALERT!\nRain has been detected.",
      ""
    );

    delay(1000);
  }

  // HIGH = No rain
  if (sensorValue == HIGH && rainState == true) {

    rainState = false;

    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);

    Serial.println("☀️ NO RAIN");

    bot.sendMessage(
      CHAT_ID,
      "☀️ Rain stopped.\nNo rain detected now.",
      ""
    );

    delay(1000);
  }

  delay(500);
}

