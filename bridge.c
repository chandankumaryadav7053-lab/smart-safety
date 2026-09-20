#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ================= WIFI =================

const char* ssid = "Sarfaraj";
const char* password = "12345678";

// =================================================
// TELEGRAM BOT 1 = BRIDGE
// =================================================

String bridgeBotToken = "8798351511:AAFDktHCCzt2OWMGuF0GjYYeF-RCmCoE47U";
String bridgeChatID = "6960024511";

// =================================================
// TELEGRAM BOT 2 = WATER
// =================================================

String waterBotToken = "8801088599:AAF3sDK7sE6IXrHGpLtaFj5iZT13Bi4AU70";
String waterChatID = "6960024511";

// ================= PINS =================

#define WATER_PIN 34

#define TRIG_PIN 5
#define ECHO_PIN 18

#define BUZZER_PIN 19

// ================= LCD =================

LiquidCrystal_I2C lcd(0x27, 16, 2);

// ================= ALERT SETTINGS =================

int WATER_ALERT = 400;

float BRIDGE_ALERT = 5.0;

// Notification control

bool waterNotificationSent = false;

bool bridgeNotificationSent = false;

// =================================================
// URL ENCODE
// =================================================

String urlEncode(String text) {

  String encoded = "";

  for (int i = 0; i < text.length(); i++) {

    char c = text.charAt(i);

    if (
      (c >= 'a' && c <= 'z') ||
      (c >= 'A' && c <= 'Z') ||
      (c >= '0' && c <= '9') ||
      c == '-' ||
      c == '_' ||
      c == '.' ||
      c == '~'
    ) {

      encoded += c;
    }

    else if (c == ' ') {

      encoded += "%20";
    }

    else if (c == '\n') {

      encoded += "%0A";
    }

    else {

      encoded += "%";
      
      if (c < 16)
        encoded += "0";

      encoded += String((int)c, HEX);
    }
  }

  return encoded;
}

// =================================================
// TELEGRAM SEND FUNCTION
// =================================================

void sendTelegram(
  String botToken,
  String chatID,
  String message
) {

  if (WiFi.status() != WL_CONNECTED) {

    Serial.println("WiFi not connected!");

    return;
  }

  Serial.println("Sending Telegram...");

  WiFiClientSecure client;

  client.setInsecure();

  HTTPClient https;

  String url =
    "https://api.telegram.org/bot" +
    botToken +
    "/sendMessage";

  if (!https.begin(client, url)) {

    Serial.println("HTTPS begin failed!");

    return;
  }

  https.addHeader(
    "Content-Type",
    "application/x-www-form-urlencoded"
  );

  String data =
    "chat_id=" +
    chatID +
    "&text=" +
    urlEncode(message);

  int httpCode =
    https.POST(data);

  Serial.print("Telegram HTTP Code: ");

  Serial.println(httpCode);

  Serial.println("Telegram Response:");

  Serial.println(
    https.getString()
  );

  https.end();
}

// =================================================
// ULTRASONIC
// =================================================

float getDistance() {

  digitalWrite(TRIG_PIN, LOW);

  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);

  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duration =
    pulseIn(
      ECHO_PIN,
      HIGH,
      30000
    );

  if (duration == 0) {

    return -1;
  }

  float distance =
    duration * 0.0343 / 2;

  if (distance > 400) {

    return -1;
  }

  return distance;
}

// =================================================
// SETUP
// =================================================

void setup() {

  Serial.begin(115200);

  // Pins

  pinMode(WATER_PIN, INPUT);

  pinMode(TRIG_PIN, OUTPUT);

  pinMode(ECHO_PIN, INPUT);

  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(
    BUZZER_PIN,
    LOW
  );

  // LCD

  Wire.begin(21, 22);

  lcd.init();

  lcd.backlight();

  lcd.clear();

  lcd.setCursor(0, 0);

  lcd.print("Water & Bridge");

  lcd.setCursor(0, 1);

  lcd.print("Starting...");

  delay(2000);

  // ================= WIFI =================

  WiFi.begin(
    ssid,
    password
  );

  Serial.print(
    "Connecting WiFi"
  );

  while (
    WiFi.status() != WL_CONNECTED
  ) {

    delay(500);

    Serial.print(".");
  }

  Serial.println();

  Serial.println(
    "WiFi Connected!"
  );

  Serial.print(
    "IP Address: "
  );

  Serial.println(
    WiFi.localIP()
  );

  lcd.clear();

  lcd.setCursor(0, 0);

  lcd.print("WiFi Connected");

  delay(1500);
}

// =================================================
// LOOP
// =================================================

void loop() {

  // ================= WATER =================

  int waterValue =
    analogRead(WATER_PIN);

  int waterPercent =
    map(
      waterValue,
      0,
      600,
      0,
      100
    );

  waterPercent =
    constrain(
      waterPercent,
      0,
      100
    );

  // ================= DISTANCE =================

  float distance =
    getDistance();

  // ================= SERIAL =================

  Serial.print(
    "Water Sensor: "
  );

  Serial.print(
    waterValue
  );

  Serial.print(
    " | Water Level: "
  );

  Serial.print(
    waterPercent
  );

  Serial.print("%");

  Serial.print(
    " | Distance: "
  );

  if (distance < 0) {

    Serial.println(
      "No Echo"
    );

  }

  else {

    Serial.print(
      distance,
      1
    );

    Serial.println(
      " cm"
    );
  }

  // ================= LCD =================

  lcd.clear();

  lcd.setCursor(0, 0);

  lcd.print("Water:");

  lcd.print(
    waterPercent
  );

  lcd.print("%");

  lcd.setCursor(0, 1);

  if (distance < 0) {

    lcd.print(
      "Dist: No Echo"
    );

  }

  else {

    lcd.print("Dist:");

    lcd.print(
      distance,
      1
    );

    lcd.print("cm");
  }

  // ================= ALERT =================

  bool waterAlert =
    waterValue >= WATER_ALERT;

  bool bridgeAlert =
    distance > 0 &&
    distance <= BRIDGE_ALERT;

  // ================= BUZZER =================

  if (
    waterAlert ||
    bridgeAlert
  ) {

    digitalWrite(
      BUZZER_PIN,
      HIGH
    );

  }

  else {

    digitalWrite(
      BUZZER_PIN,
      LOW
    );
  }

  // =================================================
  // WATER → WATER TELEGRAM BOT
  // =================================================

  if (
    waterAlert &&
    !waterNotificationSent
  ) {

    String message =
      "🚨 WATER LEVEL ALERT!\n\n"
      "Water Level: " +
      String(waterPercent) +
      "%\n"
      "Sensor Value: " +
      String(waterValue) +
      "\n\n"
      "Please check the water level.";

    sendTelegram(
      waterBotToken,
      waterChatID,
      message
    );

    waterNotificationSent = true;
  }

  // Reset water notification

  if (!waterAlert) {

    waterNotificationSent = false;
  }

  // =================================================
  // BRIDGE → BRIDGE TELEGRAM BOT
  // =================================================

  if (
    bridgeAlert &&
    !bridgeNotificationSent
  ) {

    String message =
      "🚨 BRIDGE ALERT!\n\n"
      "Object/Water detected near bridge.\n"
      "Distance: " +
      String(distance, 1) +
      " cm\n\n"
      "Please check the bridge immediately.";

    sendTelegram(
      bridgeBotToken,
      bridgeChatID,
      message
    );

    bridgeNotificationSent = true;
  }

  // Reset bridge notification

  if (
    distance > 7 ||
    distance < 0
  ) {

    bridgeNotificationSent = false;
  }

  delay(1000);
}
