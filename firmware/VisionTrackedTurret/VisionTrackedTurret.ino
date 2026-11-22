// Attribution: Camera configuration/initialization is based on
// limengdu/SeeedStudio-XIAO-ESP32S3-Sense-camera CameraWebServer_for_esp-arduino_3.0.x
// (MIT), itself adapted from Espressif's Arduino-ESP32 CameraWebServer example.
// See docs/SOURCE_ATTRIBUTION.md and third_party_licenses/ for the exact
// source links, authorship breakdown, and applicable license notices.
// This sketch also contains project-specific Wi-Fi and servo integration.

#include "esp_camera.h"
#include <WiFi.h>
#include <ESP32Servo.h>

#define CAMERA_MODEL_XIAO_ESP32S3 // Has PSRAM
#include "camera_pins.h"

// ===========================
// WiFi credentials
// ===========================
const char *ssid = "...";
const char *password = "...";

// ===========================
// Servo setup
// ===========================
Servo turretServo;

const int SERVO_PIN = D2;     // Servo signal wire goes to XIAO D2
const int MIN_ANGLE = 20;
const int MAX_ANGLE = 160;
const int CENTER_ANGLE = 90;

void startCameraServer();
void setupLedFlash(int pin);

const char *disconnectReasonName(uint8_t reason) {
  switch (reason) {
    case WIFI_REASON_AUTH_EXPIRE: return "auth expired";
    case WIFI_REASON_ASSOC_TOOMANY: return "too many associations";
    case WIFI_REASON_MIC_FAILURE: return "MIC failure / possible wrong password";
    case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT: return "4-way handshake timeout / possible wrong password";
    case WIFI_REASON_BEACON_TIMEOUT: return "beacon timeout";
    case WIFI_REASON_NO_AP_FOUND: return "AP not found";
    case WIFI_REASON_AUTH_FAIL: return "authentication failed / possible wrong password";
    case WIFI_REASON_ASSOC_FAIL: return "association failed";
    case WIFI_REASON_HANDSHAKE_TIMEOUT: return "handshake timeout / possible wrong password";
    default: return "other";
  }
}

void onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
  if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
    Serial.printf("WiFi disconnected, reason %u (%s)\n",
                  info.wifi_sta_disconnected.reason,
                  disconnectReasonName(info.wifi_sta_disconnected.reason));
  }
}

bool networkMatches(const String &scannedName) {
  String scanned = scannedName;
  scanned.trim();

  if (scanned.length() == 0) {
    return false;
  }

  if (scanned.equalsIgnoreCase(ssid) || scanned.indexOf(ssid) >= 0) {
    return true;
  }

  int prefixLen = min((int)scanned.length(), (int)strlen(ssid));
  prefixLen = min(prefixLen, 6);

  if (prefixLen >= 4) {
    return scanned.substring(0, prefixLen).equalsIgnoreCase(String(ssid).substring(0, prefixLen));
  }

  return false;
}

bool connectToHotspot() {
  Serial.println("Scanning for hotspot...");

  int n = WiFi.scanNetworks();
  int foundIndex = -1;
  int bestRssi = -1000;

  for (int i = 0; i < n; i++) {
    String scannedSsid = WiFi.SSID(i);
    int rssi = WiFi.RSSI(i);

    Serial.printf("  %s  RSSI: %d\n", scannedSsid.c_str(), rssi);

    if (networkMatches(scannedSsid) && rssi > bestRssi) {
      foundIndex = i;
      bestRssi = rssi;
    }
  }

  if (foundIndex == -1) {
    Serial.println("Hotspot not found. Keep iPhone on Settings -> Personal Hotspot.");
    return false;
  }

  String scannedSsid = WiFi.SSID(foundIndex);
  int channel = WiFi.channel(foundIndex);
  uint8_t *bssid = WiFi.BSSID(foundIndex);

  Serial.printf("Found hotspot on channel %d, connecting...\n", channel);

  WiFi.disconnect(true, true);
  delay(500);

  WiFi.begin(scannedSsid.c_str(), password, channel, bssid);

  for (int i = 0; i < 30; i++) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println();
      return true;
    }

    delay(500);
    Serial.print(".");
  }

  Serial.println();
  return false;
}

bool initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  if (psramFound()) {
    config.jpeg_quality = 10;
    config.fb_count = 2;
    config.grab_mode = CAMERA_GRAB_LATEST;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.fb_location = CAMERA_FB_IN_DRAM;
  }

  esp_err_t err = esp_camera_init(&config);

  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return false;
  }

  sensor_t *s = esp_camera_sensor_get();

  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);
    s->set_brightness(s, 1);
    s->set_saturation(s, -2);
  }

  s->set_framesize(s, FRAMESIZE_QVGA);

#if defined(LED_GPIO_NUM)
  setupLedFlash(LED_GPIO_NUM);
#endif

  return true;
}

void initServo() {
  // Avoid touching timer 0, since the camera uses LEDC timer/channel resources.
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  turretServo.setPeriodHertz(50);
  turretServo.attach(SERVO_PIN, 500, 2500);

  setTurretAngle(CENTER_ANGLE);
}

void setTurretAngle(int angle) {
  angle = constrain(angle, MIN_ANGLE, MAX_ANGLE);
  turretServo.write(angle);

  Serial.print("Servo angle set to: ");
  Serial.println(angle);
}

void checkSerialServoCommand() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command.length() == 0) {
      return;
    }

    int angle = command.toInt();

    if (angle == 0 && command != "0") {
      Serial.print("Invalid command: ");
      Serial.println(command);
      Serial.println("Type an angle like 45, 90, or 135.");
      return;
    }

    setTurretAngle(angle);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.setTimeout(50);
  delay(2000);
  Serial.println();

  if (!initCamera()) {
    return;
  }

  Serial.println("Keep your iPhone on Settings -> Personal Hotspot while connecting.");
  Serial.printf("Looking for: %s\n", ssid);

  WiFi.onEvent(onWiFiEvent);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.disconnect(true, true);
  delay(1000);

  bool connected = false;

  for (int attempt = 1; attempt <= 5 && !connected; attempt++) {
    Serial.printf("Attempt %d\n", attempt);
    connected = connectToHotspot();

    if (!connected) {
      Serial.println("Retrying in 3 seconds...");
      delay(3000);
    }
  }

  if (!connected) {
    Serial.println("FAILED TO CONNECT");
    Serial.println("Check: Allow Others to Join is ON, Maximize Compatibility is ON,");
    Serial.println("the Personal Hotspot screen is open, and the password matches.");
    return;
  }

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");

  Serial.println("Starting servo...");
  initServo();

  Serial.println();
  Serial.println("Camera + servo test ready.");
  Serial.println("Open the camera page, click Start Stream, then type angles here:");
  Serial.println("45");
  Serial.println("90");
  Serial.println("135");
}

void loop() {
  checkSerialServoCommand();
  delay(10);
}
