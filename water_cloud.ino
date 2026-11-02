// ESP32 Water Quality Monitor - cloud version
// Every 15 seconds the ESP32 reads pH, TDS, turbidity, temperature and the 5 filter
// statuses and uploads them to a Supabase database over HTTPS.
// The web dashboard (index.html) reads them from there, so it works from anywhere.
// No extra libraries needed for this file.

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// ====== 1. EDIT THESE ======
const char* WIFI_SSID    = "YOUR_WIFI_NAME";
const char* WIFI_PASS    = "YOUR_WIFI_PASSWORD";
const char* SUPABASE_URL = "https://YOUR-PROJECT-ID.supabase.co";   // no slash at the end
const char* SUPABASE_KEY = "YOUR-ANON-OR-PUBLISHABLE-KEY";
const unsigned long SEND_EVERY_MS = 15000;   // keep the same as SEND_EVERY_SECONDS in index.html (15)
// ===========================

// ====== 2. SENSOR READINGS ======
// These make fake values so you can test the whole system without sensors.
// Later replace each function body with your real sensor code (see the beginner guide, Part 6).
float readPH()          { return 7.0 + 0.3 * sin(millis() / 7000.0); }
float readTDS()         { return 250 + 20 * sin(millis() / 9000.0); }                        // ppm
float readTurbidity()   { return 0.6 + 0.3 * sin(millis() / 6000.0) + random(0, 3) / 10.0; } // NTU
float readTemperature() { return 26.0 + 1.5 * sin(millis() / 11000.0); }                     // deg C

// ====== 3. FILTER STATUS (filter number 0..4) ======
// Return short text such as "Normal", "Clogged", "Low flow", "Replace soon", "Offline".
// The dashboard colours it: Normal = green, clog/offline/fail = red, anything else = amber.
// Fake behaviour for testing: filter 3 clogs and filter 5 shows low flow now and then.
String readFilterStatus(int i) {
  unsigned long phase = (millis() / 20000) % 6;
  if (i == 2 && phase >= 4) return "Clogged";
  if (i == 4 && phase == 5) return "Low flow";
  return "Normal";
}
// ==================================

// Turns a number into JSON text. A bad reading (NaN, or -127 from a missing DS18B20) becomes null.
String num(float v, int decimals) {
  if (isnan(v) || isinf(v) || v < -100) return "null";
  return String(v, decimals);
}

bool sendReading() {
  if (WiFi.status() != WL_CONNECTED) return false;

  String body = "{";
  body += "\"ph\":"          + num(readPH(), 2)          + ",";
  body += "\"tds\":"         + num(readTDS(), 0)         + ",";
  body += "\"turbidity\":"   + num(readTurbidity(), 2)   + ",";
  body += "\"temperature\":" + num(readTemperature(), 1) + ",";
  body += "\"filters\":[";
  for (int i = 0; i < 5; i++) {
    if (i) body += ",";
    body += "\"" + readFilterStatus(i) + "\"";
  }
  body += "]}";

  WiFiClientSecure client;
  client.setInsecure();            // skips certificate checking: fine for a college project
  HTTPClient http;
  String url = String(SUPABASE_URL) + "/rest/v1/readings";
  if (!http.begin(client, url)) return false;

  http.addHeader("Content-Type", "application/json");
  http.addHeader("apikey", SUPABASE_KEY);
  if (String(SUPABASE_KEY).startsWith("eyJ")) {      // the older "anon" key is a JWT and also goes here
    http.addHeader("Authorization", String("Bearer ") + SUPABASE_KEY);
  }
  http.addHeader("Prefer", "return=minimal");

  int code = http.POST(body);
  Serial.print("Upload -> HTTP ");
  Serial.print(code);
  Serial.print("  ");
  Serial.println(body);
  if (code != 201) {
    Serial.println(http.getString());              // shows the reason if it failed
  }
  http.end();
  return code == 201;                              // 201 = row saved
}

unsigned long lastSend = 0;
bool firstSend = true;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected. IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi lost, reconnecting...");
    WiFi.reconnect();
    delay(3000);
    return;
  }
  if (firstSend || millis() - lastSend >= SEND_EVERY_MS) {
    firstSend = false;
    lastSend = millis();
    sendReading();
  }
}
