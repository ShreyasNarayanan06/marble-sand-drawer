import serial
import csv

# --- CONFIGURATION ---
COM_PORT = 'COM9'                # Change this to your STM32's COM port!
BAUD_RATE = 115200               # Matches your STM32 setup
FILENAME = "current_drawing.csv" # The single file that will be overwritten

def main():
    try:
        ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)
        print(f"✅ Successfully connected to {COM_PORT} at {BAUD_RATE} baud.")
        print(f"Waiting for drawing data... Will save to '{FILENAME}' (Press Ctrl+C to quit)")
    except Exception as e:
        print(f"❌ Error connecting to {COM_PORT}: {e}")
        return

    current_drawing = []

    try:
        while True:
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8').strip()

                if not line:
                    continue

                # 1. Handle Submit Command
                if line == "SAVE_FILE":
                    if len(current_drawing) > 0:
                        # Opening in 'w' mode automatically OVERWRITES the existing file
                        with open(FILENAME, mode='w', newline='') as file:
                            writer = csv.writer(file)
                            writer.writerow(["X", "Y"]) 
                            writer.writerows(current_drawing) 
                            
                        print(f"📁 Success! Overwrote '{FILENAME}' with {len(current_drawing)} points.")
                        current_drawing.clear()
                    else:
                        print("⚠️ Received SAVE_FILE, but no points were drawn.")

                # 2. Handle Clear Command
                elif line == "CLEAR_LOG":
                    current_drawing.clear()
                    print("🗑️ Screen cleared! Erased previous points from memory.")

                # 3. Handle Incoming Coordinates
                elif "," in line:
                    try:
                        x_str, y_str = line.split(',')
                        current_drawing.append([int(x_str), int(y_str)])
                    except ValueError:
                        pass # Ignore malformed data

    except KeyboardInterrupt:
        print("\n🛑 Disconnecting...")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print("Port closed.")

if __name__ == "__main__":
    main()