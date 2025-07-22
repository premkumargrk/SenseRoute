# SenseRoute 
  - An IoT-Powered Ecosystem for Safe Navigation and Transport Detection

🚶‍♂️ IoT-Based Assistive Devices for the Visually Impaired
This project aims to enhance the navigation, safety, and independence of visually impaired individuals, especially in public transportation environments, through a set of integrated IoT-based assistive devices.

📦 System Overview
The system consists of three core components working together to provide real-time situational awareness and transport information:

 - Smart Cane

 - Smart Glass

 - Transport Detection System

🦯 Smart Cane
Equipped with ultrasonic sensors to detect ground-level obstacles like potholes and barriers.

Provides haptic feedback using vibration motors to alert the user of nearby hazards.

Contains an RFID reader to detect public transport via RFID tags placed on buses.

👓 Smart Glasses
Includes head-level ultrasonic sensors to detect overhead obstacles (e.g., signboards, tree branches).

Alerts the user via buzzer or vibration feedback, improving safety in complex environments.

Receives data from the smart cane over the ESP-NOW wireless protocol, ensuring low-latency communication.

🚌 Transport Detection System
RFID tags are placed on public transportation vehicles (e.g., buses).

The RFID reader embedded in the smart cane detects these tags.

Real-time transport info is transmitted wirelessly to the smart glasses, which then provide audio feedback about the approaching vehicle.

This helps users identify and board public transport independently and on time.

🔗 Communication Architecture
Uses ESP-NOW protocol for fast and energy-efficient wireless communication between the smart cane and smart glasses.

Designed for low-power operation and real-time responsiveness.

💡 Key Features
Multi-level obstacle detection (ground + head height)

Haptic and audio feedback for enhanced awareness

RFID-based public transport recognition

Wireless, real-time inter-device communication

Customizable and scalable for different urban environments

⚙️ Technologies Used
ESP32 Microcontrollers

Ultrasonic Sensors

RFID Reader/Tags (MFRC522)

Vibration Motors & Buzzers

ESP-NOW Communication Protocol

Li-ion Battery Power Supply

