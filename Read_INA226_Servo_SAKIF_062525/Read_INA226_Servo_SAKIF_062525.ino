#define printdebug


#include <Wire.h>
#include <INA226.h>
#include <Servo.h>

// Initialize the INA226 sensor
INA226 ina(0x40);  // Default I2C address 0x40

// Servo setup
int pos = 0;
Servo myservo;  // Create Servo object to control the servo

void setup() {
  Serial.begin(9600);
  Wire.begin();

  // Connect to INA226 at default I2C address (0x40)
  if (ina.begin()) {
    Serial.println("INA226 connected successfully.");
  } else {
    Serial.println("Failed to connect to INA226.");
    while (1); // halt if INA226 fails to initialize
  }

  // Set calibration (shunt resistor = 0.1 ohm, max current = 3.2 A)
  ina.setMaxCurrentShunt(20.0, 0.002);   // Set max current for shunt resistor
  // ina.enableShuntOverflowAlert(true);  // Enable overflow alert for the shunt

  // Initialize Servo on pin 9
  myservo.attach(9);
}

void loop() {

  // Sweep the servo motor (0 to 180 degrees)
  for (pos = 0; pos <= 180; pos += 1) {  // Sweep from 0 to 180 degrees
    myservo.write(pos);   // Move servo to current position
    // Serial.print(pos);

      // Print readings to Serial Monitor
    float busVoltage = ina.getBusVoltage();       // in volts
    float shuntVoltage = ina.getShuntVoltage();   // in millivolts
    float current = ina.getCurrent() ;   // in amps (convert mA to A)
    float power = ina.getPower();                 // in milliwatts
    float manualpower = busVoltage * current;

    Serial.print("  Bus Voltage: ");
    Serial.print(busVoltage);
    Serial.print(" V  ");

    Serial.print("Shunt Voltage: ");
    Serial.print(shuntVoltage);
    Serial.print(" mV  ");

    Serial.print("Current: ");
    Serial.print(current, 3);  // Display in amps
    Serial.print(" mA  ");

    Serial.print("Power: ");
    Serial.print(power);
    Serial.print(" mW  ");

    Serial.print("manualPower: ");
    Serial.print(manualpower);
    Serial.println(" mW  ");

    delay(150);             // Delay to give servo time to move
  }

  // Sweep back (180 to 0 degrees)
  for (pos = 180; pos >= 0; pos -= 1) {  // Sweep from 180 to 0 degrees
    myservo.write(pos);   // Move servo to current position
    // Serial.print(pos);

    // Print readings to Serial Monitor
    float busVoltage = ina.getBusVoltage();       // in volts
    float shuntVoltage = ina.getShuntVoltage();   // in millivolts
    float current = ina.getCurrent() ;   // in amps (convert mA to A)
    float power = ina.getPower();                 // in milliwatts
    float manualpower = busVoltage * current;

    Serial.print("  Bus Voltage: ");
    Serial.print(busVoltage);
    Serial.print(" V  ");

    Serial.print("Shunt Voltage: ");
    Serial.print(shuntVoltage);
    Serial.print(" mV  ");

    Serial.print("Current: ");
    Serial.print(current, 3);  // Display in amps
    Serial.print(" mA  ");

    Serial.print("Power: ");
    Serial.print(power);
    Serial.print(" mW  ");

    Serial.print("manualPower: ");
    Serial.print(manualpower);
    Serial.println(" mW  ");

    delay(150);             // Delay to give servo time to move
  }
}
