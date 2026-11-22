import cv2
import serial
import time

# Replace <ESP32_IP> with the local address printed by the ESP32 at startup.
CAMERA_URL = "http://<ESP32_IP>:81/stream"
SERIAL_PORT = "COM3"
BAUD_RATE = 115200

CENTER_ANGLE = 90

# Tighten these so it cannot swing too far
MIN_ANGLE = 60
MAX_ANGLE = 130

# Tracking tuning
DEAD_ZONE_PIXELS = 35
GAIN_LEFT = 0.040
GAIN_RIGHT = 0.040   # lower because you said right moves too much
SEND_DELAY = 0.03

# If it moves away from you instead of toward you, press i
INVERT_DIRECTION = False

angle = CENTER_ANGLE
last_send_time = 0

print("Opening serial...")
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
time.sleep(2)

print("Opening camera stream...")
cap = cv2.VideoCapture(CAMERA_URL)

if not cap.isOpened():
    print("Could not open camera stream.")
    ser.close()
    raise SystemExit

face_cascade = cv2.CascadeClassifier(
    cv2.data.haarcascades + "haarcascade_frontalface_default.xml"
)

if face_cascade.empty():
    print("Could not load face detector.")
    cap.release()
    ser.close()
    raise SystemExit

def clamp(value, low, high):
    return max(low, min(high, value))

def send_angle(new_angle, force=False):
    global angle, last_send_time

    new_angle = clamp(int(round(new_angle)), MIN_ANGLE, MAX_ANGLE)
    now = time.time()

    if force or (new_angle != angle and now - last_send_time >= SEND_DELAY):
        angle = new_angle
        ser.write(f"{angle}\n".encode())
        last_send_time = now
        print(f"Sent angle: {angle}")

send_angle(CENTER_ANGLE, force=True)

while True:
    ret, frame = cap.read()

    if not ret:
        print("Failed to read frame.")
        time.sleep(0.1)
        continue

    frame = cv2.resize(frame, (320, 240))
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

    faces = face_cascade.detectMultiScale(
        gray,
        scaleFactor=1.1,
        minNeighbors=5,
        minSize=(40, 40)
    )

    frame_center_x = frame.shape[1] // 2

    if len(faces) > 0:
        x, y, w, h = max(faces, key=lambda box: box[2] * box[3])
        face_center_x = x + w // 2

        error = face_center_x - frame_center_x

        if abs(error) > DEAD_ZONE_PIXELS:
            # Negative error = face is left of center
            # Positive error = face is right of center
            if error < 0:
                correction = abs(error) * GAIN_LEFT
                direction = 1
            else:
                correction = abs(error) * GAIN_RIGHT
                direction = -1

            if INVERT_DIRECTION:
                direction *= -1

            send_angle(angle + direction * correction)

        cv2.rectangle(frame, (x, y), (x + w, y + h), (255, 255, 255), 2)
        cv2.circle(frame, (face_center_x, y + h // 2), 4, (255, 255, 255), -1)
        cv2.putText(frame, "FACE LOCK", (10, 45), cv2.FONT_HERSHEY_SIMPLEX, 0.55, (255, 255, 255), 2)

    else:
        cv2.putText(frame, "NO FACE", (10, 45), cv2.FONT_HERSHEY_SIMPLEX, 0.55, (255, 255, 255), 2)

    cv2.line(frame, (frame_center_x, 0), (frame_center_x, 240), (255, 255, 255), 1)

    cv2.putText(
        frame,
        f"Servo angle: {angle}",
        (10, 22),
        cv2.FONT_HERSHEY_SIMPLEX,
        0.6,
        (255, 255, 255),
        2
    )

    cv2.imshow("Turret Face Tracking", frame)

    key = cv2.waitKey(1) & 0xFF

    if key == ord("q"):
        break
    elif key == ord("c"):
        send_angle(CENTER_ANGLE, force=True)
    elif key == ord("i"):
        INVERT_DIRECTION = not INVERT_DIRECTION
        print(f"INVERT_DIRECTION = {INVERT_DIRECTION}")

cap.release()
ser.close()
cv2.destroyAllWindows()
