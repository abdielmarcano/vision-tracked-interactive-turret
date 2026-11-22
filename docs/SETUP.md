# Setup and reproduction notes

This repository documents a working historical proof of concept. The recovered programs have **not been recompiled or smoke-tested** in this documentation package. Board, package, library, operating-system, and camera-version compatibility may differ from the original environment.

## Components

- Seeed Studio XIAO ESP32S3 Sense with camera
- MG996R servo with an appropriately rated separate motor power supply
- Common electrical ground between servo supply and microcontroller
- Laptop with Python and a wired Universal Serial Bus (USB) connection to the board
- Same local wireless network for the camera stream and computer

**Electrical caution:** Do not power the servo directly from a general-purpose microcontroller input/output pin. Confirm wiring, voltage, and the common ground before attaching power.

## Firmware

1. Open `firmware/VisionTrackedTurret/VisionTrackedTurret.ino` in Arduino Integrated Development Environment (IDE).
2. Configure suitable ESP32 board support, the camera drivers, and the `ESP32Servo` library. Use a board selection matching the XIAO ESP32S3 Sense hardware. The source camera example from Seeed was targeted at **esp-arduino 3.0.x** and reports compatibility concerns with some newer core releases; verify version compatibility rather than assuming the latest release will compile.
3. Replace the sketch's placeholder `ssid = "..."` and `password = "..."` with **local** network settings. Do not commit personal credentials to version control.
4. Confirm the chosen network and peripheral configuration, compile, and upload.
5. With a successful wireless connection, the firmware exposes the camera server. Its stream is at port `81`, path `/stream`.

**Known sequencing constraint:** The original firmware initializes servo control only after establishing a wireless connection. With placeholders unchanged, that step will not run.

## Laptop

1. Install Python dependencies:

   ```bash
   python -m pip install -r tracking/requirements.txt
   ```
2. In `tracking/face_tracking.py`, set `CAMERA_URL` to your board's local address, for example `http://YOUR_DEVICE_IP:81/stream`.
3. Change `SERIAL_PORT` from the historical Windows `COM3` setting to the port used by your board.
4. Close other programs that may hold the serial port open (including the Arduino Serial Monitor).
5. Start the script:

   ```bash
   python tracking/face_tracking.py
   ```
6. Test the face detector and servo with an unobstructed path of motion. Press `q` to quit, `c` to center, or `i` to reverse the tracking direction.

The sketch's wireless-network selection is based on partial name matching, which was acceptable for a controlled test but should be hardened before a public-space installation. There is no measured network recovery guarantee.

## Not included in this implementation

- Live motion-sensor activation of the camera-following program
- Room lighting control
- Completed prop enclosure
- Independent face detection on the microcontroller
- Multi-axis control, identity tracking, or object recognition
- Performance benchmarks or a reproducible hardware-test report
