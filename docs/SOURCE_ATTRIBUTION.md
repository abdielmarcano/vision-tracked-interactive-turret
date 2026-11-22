# Source attribution and provenance

This project combined an existing camera-web-server implementation with a separate Python/OpenCV computer-vision loop and custom servo-control integration. The author **does not claim to have written the camera server or the face-detection algorithm**.

## Confirmed upstream source

**Board-specific source:** [limengdu / SeeedStudio-XIAO-ESP32S3-Sense-camera, `CameraWebServer_for_esp-arduino_3.0.x`](https://github.com/limengdu/SeeedStudio-XIAO-ESP32S3-Sense-camera/tree/main/CameraWebServer_for_esp-arduino_3.0.x) (MIT license, repository copyright © 2023 limengdu).

**Underlying camera example:** [Espressif / Arduino-ESP32 CameraWebServer](https://github.com/espressif/arduino-esp32/tree/master/libraries/ESP32/examples/Camera/CameraWebServer) (the copied `app_httpd.cpp` retains its Espressif copyright and Apache-2.0 notice).

The Seeed repository's own README says the implementation was adapted from Espressif's CameraWebServer for the XIAO ESP32S3 Sense. The project's saved files were compared directly with the GitHub repository's default-branch copies on November 21, 2026:

| Project source file | Comparison result | Attribution |
|---|---|---|
| `firmware/VisionTrackedTurret/app_httpd.cpp` | **Character-for-character match** with upstream Seeed version | Third-party web server (Espressif-derived); not original project code |
| `firmware/VisionTrackedTurret/camera_index.h` | **Character-for-character match** | Third-party generated/compressed camera web UI, not original project code |
| `firmware/VisionTrackedTurret/camera_pins.h` | Same content apart from one leading newline | Third-party camera GPIO definitions; not original project code |
| `firmware/VisionTrackedTurret/VisionTrackedTurret.ino` | Adapted; **not** a direct match | Camera initialization and configuration follow the Seeed/Espressif examples; the saved sketch also includes project-specific hotspot management, command parsing, motor control and integration |

The note `modified 20260207 to correct the resolution selections` in `camera_index.h` **already appears in the Seeed upstream file**. Its individual author is not identified by that note, so it must **not** be attributed to the turret author.

## Project-specific engineering work

- Combined the camera stream with a laptop-based computer-vision application and a wired serial command channel back to the microcontroller.
- Wrote/integrated the host-side Python program to capture Motion JPEG frames, call OpenCV's **existing** Haar cascade frontal-face detector, choose a face, calculate horizontal error and dead-zone correction, and transmit constrained angle commands.
- Integrated a servo motor using the ESP32Servo library, command parsing and angle limits, and built/tested the prototype wiring.
- Adapted Wi-Fi connection management to the controlled development setup.
- Validated face-following servo movement. Separately tested a passive infrared (PIR) sensor trigger, but did not integrate it into the working tracking loop.

OpenCV, the Haar cascade classifier, ESP32Servo and the ESP32 platform libraries are third-party technologies, not original algorithms or libraries authored for this project.

## Redistribution and licenses

- The Seeed repository includes an **MIT license** and its copyright notice; a copy is provided at [`third_party_licenses/SeeedStudio-XIAO-Camera-MIT.txt`](../third_party_licenses/SeeedStudio-XIAO-Camera-MIT.txt).
- The Espressif-derived `app_httpd.cpp` retains the copyright and **Apache License 2.0** header. A copy of the license is provided at [`third_party_licenses/Apache-2.0.txt`](../third_party_licenses/Apache-2.0.txt).
- All third-party code headers and existing attribution statements are kept in their original files; modification notices should be preserved.
- These included licenses cover the applicable **third-party code**. They do **not** mean the entire original turret project is offered under the MIT or Apache licenses. No project-wide license has been selected.
- The package is a historical project snapshot and has not been freshly compiled against a modern ESP32 Arduino core. Check the upstream README for version-specific compatibility warnings before attempting reproduction.

If publishing a modified derivative later, retain copyright/license notices and clearly mark those modifications.
