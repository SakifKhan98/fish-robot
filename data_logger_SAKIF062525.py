# import serial
# import csv
# from datetime import datetime

# # ----------------------------------------------------
# # Change the port to one your Arduino is connected to. Uncomment the appropriate line below.
# # ----------------------------------------------------

# # Arduino Port on Linux
# port = "/dev/ttyACM0"

# # Arduino Port on Mac
# # port = "/dev/cu.usbmodem14201"

# # Arduino Port Example on Windows
# # port = "COM3"

# baud = 9600  # Match the baud rate in your Arduino code
# output_file = "loadcell_current_power_SAKIF061625.csv"

# ser = serial.Serial(port, baud, timeout=1)
# print("Connected to", port)

# with open(output_file, "w", newline="") as csvfile:
#     writer = csv.writer(csvfile)
#     writer.writerow(
#         [
#             "Timestamp",
#             "Load Cell 1 (mN)",
#             "Load Cell 2 (mN)",
#             "Current (A)",
#             "Voltage (V)",
#             "Shunt Voltage (mV)",
#             "Power (W)",
#             "Manual Power (W)",
#         ]
#     )

#     while True:
#         try:
#             line = ser.readline().decode("utf-8").strip()
#             # print("Received line:", line)  # Debugging output

#             # Received line: Load_cell 1 output val: nan    Load_cell 2 output val: nan  Bus Voltage: 7.46 V  Shunt Voltage: 0.00 mV  Current: 0.004 mA  Power: 0.05 mW  manualPower: 0.03 mW

#             if "Load_cell" in line and "Current:" in line:
#                 # print("Processing line:", line)  # Debugging output
#                 # Example line format:
#                 # LoadCell 1: 12.345 N, LoadCell 2: 10.567 N, Current: 0.123 A, Bus Voltage: 12.4 V, Shunt Voltage: 0.5 mV, Power: 1.23 mW, Manual Power: 1.1 mW

#                 # Parts [' nan     nan   7.44    0.00 m   0.005 m   0.05   manual 0.04 ']
#                 parts = (
#                     line.replace("Load_cell 1 output val:", "")
#                     .replace("Load_cell 2 output val:", "")
#                     .replace("Bus Voltage:", "")
#                     .replace("Shunt Voltage:", "")
#                     .replace("Current:", "")
#                     .replace("Power:", "")
#                     .replace("manualPower:", "")
#                     .replace("N,", "")
#                     .replace("N", "")
#                     .replace("A", "")
#                     .replace("V", "")
#                     .replace("mV", "")
#                     .replace("mW", "")
#                     .replace("mW", "")
#                     .split(",")
#                 )
#                 print(parts)

#                 # Extract values from the parsed string
#                 val1 = float(parts[0].strip()) * 1000  # Convert N to mN
#                 val2 = float(parts[1].strip()) * 1000  # Convert N to mN
#                 current = float(parts[2].strip())  # Current in A
#                 bus_voltage = float(parts[3].strip())  # Bus Voltage in V
#                 shunt_voltage = float(parts[4].strip())  # Shunt Voltage in mV
#                 power = float(parts[5].strip()) / 1000  # Power in W (convert mW to W)
#                 manualpower = bus_voltage * current  # Manual Power calculation

#                 timestamp = datetime.now().astimezone().strftime("%Y-%m-%d %H:%M:%S %Z")
#                 writer.writerow(
#                     [
#                         timestamp,
#                         val1,
#                         val2,
#                         current,
#                         bus_voltage,
#                         shunt_voltage,
#                         power,
#                         manualpower,
#                     ]
#                 )
#                 print(
#                     f"{timestamp}, {val1:.2f} mN, {val2:.2f} mN, {current:.3f} A, {bus_voltage:.2f} V, {shunt_voltage:.3f} mV, {power:.3f} W, {manualpower:.3f} W"
#                 )

#         except Exception as e:
#             print("Error:", e)
#             continue

import serial
import csv
from datetime import datetime

# ----------------------------------------------------
# Change the port to one your Arduino is connected to. Uncomment the appropriate line below.
# ----------------------------------------------------

# Arduino Port on Linux
port = "/dev/ttyACM0"

# Arduino Port on Mac
# port = "/dev/cu.usbmodem14201"

# Arduino Port Example on Windows
# port = "COM3"

baud = 9600  # Match the baud rate in your Arduino code
output_file = "loadcell_current_power_SAKIF062525.csv"

ser = serial.Serial(port, baud, timeout=1)
print("Connected to", port)

with open(output_file, "w", newline="") as csvfile:
    writer = csv.writer(csvfile)
    writer.writerow(
        [
            "Timestamp",
            "Load Cell 1 (mN)",
            "Load Cell 2 (mN)",
            "Current (A)",
            "Voltage (V)",
            "Shunt Voltage (mV)",
            "Power (W)",
            "Manual Power (W)",
        ]
    )

    while True:
        try:
            line = ser.readline().decode("utf-8").strip()
            print("Received line:", line)  # Debugging output
            # Received line: Load_cell 1 output val: nan    Load_cell 2 output val: nan  Bus Voltage: 7.41 V  Shunt Voltage: 0.00 mV  Current: 0.005 mA  Power: 0.05 mW  manualPower: 0.04 mW

            if "Load_cell" in line and "Current:" in line:
                # Example line format:
                # Load_cell 1 output val: nan    Load_cell 2 output val: nan  Bus Voltage: 7.42 V  Shunt Voltage: 0.00 mV  Current: 0.005 mA  Power: 0.05 mW  manualPower: 0.04 mW

                # ['nan', 'nan', '7.41', '0.00', 'm', '0.005', '0.05', 'manual', '0.04']

                # Parse the line to extract data
                parts = (
                    line.replace("Load_cell 1 output val:", "")
                    .replace("Load_cell 2 output val:", "")
                    .replace("Bus Voltage:", "")
                    .replace("Shunt Voltage:", "")
                    .replace("Current:", "")
                    .replace("Power:", "")
                    .replace("Manual Power:", "")
                    .replace("Manual", "")
                    .replace("m_a", "")
                    .replace("m_w", "")
                    .replace("V", "")
                    .replace("m_v", "")
                    .split()
                )
                print(parts)

                # Extract values and handle 'nan' (Not a Number) values
                try:
                    val1 = (
                        float(parts[0].strip()) if parts[0].strip() != "nan" else 0
                    )  # Convert N to mN, handle 'nan'
                    val2 = (
                        float(parts[1].strip()) if parts[1].strip() != "nan" else 0
                    )  # Convert N to mN, handle 'nan'
                    bus_voltage = float(parts[2].strip())
                    shunt_voltage = float(parts[3].strip())
                    current = float(parts[4].strip())
                    power = float(parts[5].strip())
                    manual_power = float(parts[6].strip())
                except ValueError:
                    # Handle parsing errors if any data cannot be converted to float
                    print("Error parsing values.")
                    continue

                # Get the current timestamp
                timestamp = datetime.now().astimezone().strftime("%Y-%m-%d %H:%M:%S %Z")

                # Write the data to the CSV file
                writer.writerow(
                    [
                        timestamp,
                        val1,
                        val2,
                        current,
                        bus_voltage,
                        shunt_voltage,
                        power,
                        manual_power,
                    ]
                )

                # ['nan', 'nan', '7.37', '0.00', '0.006', '0.05', '0.04']
                # 2025-06-25 16:38:52 CDT, 0.00 mN, 0.00 mN, 0.000 A, 7.37 V, 0.000 mV, 0.000 W, 0.000 W
                # Received line: Load_cell 1 output val: nan Load_cell 2 output val: nanBus Voltage: 7.37 V Shunt Voltage: 0.00 m_v Current: 0.005 m_a  Power: 0.05 m_w Manual Power: 0.04 m_w

                # Print the data to the console for debugging
                print(
                    f"{timestamp}, {val1:.2f} mN, {val2:.2f} mN, {current:.3f} A, {bus_voltage:.2f} V, {shunt_voltage:.3f} V, {power:.3f} W, {manual_power:.3f} W"
                )

        except Exception as e:
            print("Error:", e)
            continue
