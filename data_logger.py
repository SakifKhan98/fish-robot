import serial
import csv
from datetime import datetime

# Arduino Port on my Linux Machine
port = "/dev/ttyACM0"
baud = 57600
output_file = "loadcell_readings.csv"

ser = serial.Serial(port, baud, timeout=1)
print("Connected to", port)

with open(output_file, "w", newline="") as csvfile:
    writer = csv.writer(csvfile)
    writer.writerow(["Timestamp", "Load Cell 1", "Load Cell 2"])

    while True:
        try:
            line = ser.readline().decode("utf-8").strip()
            if "Load_cell" in line:
                parts = (
                    line.replace("Load_cell 1 output val:", "")
                    .replace("Load_cell 2 output val:", "")
                    .split()
                )
                val1 = float(parts[0])
                val2 = float(parts[-1])
                # timestamp = datetime.now().isoformat()
                timestamp = datetime.now().astimezone().strftime("%Y-%m-%d %H:%M:%S %Z")
                writer.writerow([timestamp, val1, val2])
                print(f"{timestamp}, {val1}, {val2}")
        except Exception as e:
            print("Error:", e)
            continue
