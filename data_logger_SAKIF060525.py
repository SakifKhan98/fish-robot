import serial
import csv
from datetime import datetime

# ----------------------------------------------------
# Change the port to one ypur arduino is conected to. Comment the linux port and uncomment the windows port below.
# ----------------------------------------------------

# Arduino Port on my Linux Machine
# port = "/dev/ttyACM0"

# Arduino Port on my Mac Machine
port = "/dev/cu.usbmodem14201"

# Arduino Port Example on a Windows Machine
# port = "COM3"

baud = 57600
output_file = "loadcell_readings_SAKIF062525.csv"

ser = serial.Serial(port, baud, timeout=1)
print("Connected to", port)

with open(output_file, "w", newline="") as csvfile:
    writer = csv.writer(csvfile)
    writer.writerow(["Timestamp", "Load Cell 1 (mN)", "Load Cell 2 (mN)"])

    while True:
        try:
            line = ser.readline().decode("utf-8").strip()
            if "Load_cell" in line:
                parts = (
                    line.replace("Load_cell 1 output val:", "")
                    .replace("Load_cell 2 output val:", "")
                    .split()
                )
                # print(parts[0], parts[1])
                # print(type(parts[0]))
                val1 = float(parts[0])
                val2 = float(parts[-1])
                # print(f"{val1:.2f}")
                # timestamp = datetime.now().isoformat()
                timestamp = datetime.now().astimezone().strftime("%Y-%m-%d %H:%M:%S %Z")
                writer.writerow([timestamp, val1, val2])
                print(f"{timestamp}, {val1:.2f} mN, {val2:.2f} mN")
        except Exception as e:
            print("Error:", e)
            continue
