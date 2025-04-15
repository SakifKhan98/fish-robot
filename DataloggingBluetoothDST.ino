#include <TinyGPSPlus.h>
#include <SD.h>
#include <SPI.h>
#include <Arduino_LSM6DS3.h>
#include <ArduinoBLE.h>
#include <ArduinoLowPower.h>

// Pin Definitions
#define SD_CS 10
#define PPS_PIN 2
#define LED_PIN 13

// Constants
#define BUFFER_SIZE 100
#define IMU_INTERVAL 100   // IMU data collection interval (ms)
#define GPS_INTERVAL 1000  // GPS data collection interval (ms)

// Global Variables
TinyGPSPlus gps;
File dataFile;
BLEService controlService("180A");
BLEStringCharacteristic controlCharacteristic("2A57", BLEWrite | BLERead, 10);

bool sensorsEnabled = true;
bool sensorsWereDisabled = false;
String dataBuffer[BUFFER_SIZE];
int bufferIndex = 0;

unsigned long lastIMUTime = 0, lastGPSTime = 0;
char lastTimestamp[10] = "00:00:00";
char logFilename[20] = "default.csv";

double lastLat = 0.0, lastLng = 0.0;
float lastAltitude = 0.0;
int lastSatellites = 0;

void setup() {
    Serial.begin(115200);
    initSensors();
    setupBLE();
    Serial.println("System ready. Send 'STOP' to disable sensors, 'START' to resume.");
}

void loop() {
    BLEDevice central = BLE.central();

    if (central) {
        processBLECommands();
    }

    // Keep BLE active while sensors are disabled
    while (!sensorsEnabled) {
        BLE.poll();
        checkBLECommand();  
        delay(500);
    }

    if (sensorsWereDisabled) {
        restartSensors();
        sensorsWereDisabled = false;
        Serial.println("Sensors restarted successfully.");
    }

    collectSensorData();
}

// ------------------ Sensor & BLE Functions ------------------

void initSensors() {
    Serial1.begin(9600);
    pinMode(PPS_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);

    if (!IMU.begin()) Serial.println("IMU initialization failed!");
    if (!SD.begin(SD_CS)) Serial.println("SD card initialization failed!");
}

void setupBLE() {
    if (!BLE.begin()) {
        Serial.println("BLE initialization failed!");
        while (1);
    }

    BLE.setLocalName("Nano33_BLE_Sensor");
    BLE.setAdvertisedService(controlService);
    controlService.addCharacteristic(controlCharacteristic);
    BLE.addService(controlService);
    controlCharacteristic.writeValue("START");
    BLE.advertise();
}

void processBLECommands() {
    if (controlCharacteristic.written()) {
        String cmd = controlCharacteristic.value();
        Serial.print("Received: ");
        Serial.println(cmd);

        if (cmd == "STOP" && sensorsEnabled) {
            sensorsEnabled = false;
            sensorsWereDisabled = true;
            powerDownSensors();
            Serial.println("Sensors OFF. BLE still active.");
        }
    }
}

void checkBLECommand() {
    String bleValue = controlCharacteristic.value();

    if (bleValue == "START") {
        Serial.println("Detected START command! Restarting sensors...");
        sensorsEnabled = true;
    }
}

void powerDownSensors() {
    Serial1.end();
    IMU.end();
    SD.end();
    digitalWrite(LED_PIN, LOW);
    Serial.println("Sensors powered down. BLE still active.");
}

void restartSensors() {
    Serial.println("Reinitializing all sensors...");
    initSensors();
    Serial1.begin(9600); // Restart GPS serial connection
    BLE.advertise(); // Ensure BLE remains discoverable
    controlCharacteristic.writeValue("START"); // Notify BLE central
    digitalWrite(LED_PIN, HIGH);

    // Wait for a valid GPS fix before proceeding
    Serial.println("Waiting for valid GPS fix...");
    waitForValidGPS();
    Serial.println("GPS fix acquired. Resuming data collection.");
}

// ------------------ Data Collection & Logging ------------------

void collectSensorData() {
    unsigned long now = millis();

    if (now - lastGPSTime >= GPS_INTERVAL) {
        lastGPSTime = now;
        updateGPSData();
    }

    if (now - lastIMUTime >= IMU_INTERVAL) {
        lastIMUTime = now;
        storeIMUData();
    }
}

void updateGPSData() {
    while (Serial1.available()) {
        gps.encode(Serial1.read());
    }

    if (gps.location.isValid() && gps.time.isValid() && gps.date.isValid()) {
        int hour = gps.time.hour();
        
        // Adjust for Central Time (CST/CDT)
        int month = gps.date.month();
        int day = gps.date.day();
        bool isDST = isDaylightSaving(month, day);

        if (isDST) {
            hour += -5;  // CDT (UTC-5)
        } else {
            hour += -6;  // CST (UTC-6)
        }

        if (hour < 0) hour += 24; // Adjust for previous day

        sprintf(lastTimestamp, "%02d:%02d:%02d", hour, gps.time.minute(), gps.time.second());
        lastLat = gps.location.lat();
        lastLng = gps.location.lng();
        lastAltitude = gps.altitude.meters();
        lastSatellites = gps.satellites.value();

        sprintf(logFilename, "%04d%02d%02d.csv", gps.date.year(), gps.date.month(), gps.date.day());
        if (!SD.exists(logFilename)) createLogFile();
    }
}

bool isDaylightSaving(int month, int day) {
    // Daylight Saving Time starts on the second Sunday of March and ends on the first Sunday of November
    if (month < 3 || month > 11) return false;
    if (month > 3 && month < 11) return true;

    int secondSunday = (14 - (1 + 5) % 7) % 7 + 8;  // Second Sunday of March
    int firstSunday = (7 - (1 + 5) % 7) % 7 + 1;   // First Sunday of November

    if (month == 3) return (day >= secondSunday);
    if (month == 11) return (day < firstSunday);

    return false;
}

void storeIMUData() {
    float ax, ay, az, gx, gy, gz;
    
    if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
        IMU.readAcceleration(ax, ay, az);
        IMU.readGyroscope(gx, gy, gz);
    } else {
        ax = ay = az = gx = gy = gz = 0;
    }

    dataBuffer[bufferIndex++] = String(lastTimestamp) + "," + String(lastLat, 6) + "," + 
                                String(lastLng, 6) + "," + String(lastAltitude, 2) + "," + 
                                String(lastSatellites) + "," + String(ax, 3) + "," + 
                                String(ay, 3) + "," + String(az, 3) + "," + String(gx, 3) + "," + 
                                String(gy, 3) + "," + String(gz, 3);

    Serial.println(dataBuffer[bufferIndex - 1]);

    if (bufferIndex >= BUFFER_SIZE) writeBufferToSD();
}

void waitForValidGPS() {
    while (true) {
        while (Serial1.available()) {
            gps.encode(Serial1.read());
        }

        if (gps.location.isValid() && gps.time.isValid() && gps.date.isValid()) {
            break;  // GPS fix acquired
        }

        Serial.println("Waiting for GPS fix...");
        delay(1000);
    }
}

void createLogFile() {
    dataFile = SD.open(logFilename, FILE_WRITE);
    if (dataFile) {
        dataFile.println("Time,GPS_Lat,GPS_Lng,Altitude,Satellites,AccX,AccY,AccZ,GyroX,GyroY,GyroZ");
        dataFile.close();
        Serial.println("Log file created.");
    }
}

void writeBufferToSD() {
    dataFile = SD.open(logFilename, FILE_WRITE);
    if (dataFile) {
        for (int i = 0; i < bufferIndex; i++) {
            dataFile.println(dataBuffer[i]);
        }
        dataFile.close();
        bufferIndex = 0;
        Serial.println("Data saved to SD.");
    }
}
