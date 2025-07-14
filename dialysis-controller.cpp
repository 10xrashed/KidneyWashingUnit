// dialysis-controller.cpp
// Kidney Dialysis System Controller for ESP32
// This code manages a kidney dialysis system using an ESP32 microcontroller.
// It includes WiFi connectivity, LCD display, temperature control, flow measurement,
// solution dispensing, and dashboard integration.
// made by 10xRashed [the Only  10x Developer in jordan]
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ===== WiFi Configuration =====
const char* ssid = "WiFi name"; // Replace with your WiFi name
const char* password = "password"; // Replace with your WiFi password

// ===== Dashboard Configuration =====
const char* dashboardIP = "192.168.1.100";  // Replace with your computer's IP
const int dashboardPort = 3000; // Port number (use 80 for direct HTML)
const char* endpoint = "/api/sensor-data";  // API endpoint

// ===== Pin Configuration (ESP32) =====
const int relayPumpPin = 18;       // Pump control
const int relayHeaterPin = 19;     // Heater control
const int solenoidAPin = 21;       // Solution A control
const int solenoidBPin = 22;       // Solution B control
const int flowSensorPin = 2;       // Flowmeter (interrupt capable)
const int tempSensorPin = 4;       // DS18B20 (1-Wire)
const int buttonPin = 0;           // Start/stop button
const int buzzerPin = 23;          // Passive buzzer
const int greenLedPin = 25;        // System running
const int redLedPin = 26;          // System error
const int pressureSensorPin = 34;  // Analog pressure sensor
const int conductivityPin = 35;    // Analog conductivity sensor

// ===== LCD Setup =====
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ===== Temperature Sensor =====
OneWire oneWire(tempSensorPin);
DallasTemperature tempSensors(&oneWire);
DeviceAddress tempSensor;

// ===== Flow Measurement =====
volatile unsigned long flowPulseCount = 0;
float flowRate = 0.0;             // mL/min
float totalVolume = 0.0;          // mL
const float pulsesPerML = 7.5;    // Calibration factor

// ===== Solution Control =====
enum Solution {SOLUTION_A, SOLUTION_B, SOLUTION_NONE};
Solution currentSolution = SOLUTION_NONE;
float solutionAVolume = 1000.0;   // Target volume for A (mL)
float solutionBVolume = 500.0;    // Target volume for B (mL)
float dispensedA = 0.0;           // Already dispensed A (mL)
float dispensedB = 0.0;           // Already dispensed B (mL)

// ===== System Variables =====
bool systemRunning = false;
bool heaterEnabled = false;
float currentTemp = 0.0;
float currentPressure = 0.0;      // kPa
float currentConductivity = 0.0;  // µS/cm
const float targetTemp = 37.0;    // 37°C for dialysate
const float tempTolerance = 0.5;  // ±0.5°C

// ===== Timing Variables =====
unsigned long lastDebounceTime = 0;
unsigned long lastUpdateTime = 0;
unsigned long lastSensorRead = 0;
unsigned long lastDashboardSend = 0;
const unsigned long debounceDelay = 50;
const unsigned long updateInterval = 1000;
const unsigned long sensorInterval = 2000;
const unsigned long dashboardInterval = 5000; // Send to dashboard every 5 seconds

// ===== WiFi and Dashboard Variables =====
bool wifiConnected = false;
bool dashboardConnected = false;

// ===== Function Prototypes =====
void connectToWiFi();
void checkWiFiConnection();
void sendToDashboard();
void toggleSystem();
void monitorTemperature();
void readAnalogSensors();
void monitorFlow();
void controlSolutions();
void startSolution(Solution solution);
void stopPump();
void updateDisplay();
void temperatureAlarm();
void pressureAlarm();
void errorState();
String getTimestamp();

// ===== Interrupt Service Routine =====
void IRAM_ATTR flowPulse() {
  static unsigned long lastPulseTime = 0;
  unsigned long currentTime = millis();
  
  // Debounce flow sensor pulses
  if (currentTime - lastPulseTime > 5) {
    flowPulseCount++;
    lastPulseTime = currentTime;
    
    // Update dispensed volumes
    if (currentSolution == SOLUTION_A) {
      dispensedA = flowPulseCount / pulsesPerML;
    } 
    else if (currentSolution == SOLUTION_B) {
      dispensedB = flowPulseCount / pulsesPerML;
    }
  }
}

// ===== Setup Function =====
void setup() {
  Serial.begin(115200);
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Kidney System");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  
  delay(2000); // Delay for LCD to initialize
  
  // Set pin modes
  pinMode(relayPumpPin, OUTPUT);
  pinMode(relayHeaterPin, OUTPUT);
  pinMode(solenoidAPin, OUTPUT);
  pinMode(solenoidBPin, OUTPUT);
  pinMode(flowSensorPin, INPUT_PULLUP);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);
  pinMode(pressureSensorPin, INPUT);
  pinMode(conductivityPin, INPUT);

  // Initialize valves and relays (HIGH = OFF for relay modules)
  digitalWrite(solenoidAPin, HIGH);   // Valves closed
  digitalWrite(solenoidBPin, HIGH);
  digitalWrite(relayPumpPin, HIGH);   // Pump off
  digitalWrite(relayHeaterPin, HIGH); // Heater off
  digitalWrite(redLedPin, HIGH);      // System off
  digitalWrite(greenLedPin, LOW);

  // Initialize temperature sensor
  tempSensors.begin();
  if (!tempSensors.getAddress(tempSensor, 0)) {
    lcd.clear();
    lcd.print("Temp Sensor Error!");
    Serial.println("Temperature sensor not found!");
    errorState();
  }

  // Attach flow sensor interrupt
  attachInterrupt(digitalPinToInterrupt(flowSensorPin), flowPulse, FALLING);

  // Connect to WiFi
  connectToWiFi();
  
  // Startup complete
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready");
  lcd.setCursor(0, 1);
  if (wifiConnected) {
    lcd.print("WiFi: Connected");
  } else {
    lcd.print("WiFi: Offline");
  }
  
  tone(buzzerPin, 800, 200);
  delay(2000);
  
  lcd.clear();
  lcd.print("Press to Start");
}

// ===== Main Loop =====
void loop() {
  static bool buttonPressed = false;
  static unsigned long lastWiFiCheck = 0;
  
  // Check WiFi connection periodically
  if (millis() - lastWiFiCheck >= 30000) { // Every 30 seconds
    checkWiFiConnection();
    lastWiFiCheck = millis();
  }
  
  // Handle button press
  if (digitalRead(buttonPin) == LOW) {
    if (!buttonPressed && (millis() - lastDebounceTime > debounceDelay)) {
      buttonPressed = true;
      toggleSystem();
      lastDebounceTime = millis();
    }
  } else {
    buttonPressed = false;
  }

  if (!systemRunning) return;

  // System operation
  monitorTemperature();
  readAnalogSensors();
  monitorFlow();
  controlSolutions();
  updateDisplay();
  
  // Send data to dashboard
  if (wifiConnected && (millis() - lastDashboardSend >= dashboardInterval)) {
    sendToDashboard();
    lastDashboardSend = millis();
  }

  // Safety checks
  if (currentTemp > 38.0 || currentTemp < 16.0) {
    temperatureAlarm();
  }
  
  if (currentPressure > 350.0) { // High pressure alarm
    pressureAlarm();
  }
}

// ===== WiFi Functions =====
void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println();
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    wifiConnected = false;
    Serial.println();
    Serial.println("Failed to connect to WiFi!");
  }
}

void checkWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    wifiConnected = false;
    Serial.println("WiFi disconnected. Reconnecting...");
    connectToWiFi();
  } else {
    wifiConnected = true;
  }
}

// ===== Timestamp Function =====
String getTimestamp() {
  return String(millis()); // Simple timestamp using millis()
}

void sendToDashboard() {
  if (!wifiConnected) return;
  
  // Create JSON payload matching dashboard expectations
  StaticJsonDocument<300> doc;
  
  // Core sensor data (dashboard expects these exact field names)
  doc["temperature"] = currentTemp;
  doc["flowRate"] = flowRate * 0.001; // Convert mL/min to L/min for dashboard
  doc["pressure"] = currentPressure;
  doc["conductivity"] = currentConductivity;
  
  // Device status
  doc["deviceId"] = "ESP32_KIDNEY_SYSTEM";
  doc["timestamp"] = getTimestamp();
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  // Send HTTP POST request
  HTTPClient http;
  String url = "http://" + String(dashboardIP) + ":" + String(dashboardPort) + endpoint;
  
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  
  int httpResponseCode = http.POST(jsonString);
  
  if (httpResponseCode > 0) {
    dashboardConnected = true;
    String response = http.getString();
    Serial.println("✓ Data sent to dashboard");
    Serial.println("Response: " + response);
    
    // Print sensor values for debugging
    Serial.println("Sensor Values:");
    Serial.println("  Temperature: " + String(currentTemp) + "°C");
    Serial.println("  Flow Rate: " + String(flowRate * 0.001) + " L/min");
    Serial.println("  Pressure: " + String(currentPressure) + " kPa");
    Serial.println("  Conductivity: " + String(currentConductivity) + " µS/cm");
    Serial.println("---");
  } else {
    dashboardConnected = false;
    Serial.println("✗ Dashboard connection failed");
    Serial.println("HTTP Error Code: " + String(httpResponseCode));
    Serial.println("URL: " + url);
  }
  
  http.end();
}

// ===== System Control Functions =====
void toggleSystem() {
  systemRunning = !systemRunning;
  
  if (systemRunning) {
    digitalWrite(redLedPin, LOW);
    digitalWrite(greenLedPin, HIGH);
    lcd.clear();
    lcd.print("System RUNNING");
    tone(buzzerPin, 800, 200);
    
    // Reset dispensed volumes
    dispensedA = 0.0;
    dispensedB = 0.0;
    flowPulseCount = 0;
    totalVolume = 0.0;
    
  } else {
    stopPump();
    digitalWrite(greenLedPin, LOW);
    digitalWrite(redLedPin, HIGH);
    currentSolution = SOLUTION_NONE;
    lcd.clear();
    lcd.print("System STOPPED");
  }
}

void monitorTemperature() {
  static unsigned long lastTempCheck = 0;
  
  if (millis() - lastTempCheck >= 2000) {
    tempSensors.requestTemperatures();
    currentTemp = tempSensors.getTempC(tempSensor);
    
    // Check for invalid temperature readings
    if (currentTemp == DEVICE_DISCONNECTED_C) {
      currentTemp = 0.0;
      Serial.println("Temperature sensor disconnected!");
    }
    
    // Heater control with hysteresis
    if (currentTemp < (targetTemp - tempTolerance) && currentTemp > 0) {
      digitalWrite(relayHeaterPin, LOW);
      heaterEnabled = true;
    } 
    else if (currentTemp > (targetTemp + tempTolerance)) {
      digitalWrite(relayHeaterPin, HIGH);
      heaterEnabled = false;
    }
    
    lastTempCheck = millis();
  }
}

void readAnalogSensors() {
  // Read pressure sensor
  int pressureRaw = analogRead(pressureSensorPin);
  float pressureVoltage = pressureRaw * (3.3 / 4095.0);
  currentPressure = (pressureVoltage - 0.5) * 200; // Example conversion
  currentPressure = constrain(currentPressure, 0.0, 400.0);
  
  // Read conductivity sensor
  int conductivityRaw = analogRead(conductivityPin);
  float conductivityVoltage = conductivityRaw * (3.3 / 4095.0);
  currentConductivity = conductivityVoltage * 5; // Example conversion
  currentConductivity = constrain(currentConductivity, 0.0, 20.0);
}

void monitorFlow() {
  if (millis() - lastUpdateTime >= updateInterval) {
    noInterrupts();
    unsigned long pulseCountCopy = flowPulseCount;
    flowPulseCount = 0;
    interrupts();
    
    flowRate = (pulseCountCopy / pulsesPerML) * (60.0 / (updateInterval / 1000.0));
    totalVolume += pulseCountCopy / pulsesPerML;
    
    lastUpdateTime = millis();
  }
}

void controlSolutions() {
  if (currentSolution == SOLUTION_A && dispensedA >= solutionAVolume) {
    stopPump();
    delay(100);
    startSolution(SOLUTION_B);
  } 
  else if (currentSolution == SOLUTION_B && dispensedB >= solutionBVolume) {
    stopPump();
    currentSolution = SOLUTION_NONE;
  }
  else if (currentSolution == SOLUTION_NONE && dispensedA < solutionAVolume) {
    startSolution(SOLUTION_A);
  }
}

void startSolution(Solution solution) {
  // Close all valves first
  digitalWrite(solenoidAPin, HIGH);
  digitalWrite(solenoidBPin, HIGH);
  delay(50);
  
  if (solution == SOLUTION_A) {
    digitalWrite(solenoidAPin, LOW);
    currentSolution = SOLUTION_A;
  } 
  else if (solution == SOLUTION_B) {
    digitalWrite(solenoidBPin, LOW);
    currentSolution = SOLUTION_B;
  }
  
  // Reset flow counter for new solution
  noInterrupts();
  flowPulseCount = 0;
  interrupts();
  
  // Start pump
  digitalWrite(relayPumpPin, LOW);
}

void stopPump() {
  digitalWrite(relayPumpPin, HIGH);
  digitalWrite(solenoidAPin, HIGH);
  digitalWrite(solenoidBPin, HIGH);
}

void updateDisplay() {
  static unsigned long lastDisplayUpdate = 0;
  static int displayMode = 0;
  
  if (millis() - lastDisplayUpdate >= 2000) { // Switch display every 2 seconds
    lcd.clear();
    
    switch(displayMode) {
      case 0: // Temperature and system status
        lcd.setCursor(0, 0);
        lcd.print("T:");
        lcd.print(currentTemp, 1);
        lcd.print("C ");
        lcd.print(heaterEnabled ? "HEAT" : "    ");
        
        lcd.setCursor(0, 1);
        if (wifiConnected) {
          lcd.print("WiFi: OK ");
        } else {
          lcd.print("WiFi: -- ");
        }
        if (dashboardConnected) {
          lcd.print("Dash: OK");
        } else {
          lcd.print("Dash: --");
        }
        break;
        
      case 1: // Flow and pressure
        lcd.setCursor(0, 0);
        lcd.print("Flow:");
        lcd.print(flowRate, 1);
        lcd.print("mL/m");
        
        lcd.setCursor(0, 1);
        lcd.print("Press:");
        lcd.print(currentPressure, 1);
        lcd.print("kPa");
        break;
        
      case 2: // Solution status
        lcd.setCursor(0, 0);
        if (currentSolution == SOLUTION_A) {
          lcd.print("Sol A: ");
          lcd.print(dispensedA, 0);
          lcd.print("/");
          lcd.print(solutionAVolume, 0);
        } else if (currentSolution == SOLUTION_B) {
          lcd.print("Sol B: ");
          lcd.print(dispensedB, 0);
          lcd.print("/");
          lcd.print(solutionBVolume, 0);
        } else {
          lcd.print("Solution: READY");
        }
        
        lcd.setCursor(0, 1);
        lcd.print("Conduct:");
        lcd.print(currentConductivity, 1);
        lcd.print("uS");
        break;
    }
    
    displayMode = (displayMode + 1) % 3;
    lastDisplayUpdate = millis();
  }
}

// ===== Alarm Functions =====
void temperatureAlarm() {
  stopPump();
  digitalWrite(relayHeaterPin, HIGH);
  digitalWrite(greenLedPin, LOW);
  digitalWrite(redLedPin, HIGH);
  systemRunning = false;
  
  lcd.clear();
  lcd.print("TEMP ALARM!");
  lcd.setCursor(0, 1);
  lcd.print(currentTemp, 1);
  lcd.print("C - STOPPED");
  
  // Send alarm to dashboard
  if (wifiConnected) {
    StaticJsonDocument<200> alarmDoc;
    alarmDoc["alarm"] = "TEMPERATURE";
    alarmDoc["value"] = currentTemp;
    alarmDoc["timestamp"] = getTimestamp();
    
    String alarmJson;
    serializeJson(alarmDoc, alarmJson);
    
    HTTPClient http;
    String alarmUrl = "http://" + String(dashboardIP) + ":" + String(dashboardPort) + "/api/alarm";
    http.begin(alarmUrl);
    http.addHeader("Content-Type", "application/json");
    http.POST(alarmJson);
    http.end();
  }
  
  // Sound alarm
  for (int i = 0; i < 10; i++) {
    tone(buzzerPin, 1000, 500);
    delay(1000);
  }
}

void pressureAlarm() {
  stopPump();
  digitalWrite(greenLedPin, LOW);
  digitalWrite(redLedPin, HIGH);
  systemRunning = false;
  
  lcd.clear();
  lcd.print("PRESSURE ALARM!");
  lcd.setCursor(0, 1);
  lcd.print(currentPressure, 1);
  lcd.print("kPa - STOPPED");
  
  tone(buzzerPin, 1500, 2000);
}

void errorState() {
  digitalWrite(redLedPin, HIGH);
  digitalWrite(greenLedPin, LOW);
  while(true) {
    tone(buzzerPin, 1000, 200);
    delay(400);
  }
}