
# Real-Time Portable Dialysis Monitoring System

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-ESP32-blue.svg)](https://www.espressif.com/en/products/socs/esp32)
[![Framework](https://img.shields.io/badge/Framework-Arduino-00979D.svg)](https://www.arduino.cc/)
[![UI](https://img.shields.io/badge/UI-HTML/CSS/JS-orange.svg)](#)

A comprehensive, ESP32-based solution for monitoring and controlling portable dialysis applications. This project ensures patient safety by tracking critical parameters in real-time and providing instant alerts via a modern, glass-morphism web dashboard.

## 🏥 Project Overview

This system provides a complete hardware and software solution for monitoring the key parameters of a dialysis treatment: **temperature, flow rate, pressure, and conductivity**. The core of the system is an ESP32 microcontroller that manages sensors, controls pumps and heaters, and provides immediate local feedback.

All data is transmitted wirelessly to a sleek, responsive web dashboard, allowing for remote visualization, status monitoring, and control, ensuring the highest level of safety and operational awareness.

## ✨ Key Features

### 🔬 Hardware Controller (ESP32)

-   **Multi-Sensor Integration**: Continuously monitors Temperature, Flow Rate, Pressure, and Conductivity.
-   **Automated Safety Controls**: Manages a heating element to maintain optimal temperature and shuts down the system if pressure exceeds safe limits.
-   **Solution Management**: Controls two solenoid valves for precise dispensing of Solution A and Solution B.
-   **Wireless Data Transmission**: Utilizes built-in WiFi to send real-time data to the web dashboard.
-   **On-Device Interface**: A 16x2 I2C LCD provides immediate, at-a-glance status updates.
-   **Audio-Visual Alerts**: An onboard buzzer and status LEDs (Green/Red) provide clear system feedback.
-   **Emergency Stop**: A physical button allows for immediate system shutdown in critical situations.

### 🖥️ Web Dashboard

-   **Modern Glass-morphism UI**: A clean, intuitive, and visually appealing interface.
-   **Real-Time Data Visualization**: Circular progress indicators and clear readouts for all sensor data.
-   **Dynamic Alert System**: Visual and audible browser notifications for any parameter that falls outside the safe range.
-   **Status Monitoring**: Clear indicators for device connectivity and overall system health.
-   **Remote Control**: Start and stop the monitoring process directly from the web interface.
-   **Fully Responsive**: The layout is optimized for desktops, tablets, and mobile phones.

## 🛠️ System Architecture

The system operates in a simple, robust loop: the ESP32 gathers data from all connected sensors, processes it, performs safety checks, and then sends a structured JSON payload over WiFi to the web dashboard, which then visualizes the information.

```
[Sensors] --> [ESP32 Controller] --(WiFi)--> [Web Dashboard]
```

## ⚙️ Technology Stack

| Hardware                    | Software & Dashboard         |
| --------------------------- | ---------------------------- |
| ESP32 Development Board     | Arduino IDE / C++            |
| DS18B20 Temperature Sensor  | **Arduino Libraries:**       |
| Hall Effect Flow Sensor     | `WiFi`, `HTTPClient`         |
| Analog Pressure Sensor      | `ArduinoJson`                |
| Analog Conductivity Sensor  | `LiquidCrystal_I2C`          |
| 4-Channel Relay Module      | `OneWire`, `DallasTemperature` |
| 12V Solenoid Valves (x2)    | **Dashboard:**               |
| 12V DC Water Pump           | HTML5                        |
| 12V DC Heating Element      | CSS3 (Flexbox, Grid)         |
| 16x2 I2C LCD Display        | Vanilla JavaScript (Fetch API) |
| Push Buttons, LEDs, Buzzer  |                              |

## 🔌 Hardware Wiring

Connect the components to the ESP32 as follows. Ensure all power connections (12V for pump/heater/valves, 5V for sensors/relays) are properly regulated and sourced.

```
ESP32 Pin Connections:
├── Pin 2  → Flow Sensor (Interrupt Pin)
├── Pin 4  → DS18B20 Temperature Sensor (OneWire Bus)
├── Pin 0  → Start/Stop Button (Active LOW, requires pull-up)
├── Pin 18 → Pump Relay Control (Active LOW)
├── Pin 19 → Heater Relay Control (Active LOW)
├── Pin 21 → Solenoid Valve A Relay Control (Active LOW)
├── Pin 22 → Solenoid Valve B Relay Control (Active LOW)
├── Pin 23 → Buzzer
├── Pin 25 → Green LED (System OK)
├── Pin 26 → Red LED (System Error/Stopped)
├── Pin 34 → Pressure Sensor (Analog Input)
├── Pin 35 → Conductivity Sensor (Analog Input)
├── SDA    → LCD Display SDA
└── SCL    → LCD Display SCL
```

## 🚀 Setup and Installation

### 1. Configure the Arduino Environment

1.  **Install Arduino IDE** (version 1.8.19 or later).
2.  **Add ESP32 Board Support**:
    -   Go to `File` → `Preferences`.
    -   In "Additional Board Manager URLs", add:
        ```
        https://dl.espressif.com/dl/package_esp32_index.json
        ```
    -   Go to `Tools` → `Board` → `Boards Manager`, search for "esp32", and install it.
3.  **Install Required Libraries**:
    -   Go to `Sketch` → `Include Library` → `Manage Libraries`.
    -   Search for and install the following:
        -   `ArduinoJson`
        -   `LiquidCrystal I2C`
        -   `OneWire`
        -   `DallasTemperature`

### 2. Flash the ESP32 Controller

1.  Clone this repository or download the project files.
2.  Open the `dialysis-controller.cpp` file in the Arduino IDE.
3.  **Update WiFi Credentials**: Modify the following lines with your network details:
    ```cpp
    const char* ssid = "YOUR_WIFI_NAME";
    const char* password = "YOUR_WIFI_PASSWORD";
    ```
4.  **Set Dashboard IP Address**: Find your computer's local IP address and update this line:
    ```cpp
    const char* dashboardIP = "192.168.1.100";  // Replace with your computer's IP
    ```5.  Connect your ESP32 board, select the correct COM port and board type from the `Tools` menu, and click the **Upload** button.

### 3. Launch the Web Dashboard

1.  Ensure your computer is connected to the same WiFi network as the ESP32.
2.  Open the `dialysis-dashboard.html` file in any modern web browser (e.g., Chrome, Firefox).
3.  Power on the ESP32. The dashboard should automatically start receiving data and update the display.

## 🔧 Configuration and Calibration

Fine-tune the system by adjusting the constants defined at the top of the `dialysis-controller.cpp` file.

-   **Sensor Calibration**: Adjust the multipliers and offsets for the flow, pressure, and conductivity sensors to match your specific hardware datasheets.
    ```cpp
    const float pulsesPerML = 7.5; // From flow sensor datasheet
    // Example pressure calibration: 0.5V offset, 200 kPa/V sensitivity
    currentPressure = (pressureVoltage - 0.5) * 200;
    ```
-   **Operating Parameters**: Set the normal and critical thresholds for all monitored values.
-   **Solution Volumes**: Define the target volumes (in mL) for Solution A and B.

| Parameter      | Normal Range  | Critical Limits |
| -------------- | ------------- | --------------- |
| Temperature    | 35-40°C       | 16-38°C         |
| Flow Rate      | 200-500 L/min | 0-600 L/min     |
| Pressure       | 50-300 kPa    | 0-350 kPa       |
| Conductivity   | 13-15 µS/cm   | 0-20 µS/cm      |

## 🔒 Safety Features

Safety is the primary design consideration for this project.

-   **Automatic Shutdowns**: The system will automatically halt operations and enter a safe state if:
    -   Temperature exceeds 38°C or drops below 16°C.
    -   Pressure rises above the critical threshold of 350 kPa.
    -   A critical sensor (e.g., temperature) fails to provide a reading.
-   **Multi-Level Alerts**: The system provides progressive warnings through the buzzer, LEDs, and dashboard notifications before a critical shutdown occurs.
-   **Manual Override**: The physical emergency stop button provides an immediate manual override to stop the system instantly.

## 📈 Future Work & Advanced Features

-   [ ] **Data Logging**: Integrate an SD card module to log treatment data locally for record-keeping and analysis.
-   [ ] **Cloud Integration**: Use MQTT to send data to a cloud IoT platform (like Adafruit IO or Thingspeak) for remote access from anywhere.
-   [ ] **Predictive Maintenance**: Log sensor drift and component usage (e.g., pump run-time) to predict when maintenance or recalibration is required.
-   [ ] **User Profiles**: Add functionality to the dashboard to save and load different treatment profiles.
        
## ⚠️ Medical Device Disclaimer

> **IMPORTANT**: This project is intended for educational, research, and development purposes only. It is **NOT** a certified medical device. Before use in any clinical or patient-care application, the system must undergo rigorous testing, validation, and certification to comply with all applicable medical device regulations and safety standards (e.g., IEC 60601, FDA regulations). The author is not responsible for any misuse of this project.

## ✍️ Author

-   **10xRashed** - [GitHub Profile](https://github.com/10xrashed)

## 📄 License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
