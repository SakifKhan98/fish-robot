# fish-robot

## Overview

This repository contains the codebase for the **Fish Robot** project. The project simulates and controls a robotic fish, providing functionalities for data logging, movement control, and sensor integration.

## Features

- Simulated robotic fish movement.
- Data logging for analysis and debugging.
- Cross-platform support for running the scripts.
- Calibration and reading of load cells for precise measurements.

## How to Run `data_logger.py`

### Prerequisites

1. Install Python (version 3.7 or higher) from [python.org](https://www.python.org/).
2. Install the required dependencies by running:

```bash
pip install -r requirements.txt
```

### Running on Windows

1. Identify the Windows COM port for your device (e.g., `COM3`).
2. Run the `data_logger.py` script using the following command:

```bash
python data_logger.py --port COM3
```

Replace `COM3` with the actual port your device is connected to.

### Calibration and Reading Load Cells

#### Calibrating the Load Cell

1. Ensure the load cell is properly connected to the system.
2. Upload the calibration script to arduino and open serial monitor on arduino IDE:

```bash
More detiails will be updated here
```

3. Follow the on-screen instructions to complete the calibration process.

#### Reading Data from Two Load Cells

1. Connect both load cells to the system.
2. Upload the Read_2x_load_cell script to arduino read data from both load cells:

```bash
More Details will be updated here
```

3. The script will output the readings from both load cells for further analysis.
