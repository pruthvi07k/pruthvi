// ==========================================
// PRUTHVI HOME AUTOMATION
// ESP8266 GPIO CONTROL ARCHITECTURE
// ==========================================

// GPIO mapping
const int LED_PIN    = D1;   // GPIO5
const int RELAY1_PIN = D2;   // GPIO4
const int RELAY2_PIN = D5;   // GPIO14

// For now we use HIGH = ON.
// Relay modules may require LOW = ON.
// We will adjust after identifying the module.
const int DEVICE_ON  = LOW;
const int DEVICE_OFF = HIGH;


// ------------------------------------------
// Device control functions
// ------------------------------------------

void ledOn()
{
  digitalWrite(LED_PIN, DEVICE_ON);
}

void ledOff()
{
  digitalWrite(LED_PIN, DEVICE_OFF);
}

void relay1On()
{
  digitalWrite(RELAY1_PIN, DEVICE_ON);
}

void relay1Off()
{
  digitalWrite(RELAY1_PIN, DEVICE_OFF);
}

void relay2On()
{
  digitalWrite(RELAY2_PIN, DEVICE_ON);
}

void relay2Off()
{
  digitalWrite(RELAY2_PIN, DEVICE_OFF);
}


// ------------------------------------------
// Setup
// ------------------------------------------

void setup()
{
  // Configure GPIO pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);

  // Safe startup state
  ledOff();
  relay1Off();
  relay2Off();

  Serial.begin(115200);

  delay(1000);

  Serial.println("PRUTHVI READY");
  Serial.println("GPIO ARCHITECTURE READY");
}


// ------------------------------------------
// Serial command processing
// ------------------------------------------

void loop()
{
  if (Serial.available())
  {
    String command = Serial.readStringUntil('\n');

    command.trim();

    // LED
    if (command == "LED_ON")
    {
      ledOn();
      Serial.println("LED_ON RECEIVED");
    }
    else if (command == "LED_OFF")
    {
      ledOff();
      Serial.println("LED_OFF RECEIVED");
    }

    // Relay 1
    else if (command == "RELAY1_ON")
    {
      relay1On();
      Serial.println("RELAY1_ON RECEIVED");
    }
    else if (command == "RELAY1_OFF")
    {
      relay1Off();
      Serial.println("RELAY1_OFF RECEIVED");
    }

    // Relay 2
    else if (command == "RELAY2_ON")
    {
      relay2On();
      Serial.println("RELAY2_ON RECEIVED");
    }
    else if (command == "RELAY2_OFF")
    {
      relay2Off();
      Serial.println("RELAY2_OFF RECEIVED");
    }

    // Unknown command
    else
    {
      Serial.println("UNKNOWN COMMAND");
    }
  }
}
