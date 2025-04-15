/*
   -------------------------------------------------------------------------------------
   HX711_ADC - Dual Load Cell Example
   Arduino example for HX711 24-Bit Analog-to-Digital Converter for Weight Scales
   -------------------------------------------------------------------------------------

   This example demonstrates how to:
   - Use two separate HX711_ADC objects to read from two load cells.
   - Calibrate them both in a single script.
   - Store/retrieve each calibration value in different EEPROM locations.
   - Loadcell is Calibrated for Newton
*/

#include <HX711_ADC.h>
#if defined(ESP8266) || defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

/*****************************************************
   Pin Assignments
   Adjust as needed for your hardware setup.
 *****************************************************/

// Load Cell 1 pins
const int HX711_dout_1 = 6;   // MCU > HX711 dout pin
const int HX711_sck_1  = 7;   // MCU > HX711 sck pin

// Load Cell 2 pins
const int HX711_dout_2 = 4;   // MCU > second HX711 dout pin
const int HX711_sck_2  = 5;   // MCU > second HX711 sck pin

/*****************************************************
   Create Two Load Cell Objects
 *****************************************************/
HX711_ADC LoadCell1(HX711_dout_1, HX711_sck_1);
HX711_ADC LoadCell2(HX711_dout_2, HX711_sck_2);

/*****************************************************
   EEPROM Addresses
   Use distinct addresses so one calibration value
   doesn't overwrite the other.
 *****************************************************/
const int calVal_eepromAdress_1 = 0;   // EEPROM address for LoadCell1
const int calVal_eepromAdress_2 = 10;  // EEPROM address for LoadCell2 (just pick a different offset)

/*****************************************************
   Global Variables
 *****************************************************/
unsigned long t = 0;         // For timing in loop
const int serialPrintInterval = 500; // ms between prints (adjust as needed)

bool newDataReady1 = false;
bool newDataReady2 = false;

/*****************************************************
   Function Declarations
 *****************************************************/
void calibrateLoadCell(HX711_ADC &loadCell, int eepromAddress, const char *cellName);
void changeSavedCalFactor(HX711_ADC &loadCell, int eepromAddress, const char *cellName);

void setup() {
  Serial.begin(57600);
  delay(10);
  Serial.println();
  Serial.println("Starting...");

  // Begin communication with each load cell
  LoadCell1.begin();
  LoadCell2.begin();

  // Start each load cell, with optional tare at startup
  unsigned long stabilizingtime = 2000; // letting them stabilize
  bool doTare = true; // set false if you don’t want an initial tare

  // Initialize LoadCell1
  Serial.println("Initializing LoadCell1...");
  LoadCell1.start(stabilizingtime, doTare);
  if (LoadCell1.getTareTimeoutFlag() || LoadCell1.getSignalTimeoutFlag()) {
    Serial.println("Timeout for LoadCell1; check wiring/pin assignment.");
    while (1);
  } else {
    // Just set an initial placeholder factor
    LoadCell1.setCalFactor(1.0);
    Serial.println("LoadCell1 Startup complete.");
  }

  // Initialize LoadCell2
  Serial.println("Initializing LoadCell2...");
  LoadCell2.start(stabilizingtime, doTare);
  if (LoadCell2.getTareTimeoutFlag() || LoadCell2.getSignalTimeoutFlag()) {
    Serial.println("Timeout for LoadCell2; check wiring/pin assignment.");
    while (1);
  } else {
    LoadCell2.setCalFactor(1.0);
    Serial.println("LoadCell2 Startup complete.");
  }

  // Ensure we have at least one data update from both cells
  while (!LoadCell1.update() || !LoadCell2.update());

  /****************************************************
    OPTIONAL: Read any previously saved calibration
    from EEPROM so that each cell starts with
    the stored calibration factor.
  ****************************************************/
  float calFactor1, calFactor2;
  EEPROM.get(calVal_eepromAdress_1, calFactor1);
  EEPROM.get(calVal_eepromAdress_2, calFactor2);

  // If EEPROM value is not zero (or a plausible factor), you can decide to use it:
  if (calFactor1 != 0 && !isnan(calFactor1)) {
    LoadCell1.setCalFactor(calFactor1);
    Serial.print("Loaded calFactor for LoadCell1 from EEPROM: ");
    Serial.println(calFactor1);
  }
  if (calFactor2 != 0 && !isnan(calFactor2)) {
    LoadCell2.setCalFactor(calFactor2);
    Serial.print("Loaded calFactor for LoadCell2 from EEPROM: ");
    Serial.println(calFactor2);
  }

  // Now calibrate each load cell in turn:
  // If you only want to calibrate on demand, remove these calls and rely on serial commands.
  calibrateLoadCell(LoadCell1, calVal_eepromAdress_1, "LoadCell1");
  calibrateLoadCell(LoadCell2, calVal_eepromAdress_2, "LoadCell2");
}

void loop() {
  // Update both load cells
  if (LoadCell1.update()) {
    newDataReady1 = true;
  }
  if (LoadCell2.update()) {
    newDataReady2 = true;
  }

  // Print data if it's time
  if (millis() - t > serialPrintInterval) {
    if (newDataReady1) {
      float val1 = LoadCell1.getData();
      Serial.print("LoadCell1 output val: ");
      Serial.println(val1);
      newDataReady1 = false;
    }

    if (newDataReady2) {
      float val2 = LoadCell2.getData();
      Serial.print("LoadCell2 output val: ");
      Serial.println(val2);
      newDataReady2 = false;
    }

    t = millis();
  }

  // Check if last tare operation for each load cell is complete
  if (LoadCell1.getTareStatus()) {
    Serial.println("LoadCell1 Tare complete");
  }
  if (LoadCell2.getTareStatus()) {
    Serial.println("LoadCell2 Tare complete");
  }

  // Check Serial for commands
  // For example:
  //   't1' -> tare cell1, 't2' -> tare cell2
  //   'r1' -> calibrate cell1, 'r2' -> calibrate cell2
  //   'c1' -> change cal factor cell1, 'c2' -> change cal factor cell2
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    switch (inByte) {
      case 't':
        // We need a second character to see which cell to tare
        while (!Serial.available());
        inByte = Serial.read();
        if (inByte == '1') {
          LoadCell1.tareNoDelay();
        } else if (inByte == '2') {
          LoadCell2.tareNoDelay();
        }
        break;

      case 'r':
        // Re-calibrate either cell1 or cell2
        while (!Serial.available());
        inByte = Serial.read();
        if (inByte == '1') {
          calibrateLoadCell(LoadCell1, calVal_eepromAdress_1, "LoadCell1");
        } else if (inByte == '2') {
          calibrateLoadCell(LoadCell2, calVal_eepromAdress_2, "LoadCell2");
        }
        break;

      case 'c':
        // Change calibration value for cell1 or cell2
        while (!Serial.available());
        inByte = Serial.read();
        if (inByte == '1') {
          changeSavedCalFactor(LoadCell1, calVal_eepromAdress_1, "LoadCell1");
        } else if (inByte == '2') {
          changeSavedCalFactor(LoadCell2, calVal_eepromAdress_2, "LoadCell2");
        }
        break;

      default:
        Serial.println("Unknown command. Use 't1','t2','r1','r2','c1','c2'.");
        break;
    }
  }
}

/**********************************************************************
   Calibrate a given HX711_ADC load cell interactively via Serial.
   This is basically the same procedure as the original script’s
   `calibrate()` function, but now it’s parameterized so we can
   reuse it for any load cell object.
***********************************************************************/
void calibrateLoadCell(HX711_ADC &loadCell, int eepromAddress, const char *cellName) {
  Serial.println();
  Serial.println("************************************************");
  Serial.print("Start calibration for ");
  Serial.println(cellName);

  Serial.println("Place on a level, stable surface.");
  Serial.println("Remove all weight from the load cell.");
  Serial.println("Then send 't' from the serial monitor to tare.");
  bool tareComplete = false;

  // Wait for user to type 't' to tare
  while (!tareComplete) {
    loadCell.update();
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 't') {
        loadCell.tareNoDelay();
      }
    }
    if (loadCell.getTareStatus()) {
      Serial.println("Tare complete.");
      tareComplete = true;
    }
  }

  // Now prompt user to place a known mass
  Serial.println("Now place your known mass on the load cell.");
  Serial.println("Type the weight (e.g. 100.0) in the Serial Monitor, then press Enter.");

  float known_mass = 0;
  bool massEntered = false;
  while (!massEntered) {
    loadCell.update();
    if (Serial.available() > 0) {
      known_mass = Serial.parseFloat();
      if (known_mass != 0) {
        Serial.print("Known mass is: ");
        Serial.println(known_mass);
        massEntered = true;
      }
    }
  }

  // Refresh data set to get accurate reading
  loadCell.refreshDataSet();
  float newCalibrationValue = loadCell.getNewCalibration(known_mass);

  // Inform user
  Serial.print("New calibration value for ");
  Serial.print(cellName);
  Serial.print(": ");
  Serial.println(newCalibrationValue);

  Serial.print("Save this value to EEPROM address ");
  Serial.print(eepromAddress);
  Serial.println("? (y/n)");

  bool responded = false;
  while (!responded) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 'y') {
#if defined(ESP8266) || defined(ESP32)
        EEPROM.begin(512);
#endif
        EEPROM.put(eepromAddress, newCalibrationValue);
#if defined(ESP8266) || defined(ESP32)
        EEPROM.commit();
#endif
        EEPROM.get(eepromAddress, newCalibrationValue);
        Serial.print("Value ");
        Serial.print(newCalibrationValue);
        Serial.print(" saved to EEPROM address ");
        Serial.println(eepromAddress);
        responded = true;
      } else if (inByte == 'n') {
        Serial.println("Value not saved to EEPROM.");
        responded = true;
      }
    }
  }

  // Update the load cell’s calibration factor in RAM
  loadCell.setCalFactor(newCalibrationValue);

  Serial.print("Calibration for ");
  Serial.print(cellName);
  Serial.println(" complete.");
  Serial.println("************************************************");
  Serial.println();
}

/***********************************************************************
   Let the user manually input a new calibration value and optionally
   save it to EEPROM. Similar to original `changeSavedCalFactor()`.
***********************************************************************/
void changeSavedCalFactor(HX711_ADC &loadCell, int eepromAddress, const char *cellName) {
  float oldCalibrationValue = loadCell.getCalFactor();
  Serial.println("************************************************");
  Serial.print(cellName);
  Serial.print(": Current calibration value is: ");
  Serial.println(oldCalibrationValue);

  Serial.println("Now, send the new value from the serial monitor, e.g. 696.0");
  bool gotNewValue = false;
  float newCalibrationValue;
  while (!gotNewValue) {
    if (Serial.available() > 0) {
      newCalibrationValue = Serial.parseFloat();
      if (newCalibrationValue != 0) {
        Serial.print("New calibration value is: ");
        Serial.println(newCalibrationValue);
        loadCell.setCalFactor(newCalibrationValue);
        gotNewValue = true;
      }
    }
  }

  Serial.print("Save this value to EEPROM address ");
  Serial.print(eepromAddress);
  Serial.println("? (y/n)");

  bool responded = false;
  while (!responded) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 'y') {
#if defined(ESP8266) || defined(ESP32)
        EEPROM.begin(512);
#endif
        EEPROM.put(eepromAddress, newCalibrationValue);
#if defined(ESP8266) || defined(ESP32)
        EEPROM.commit();
#endif
        EEPROM.get(eepromAddress, newCalibrationValue);
        Serial.print("Value ");
        Serial.print(newCalibrationValue);
        Serial.print(" saved to EEPROM address ");
        Serial.println(eepromAddress);
        responded = true;
      } else if (inByte == 'n') {
        Serial.println("Value not saved to EEPROM.");
        responded = true;
      }
    }
  }

  Serial.println("End change calibration value.");
  Serial.println("************************************************");
  Serial.println();
}
