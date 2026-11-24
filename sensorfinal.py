
import serial
import csv
import time
import math
from datetime import datetime

# ================ CONFIGURATION ================

SERIAL_PORT = "COM7"
BAUD_RATE = 115200

# MQ2: Propane
MQ2_M = -0.683
MQ2_B = 1.002

# MQ135: Hydrogen
MQ135_M = -0.523
MQ135_B = 0.347

filename = (
    "TRINETRA_GAS_PPM_"
    + datetime.now().strftime("%Y%m%d_%H%M%S")
    + ".csv"
)

# ================ PPM FUNCTION ================

def calculate_ppm(ratio, slope, intercept):

    try:

        ratio = float(ratio)

        if not math.isfinite(ratio) or ratio <= 0:
            return "NA"

        log_ppm = (
            math.log10(ratio) - intercept
        ) / slope

        ppm = 10 ** log_ppm

        if not math.isfinite(ppm) or ppm <= 0:
            return "NA"

        return round(ppm, 2)

    except (ValueError, OverflowError):
        return "NA"


# ================ SERIAL CONNECTION ================

try:

    ser = serial.Serial(
        SERIAL_PORT,
        BAUD_RATE,
        timeout=2
    )

    time.sleep(2)

    print("ESP32 Connected")
    print("Saving:", filename)

except serial.SerialException as e:

    print("Connection Error:", e)
    raise SystemExit


# ================ CSV HEADER ================

header = [
    "PC_Timestamp",
    "Time_ms",
    "Temperature_C",
    "Humidity_percent",

    "MQ2_ADC",
    "MQ2_Vadc",
    "MQ2_Vout",
    "MQ2_Rs",
    "MQ2_Ro",
    "MQ2_Ratio",
    "MQ2_PPM_Arduino",
    "MQ2_PPM_Python",

    "MQ135_ADC",
    "MQ135_Vadc",
    "MQ135_Vout",
    "MQ135_Rs",
    "MQ135_Ro",
    "MQ135_Ratio",
    "MQ135_PPM_Arduino",
    "MQ135_PPM_Python"
]

header_received = False

# ================ DATA LOGGER ================

try:

    with open(
        filename,
        "w",
        newline="",
        encoding="utf-8"
    ) as file:

        writer = csv.writer(file)

        while True:

            line = ser.readline().decode(
                "utf-8",
                errors="replace"
            ).strip()

            if not line:
                continue

            if line.startswith("#"):
                print(line)
                continue

            values = line.split(",")

            # Receive Arduino header
            if values[0] == "Time_ms":

                if len(values) != 17:
                    print("Incorrect header")
                    continue

                writer.writerow(header)

                header_received = True
                file.flush()

                print("CSV Header Received")
                continue

            if not header_received:
                continue

            if len(values) != 17:
                print("Invalid row:", line)
                continue

            # Extract sensor ratios
            mq2_ratio = values[8]
            mq135_ratio = values[15]

            # Calculate PPM independently
            mq2_ppm = calculate_ppm(
                mq2_ratio,
                MQ2_M,
                MQ2_B
            )

            mq135_ppm = calculate_ppm(
                mq135_ratio,
                MQ135_M,
                MQ135_B
            )

            timestamp = datetime.now().isoformat(
                timespec="milliseconds"
            )

            row = (
                [timestamp]
                + values[:10]
                + [mq2_ppm]
                + values[10:17]
                + [mq135_ppm]
            )

            writer.writerow(row)

            file.flush()

            print(
                f"MQ2 C3H8: {mq2_ppm} PPM | "
                f"MQ135 H2: {mq135_ppm} PPM"
            )

except KeyboardInterrupt:

    print("\nLogging stopped.")

finally:

    ser.close()

    print("Serial disconnected.")
    print("CSV saved:", filename)
