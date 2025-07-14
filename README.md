# made by 10xRashed [the Only  10x Developer in jordan]
# Mobile Kidney Washing Unit

A comprehensive real-time monitoring and control system for portable dialysis applications, featuring ESP32-based hardware control and a modern web dashboard interface.

## 🏥 Overview

This project provides a complete solution for monitoring critical parameters in dialysis treatment, ensuring patient safety through continuous monitoring of temperature, flow rate, pressure, and conductivity. The system combines robust embedded hardware control with an intuitive web-based dashboard for real-time visualization and alerts.

## ✨ Features

### Hardware Controller (ESP32)
- **Real-time Sensor Monitoring**: Temperature, flow rate, pressure, and conductivity
- **Automatic Safety Controls**: Temperature regulation and pressure monitoring
- **Solution Management**: Automated dispensing of Solution A and Solution B
- **WiFi Connectivity**: Wireless data transmission to dashboard
- **Local Display**: 16x2 LCD for on-device status monitoring
- **Audio/Visual Alerts**: Buzzer and LED indicators for system status
- **Emergency Stop**: Immediate system shutdown capability

### Web Dashboard
- **Modern UI**: Glass-morphism design with responsive layout
- **Real-time Visualization**: Circular progress indicators for all parameters
- **Alert System**: Visual and audible notifications for out-of-range values
- **Status Monitoring**: Device connectivity and parameter health tracking
- **Remote Control**: Start/stop monitoring from web interface
- **Mobile Responsive**: Optimized for desktop, tablet, and mobile devices

## 🔧 Hardware Requirements

### Core Components
- **ESP32 Development Board** (DevKit or similar)
- **DS18B20 Temperature Sensor** - Waterproof version recommended
- **Flow Sensor** - Hall effect or turbine type with pulse output
- **Pressure Sensor** - 0-5V analog output, 0-400 kPa range
- **Conductivity Sensor** - Analog output, suitable for aqueous solutions

### Control Components
- **Relay Module (4-channel)** - For pump and heater control
- **Solenoid Valves (2x)** - For solution A and B control
- **Water Pump** - 12V DC, suitable for medical applications
- **Heating Element** - 12V DC, with appropriate wattage

### Interface Components
- **16x2 LCD Display** - I2C interface (PCF8574)
- **Push Button** - Momentary, normally open
- **Buzzer** - Active or passive, 5V compatible
- **LEDs** - Green and Red status indicators
- **Resistors** - Pull-up resistors for buttons and sensors

## 📋 Software Requirements

### Arduino IDE Setup
\`\`\`bash
# Install Arduino IDE (version 1.8.19 or later)
# Add ESP32 board support:
# File → Preferences → Additional Board Manager URLs:
# https://dl.espressif.com/dl/package_esp32_index.json
\`\`\`

### Required Libraries
\`\`\`cpp
WiFi                    // ESP32 WiFi (built-in)
HTTPClient             // HTTP requests (built-in)
ArduinoJson            // JSON handling
Wire                   // I2C communication (built-in)
LiquidCrystal_I2C     // LCD display control
OneWire               // 1-Wire protocol
DallasTemperature     // DS18B20 temperature sensor
\`\`\`

Install libraries via Arduino IDE Library Manager or:
\`\`\`bash
# Using Arduino CLI
arduino-cli lib install "ArduinoJson"
arduino-cli lib install "LiquidCrystal I2C"
arduino-cli lib install "OneWire"
arduino-cli lib install "DallasTemperature"
\`\`\`

## 🚀 Quick Start

### 1. Hardware Assembly
\`\`\`
ESP32 Pin Connections:
├── Pin 2  → Flow Sensor (Interrupt)
├── Pin 4  → DS18B20 Temperature Sensor
├── Pin 0  → Start/Stop Button (Pull-up)
├── Pin 18 → Pump Relay (Active LOW)
├── Pin 19 → Heater Relay (Active LOW)
├── Pin 21 → Solenoid Valve A (Active LOW)
├── Pin 22 → Solenoid Valve B (Active LOW)
├── Pin 23 → Buzzer
├── Pin 25 → Green LED (System Running)
├── Pin 26 → Red LED (System Error)
├── Pin 34 → Pressure Sensor (Analog)
├── Pin 35 → Conductivity Sensor (Analog)
└── I2C    → LCD Display (SDA: Pin 21, SCL: Pin 22)
\`\`\`

### 2. Software Configuration

1. **Clone or download the project files**
2. **Open \`dialysis-controller.cpp\` in Arduino IDE**
3. **Update WiFi credentials:**
   \`\`\`cpp
   const char* ssid = "YOUR_WIFI_NAME";
   const char* password = "YOUR_WIFI_PASSWORD";
   \`\`\`
4. **Set dashboard IP address:**
   \`\`\`cpp
   const char* dashboardIP = "192.168.1.100";  // Your computer's IP
   \`\`\`
5. **Upload code to ESP32**

### 3. Dashboard Setup

1. **Find your computer's IP address:**
   - Windows: \`ipconfig\`
   - Mac/Linux: \`ifconfig\`

2. **Open \`dialysis-dashboard.html\` in a web browser**
   - For local file: Double-click the HTML file
   - For web server: Use Live Server extension in VS Code

3. **Verify connection between ESP32 and dashboard**

## ⚙️ Configuration

### Sensor Calibration

#### Flow Sensor
\`\`\`cpp
const float pulsesPerML = 7.5;  // Adjust based on your sensor specifications
\`\`\`

#### Pressure Sensor
\`\`\`cpp
// Calibrate based on sensor datasheet
currentPressure = (pressureVoltage - 0.5) * 200; // Example: 0.5V offset, 200 kPa/V
\`\`\`

#### Conductivity Sensor
\`\`\`cpp
// Adjust multiplier based on sensor characteristics
currentConductivity = conductivityVoltage * 5; // Example: 5 µS/cm per volt
\`\`\`

### Operating Parameters

| Parameter | Normal Range | Critical Limits |
|-----------|--------------|-----------------|
| Temperature | 35-40°C | 16-38°C |
| Flow Rate | 200-500 L/min | 0-600 L/min |
| Pressure | 50-300 kPa | 0-350 kPa |
| Conductivity | 13-15 µS/cm | 0-20 µS/cm |

### Solution Volumes
\`\`\`cpp
float solutionAVolume = 1000.0;   // mL
float solutionBVolume = 500.0;    // mL
\`\`\`

## 🔒 Safety Features

### Automatic Safety Shutdowns
- **Temperature Limits**: System stops if temperature exceeds 38°C or drops below 16°C
- **High Pressure**: Automatic shutdown at pressures above 350 kPa
- **Sensor Failures**: System stops if critical sensors fail
- **Emergency Stop**: Manual override button for immediate shutdown

### Alert System
- **Audio Alerts**: Buzzer notifications for critical conditions
- **Visual Indicators**: LED status lights and LCD messages
- **Remote Notifications**: Dashboard alerts and status updates
- **Multi-level Warnings**: Progressive alerts before critical shutdowns

## 📊 Dashboard Features

### Real-time Monitoring
- **Circular Progress Indicators**: Visual representation of all sensor values
- **Status Badges**: Color-coded system health indicators
- **Auto-refresh**: Configurable update intervals (default: 2 seconds)
- **Connection Status**: WiFi and device connectivity monitoring

### Control Interface
- **Remote Start/Stop**: Control system operation from dashboard
- **Manual Refresh**: Force immediate data update
- **Parameter Monitoring**: Track all critical values in real-time

### Alert Management
- **Visual Alerts**: Color-coded warnings and errors
- **Parameter Thresholds**: Configurable alert limits
- **Alert History**: Track and review past alerts

## 🔧 Troubleshooting

### Common Issues

#### WiFi Connection Problems
\`\`\`cpp
// Check serial monitor output for connection status
Serial.println("WiFi connected!");
Serial.print("IP address: ");
Serial.println(WiFi.localIP());
\`\`\`

#### Sensor Reading Issues
- **Temperature**: Verify DS18B20 wiring and 4.7kΩ pull-up resistor
- **Flow**: Check interrupt pin connection and sensor power
- **Pressure/Conductivity**: Verify analog pin connections and sensor power

#### Dashboard Connection Issues
1. Verify IP address configuration matches between ESP32 and computer
2. Check that both devices are on the same network
3. Ensure firewall allows HTTP connections on configured port
4. Monitor ESP32 serial output for HTTP error codes

### Debug Mode
Enable detailed logging:
\`\`\`cpp
Serial.begin(115200);
Serial.setDebugOutput(true);
\`\`\`

## 🛠️ Maintenance

### Regular Maintenance Schedule

#### Daily
- [ ] Visual inspection of all connections
- [ ] Verify sensor readings are within normal ranges
- [ ] Test emergency stop function

#### Weekly
- [ ] Clean flow sensor to remove debris
- [ ] Check all electrical connections for corrosion
- [ ] Verify relay operation

#### Monthly
- [ ] Calibrate all sensors with known references
- [ ] Check software for updates
- [ ] Inspect wiring for wear or damage
- [ ] Test all alarm functions

#### Quarterly
- [ ] Complete system validation
- [ ] Replace consumable components
- [ ] Update documentation
- [ ] Conduct training review

## ⚠️ Important Safety Warnings

> **MEDICAL DEVICE WARNING**: This system is designed for educational and development purposes. Before using in any medical application, ensure compliance with all applicable medical device regulations and standards.

> **ELECTRICAL SAFETY**: Always disconnect power before making any electrical connections. Use proper grounding and isolation techniques.

> **WATER SAFETY**: Ensure all electrical components are properly sealed and protected from water ingress.

> **EMERGENCY PROCEDURES**: Always have manual override capabilities and emergency shutdown procedures in place.

## 📈 Advanced Features

### Data Logging
Add SD card support for treatment record keeping:
\`\`\`cpp
#include <SD.h>
#include <SPI.h>

void logData(float temp, float flow, float pressure, float conductivity) {
  File dataFile = SD.open("treatment_log.csv", FILE_WRITE);
  // Implementation details in  guide
}
\`\`\`

### Remote Monitoring
- **MQTT Integration**: Connect to IoT platforms
- **Cloud Connectivity**: Remote access via internet
- **Mobile Applications**: Smartphone monitoring capabilities

### Predictive Maintenance
- **Trend Analysis**: Monitor parameter drift over time
- **Predictive Alerts**: Early warning system for maintenance needs
- **Usage Statistics**: Track system utilization and performance

## 🤝 Contributing

We welcome contributions to improve the Mobile Kidney Washing Unit project!

### How to Contribute
1. Fork the repository
2. Create a feature branch (\`git checkout -b feature/AmazingFeature\`)
3. Commit your changes (\`git commit -m 'Add some AmazingFeature'\`)
4. Push to the branch (\`git push origin feature/AmazingFeature\`)
5. Open a Pull Request

### Development Guidelines
- Follow existing code style and conventions
- Add comments for complex functionality
- Test all changes thoroughly
- Update documentation as needed
- Ensure safety features remain intact

### Reporting Issues
- Use the issue tracker for bug reports and feature requests
- Provide detailed information about your setup and the issue
- Include serial monitor output when reporting bugs
- Specify hardware configuration and software versions

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
\`\`\`
