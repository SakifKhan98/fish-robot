#include <ACS712.h>
#include <Servo.h>

// ACS712 setup (5A version on A0)
ACS712 sensor(ACS712_05B, A0);

// Servo setup on digital pin 9
Servo myServo;
int angle = 0;
int step = 1; // degrees per loop

unsigned long lastSweepTime = 0;
const unsigned long sweepDelay = 15; // ms between servo steps

unsigned long lastPrintTime = 0;
const unsigned long printInterval = 100; // ms for 10Hz current reading

void setup() {
  Serial.begin(9600);
  sensor.calibrate();
  Serial.println("ACS712 sensor calibrated.");

  myServo.attach(9);     // Attach servo to pin 9
  myServo.write(angle);  // Initial position
}

void loop() {
  unsigned long now = millis();

  // Print current at 10 Hz
  if (now - lastPrintTime >= printInterval) {
    float current = sensor.getCurrentAC();
    if (current < 0.09) current = 0;
    Serial.print("Current: ");
    Serial.print(current, 3);
    Serial.println(" A");
    lastPrintTime = now;
  }

  // Sweep the servo gradually
  if (now - lastSweepTime >= sweepDelay) {
    myServo.write(angle);
    angle += step;
    if (angle >= 180 || angle <= 0) step = -step; // reverse direction
    lastSweepTime = now;
  }
}
