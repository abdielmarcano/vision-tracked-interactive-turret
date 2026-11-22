# Test evidence, limitations, and future work

## Observed during development

- The ESP32 camera streamed video while serial-based servo-angle commands remained available.
- Manual servo commands were used to verify command reception and physical rotation.
- The host-side Python application detected frontal faces using OpenCV, drew status overlays, and transmitted updated angle commands to the board.
- The complete camera-to-actuator feedback loop was demonstrated on physical hardware and followed a moving face horizontally.
- A passive infrared motion sensor was tested independently with servo response. It was not integrated into the recovered camera-tracking loop.

Four original hardware photographs are in `media/`. An original ~55-second demonstration recording is held outside this repository pending privacy/consent review.

## Deliberate limits of the recovered implementation

- Single horizontal servo axis; no full pan/tilt installation.
- Host-computer face detection instead of on-device inference.
- Largest-face selection rather than persistent subject identity tracking.
- If no face is found, the servo holds its last requested angle rather than performing a search or return-to-center routine.
- No confirmed automatic video reconnection or recovery from serial disconnects.
- Motion-triggered entry and lighting effects are unintegrated concept requirements.
- **No measured accuracy, latency, frame-rate, range, reliability, or cost-savings claims.**

## Code-specific control parameters

| Parameter | Recovered value | Purpose |
|---|---:|---|
| Vision resolution | 320 × 240 pixels | Host detection input |
| Dead-zone magnitude | 35 pixels | Ignore small face-center deviations |
| Left/right gain | 0.040 degrees/pixel | Incremental angle correction |
| Host angle limits | 60°–130° | Restrict requested pan motion |
| Firmware angle limits | 20°–160° | Additional board-side angle constraint |
| Serial speed | 115,200 baud | Host/board command channel |
| Minimum send interval | 0.03 seconds | Rate limit; **not** a measured update rate |
| Center position | 90° | Initial/reset angle |

## Future improvements (not implemented)

- Integrate the passive infrared room-entry trigger and a separate lighting-control circuit.
- Enclose and mechanically stabilize the prop for its intended installation.
- Implement explicit stream/serial reconnection handling and lost-target behavior.
- Move network credentials into a nontracked local configuration and require exact network matching.
- Measure camera frame rates, servo response times, and detection stability using repeatable tests.
- Consider tracking persistence to avoid switching subjects when multiple faces appear.

## Publication review still required

- Confirm source and derivative license requirements for bundled Espressif camera-server web assets.
- Approve all photographs and any future video for public viewing.
- Compile and smoke-test on a specified current hardware/software version before claiming reproducibility.
