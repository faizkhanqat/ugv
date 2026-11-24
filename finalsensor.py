
import requests
import csv
import time
import re
from pathlib import Path
from datetime import datetime

# ---------------- CONFIGURATION ----------------

ESP32_URL = "http://10.170.214.175/sensor"

INTERVAL_SECONDS = 3

# 1-hour experiment
SESSION_MINUTES = 60

TOTAL_SECONDS = SESSION_MINUTES * 60

# -----------------------------------------------

def get_next_trial_id():
    """Generate the next trial number from existing CSV files."""

    existing_files = Path(".").glob("ESP32_sensor_data_trial_*.csv")
    numbers = []

    for file in existing_files:
        match = re.search(r"trial_(\d+)_", file.name)

        if match:
            numbers.append(int(match.group(1)))

    next_number = max(numbers, default=0) + 1

    return f"trial_{next_number:03d}"


def get_sensor_data():
    """Read sensor values from ESP32 using Wi-Fi."""

    response = requests.get(ESP32_URL, timeout=5)
    response.raise_for_status()

    data = response.json()

    required_keys = [
        "mq2_adc",
        "mq135_adc",
        "temperature_c",
        "humidity_pct"
    ]

    for key in required_keys:
        if key not in data:
            raise ValueError(
                f"Missing ESP32 field: {key}. "
                f"Received JSON: {data}"
            )

    return data


def main():

    trial_id = get_next_trial_id()

    session_timestamp = datetime.now().strftime(
        "%Y%m%d_%H%M%S"
    )

    filename = (
        f"ESP32_sensor_data_{trial_id}_"
        f"{session_timestamp}.csv"
    )

    headings = [
        "timestamp",
        "elapsed_s",
        "trial_id",
        "mq2_adc",
        "mq135_adc",
        "temperature_c",
        "humidity_pct"
    ]

    saved_rows = 0
    start_time = None

    print("\nESP32 WI-FI SENSOR DATA COLLECTION")
    print("----------------------------------")
    print("ESP32 URL:", ESP32_URL)
    print("Trial ID:", trial_id)
    print("Session duration:", SESSION_MINUTES, "minutes")
    print("Sampling interval:", INTERVAL_SECONDS, "seconds")
    print("Expected readings: approximately 1200")
    print("CSV file:", filename)
    print("\nWaiting for ESP32 readings...\n")

    try:
        with open(
            filename, "w", newline="", encoding="utf-8"
        ) as csvfile:

            writer = csv.DictWriter(
                csvfile,
                fieldnames=headings
            )

            writer.writeheader()
            csvfile.flush()

            try:
                while True:

                    # Stop when one hour is completed
                    if start_time is not None:
                        elapsed = time.monotonic() - start_time

                        if elapsed >= TOTAL_SECONDS:
                            break

                    # Get data from ESP32
                    try:
                        data = get_sensor_data()

                    except (
                        requests.RequestException,
                        ValueError
                    ) as error:

                        print("Wi-Fi read failed:", error)
                        print("Retrying in 1 second...\n")
                        time.sleep(1)
                        continue

                    # Start timer at first successful reading
                    if start_time is None:
                        start_time = time.monotonic()

                    elapsed = time.monotonic() - start_time

                    if elapsed >= TOTAL_SECONDS:
                        break

                    timestamp = datetime.now().astimezone().isoformat(
                        timespec="seconds"
                    )

                    # Prepare CSV row
                    row = {
                        "timestamp": timestamp,
                        "elapsed_s": round(elapsed, 2),
                        "trial_id": trial_id,
                        "mq2_adc": data["mq2_adc"],
                        "mq135_adc": data["mq135_adc"],
                        "temperature_c": data["temperature_c"],
                        "humidity_pct": data["humidity_pct"]
                    }

                    # Save reading
                    writer.writerow(row)
                    csvfile.flush()

                    saved_rows += 1

                    # Display reading
                    print(
                        f"Trial: {trial_id} | "
                        f"Elapsed: {elapsed:.1f} s | "
                        f"MQ-2: {data['mq2_adc']} | "
                        f"MQ-135: {data['mq135_adc']} | "
                        f"Temperature: {data['temperature_c']} C | "
                        f"Humidity: {data['humidity_pct']}%"
                    )

                    # Wait until next sample
                    remaining = TOTAL_SECONDS - (
                        time.monotonic() - start_time
                    )

                    if remaining > 0:
                        time.sleep(
                            min(INTERVAL_SECONDS, remaining)
                        )

            except KeyboardInterrupt:
                print("\nCollection stopped manually.")

    except OSError as error:
        print("\nCould not create or write the CSV file:", error)
        return

    print("\nCOLLECTION FINISHED")
    print("-------------------")
    print("Trial ID:", trial_id)
    print("Readings saved:", saved_rows)
    print("CSV file location:", Path(filename).resolve())


if __name__ == "__main__":
    main()
