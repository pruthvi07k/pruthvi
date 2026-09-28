#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// ===============================
// PRUTHVI Wi-Fi Configuration
// ===============================

const char* WIFI_SSID = "Yash's vivo Y11 5G";
const char* WIFI_PASSWORD = "8261865488";

// ===============================
// Pin Configuration
// ===============================

const int LED_PIN    = D1;
const int RELAY1_PIN = D2;
const int RELAY2_PIN = D5;

// LED is active HIGH
const int LED_ON  = HIGH;
const int LED_OFF = LOW;

// Relay module is active LOW
const int RELAY_ON  = LOW;
const int RELAY_OFF = HIGH;

// ===============================
// Web Server
// ===============================

ESP8266WebServer server(80);

// ===============================
// Device Functions
// ===============================

void ledOn() {
  digitalWrite(LED_PIN, LED_ON);
}

void ledOff() {
  digitalWrite(LED_PIN, LED_OFF);
}

void relay1On() {
  digitalWrite(RELAY1_PIN, RELAY_ON);
}

void relay1Off() {
  digitalWrite(RELAY1_PIN, RELAY_OFF);
}

void relay2On() {
  digitalWrite(RELAY2_PIN, RELAY_ON);
}

void relay2Off() {
  digitalWrite(RELAY2_PIN, RELAY_OFF);
}

// ===============================
// CORS
// ===============================

void addCORS() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

// ===============================
// Status API
// ===============================

void handleStatus() {

  addCORS();

  String ledState =
      digitalRead(LED_PIN) == LED_ON ? "ON" : "OFF";

  String relay1State =
      digitalRead(RELAY1_PIN) == RELAY_ON ? "ON" : "OFF";

  String relay2State =
      digitalRead(RELAY2_PIN) == RELAY_ON ? "ON" : "OFF";

  String json = "{";
  json += "\"led\":\"" + ledState + "\",";
  json += "\"bulb1\":\"" + relay1State + "\",";
  json += "\"bulb2\":\"" + relay2State + "\",";
  json += "\"wifi\":true,";
  json += "\"ip\":\"" + WiFi.localIP().toString() + "\"";
  json += "}";

  server.send(200, "application/json", json);
}

// ===============================
// Device Control API
// ===============================

void handleDevice() {

  addCORS();

  String device = server.arg("device");
  String state  = server.arg("state");

  device.toLowerCase();
  state.toUpperCase();

  Serial.println();
  Serial.println("========== DEVICE COMMAND ==========");
  Serial.print("Device: ");
  Serial.println(device);
  Serial.print("State : ");
  Serial.println(state);
  Serial.println("====================================");

  // LED
  if (device == "led") {

    if (state == "ON") {
      ledOn();
      Serial.println("LED -> ON");
    }
    else if (state == "OFF") {
      ledOff();
      Serial.println("LED -> OFF");
    }
    else {
      server.send(400, "application/json",
                  "{\"error\":\"Invalid LED state\"}");
      return;
    }
  }

  // Relay 1
  else if (device == "bulb1" || device == "relay1") {

    if (state == "ON") {
      relay1On();
      Serial.println("RELAY1 -> ON");
    }
    else if (state == "OFF") {
      relay1Off();
      Serial.println("RELAY1 -> OFF");
    }
    else {
      server.send(400, "application/json",
                  "{\"error\":\"Invalid relay1 state\"}");
      return;
    }
  }

  // Relay 2
  else if (device == "bulb2" || device == "relay2") {

    if (state == "ON") {
      relay2On();
      Serial.println("RELAY2 -> ON");
    }
    else if (state == "OFF") {
      relay2Off();
      Serial.println("RELAY2 -> OFF");
    }
    else {
      server.send(400, "application/json",
                  "{\"error\":\"Invalid relay2 state\"}");
      return;
    }
  }

  else {

    server.send(
      400,
      "application/json",
      "{\"error\":\"Unknown device\"}"
    );

    return;
  }

  server.send(
    200,
    "application/json",
    "{\"success\":true}"
  );
}

// ===============================
// OPTIONS / CORS
// ===============================

void handleOptions() {

  addCORS();

  server.send(
    204,
    "text/plain",
    ""
  );
}

// ===============================
// Root Page
// ===============================

void handleRoot() {

  addCORS();

  String html;

  html += "<!DOCTYPE html>";
  html += "<html>";
  html += "<head>";
  html += "<title>PRUTHVI ESP8266</title>";
  html += "</head>";

  html += "<body>";
  html += "<h1>PRUTHVI ESP8266 Wi-Fi Controller</h1>";

  html += "<p>Status: ONLINE</p>";

  html += "<p>IP Address: ";
  html += WiFi.localIP().toString();
  html += "</p>";

  html += "<p>LED: D1</p>";
  html += "<p>Relay 1: D2</p>";
  html += "<p>Relay 2: D5</p>";

  html += "</body>";
  html += "</html>";

  server.send(
    200,
    "text/html",
    html
  );
}

// ===============================
// Wi-Fi Connection
// ===============================

void connectWiFi() {

  Serial.println();
  Serial.println("====================================");
  Serial.println("       PRUTHVI Wi-Fi START");
  Serial.println("====================================");

  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);

  WiFi.begin(
    WIFI_SSID,
    WIFI_PASSWORD
  );

  int attempts = 0;

  while (WiFi.status() != WL_CONNECTED &&
         attempts < 60) {

    delay(500);

    Serial.print(".");

    attempts++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {

    Serial.println();
    Serial.println("WiFi connected!");

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    Serial.print("MAC Address: ");
    Serial.println(WiFi.macAddress());

    Serial.print("Signal RSSI: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");

  }
  else {

    Serial.println();
    Serial.println("ERROR: WiFi connection failed!");

    Serial.print("WiFi status: ");
    Serial.println(WiFi.status());
  }
}

// ===============================
// HTTP Server
// ===============================

void startServer() {

  server.on(
    "/",
    HTTP_GET,
    handleRoot
  );

  server.on(
    "/api/status",
    HTTP_GET,
    handleStatus
  );

  server.on(
    "/api/device",
    HTTP_POST,
    handleDevice
  );

  server.on(
    "/api/device",
    HTTP_OPTIONS,
    handleOptions
  );

  server.onNotFound([]() {

    addCORS();

    server.send(
      404,
      "application/json",
      "{\"error\":\"Not found\"}"
    );

  });

  server.begin();

  Serial.println();
  Serial.println("====================================");
  Serial.println("   PRUTHVI HTTP SERVER READY");
  Serial.println("====================================");

  Serial.print("Server: http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

  Serial.println();
  Serial.println("API:");
  Serial.println("GET  /api/status");
  Serial.println("POST /api/device");
}

// ===============================
// SETUP
// ===============================

void setup() {

  // Start serial FIRST
  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println();
  Serial.println("####################################");
  Serial.println("#          PRUTHVI BOOT            #");
  Serial.println("####################################");

  // Reset diagnostic
  Serial.print("Reset reason: ");
  Serial.println(ESP.getResetReason());

  Serial.print("Reset info: ");
  Serial.println(ESP.getResetInfo());

  Serial.print("Free heap: ");
  Serial.println(ESP.getFreeHeap());

  Serial.println();

  // Enable ESP8266 debug messages
  Serial.setDebugOutput(true);

  // Configure GPIO
  pinMode(
    LED_PIN,
    OUTPUT
  );

  pinMode(
    RELAY1_PIN,
    OUTPUT
  );

  pinMode(
    RELAY2_PIN,
    OUTPUT
  );

  // Safe initial state
  ledOff();
  relay1Off();
  relay2Off();

  Serial.println("GPIO initialized.");

  Serial.println("LED    -> D1");
  Serial.println("RELAY1 -> D2");
  Serial.println("RELAY2 -> D5");

  Serial.println();

  // Connect Wi-Fi
  connectWiFi();

  // Start HTTP server only if Wi-Fi connected
  if (WiFi.status() == WL_CONNECTED) {

    startServer();

  }
  else {

    Serial.println();
    Serial.println("HTTP SERVER NOT STARTED");
    Serial.println("WiFi is not connected.");

  }

  Serial.println();
  Serial.println("========== SETUP COMPLETE ==========");
}

// ===============================
// LOOP
// ===============================

void loop() {

  if (WiFi.status() == WL_CONNECTED) {

    server.handleClient();

  }
  else {

    // Try reconnecting if Wi-Fi drops
    static unsigned long lastReconnect = 0;

    if (millis() - lastReconnect > 10000) {

      lastReconnect = millis();

      Serial.println();
      Serial.println("WiFi disconnected.");
      Serial.println("Trying to reconnect...");

      WiFi.disconnect();
      WiFi.begin(
        WIFI_SSID,
        WIFI_PASSWORD
      );
    }
  }

  yield();
}
