/*
  WALL-E AI Body v4.1 - Complete with Emotional Face Engine
  ==========================================================
  Board: XIAO ESP32S3 Sense

  NEW in v4.1:
    - 16 emotional faces with animated eyes
    - Blinking, eye tracking, smooth transitions
    - Top/Bottom status bars (battery, WiFi, state, distance)
    - Speech bubbles on screen
    - Boot animation sequence
    - Sleep breathing animation
    - Screen fully utilized for personality
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoOTA.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <Adafruit_VL53L0X.h>
#include <Adafruit_INA219.h>
#include <TFT_eSPI.h>
#include <esp_camera.h>
#include <SD_MMC.h>
#include <FS.h>
#include <driver/i2s.h>

// ==================== CONFIG ====================
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* OPENAI_API_KEY = "sk-YOUR_OPENAI_API_KEY_HERE";
const char* OPENAI_CHAT_URL = "https://api.openai.com/v1/chat/completions";

#define I2C_SDA D0
#define I2C_SCL D1

#define PAN_CH 0
#define TILT_CH 1
#define L_AIN1 2
#define L_AIN2 3
#define R_BIN1 4
#define R_BIN2 5
#define LEFT_ARM_CH 6
#define RIGHT_ARM_CH 7

#define SERVO_MIN_PULSE 150
#define SERVO_MAX_PULSE 600
#define ARM_DOWN 45
#define ARM_HOME 90
#define ARM_UP 135
#define HEAD_MIN 30
#define HEAD_MAX 150

#define SAFE_DISTANCE_MM 200
#define BATTERY_MIN_PCT 10.0f

#define SENSOR_INTERVAL_MS 50
#define SAFETY_INTERVAL_MS 100
#define ANIMATION_INTERVAL_MS 20
#define BATTERY_INTERVAL_MS 5000
#define AUTONOMY_INTERVAL_MS 2000
#define VISION_INTERVAL_MS 10000
#define MEMORY_SYNC_INTERVAL_MS 30000
#define FACE_UPDATE_INTERVAL_MS 50

#define I2S_WS D7
#define I2S_SD D6
#define I2S_SCK D5
#define I2S_PORT I2S_NUM_0
#define I2S_SAMPLE_RATE 16000
#define I2S_BUFFER_SIZE 1024

#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 15
#define SIOD_GPIO_NUM 4
#define SIOC_GPIO_NUM 5
#define Y9_GPIO_NUM 16
#define Y8_GPIO_NUM 17
#define Y7_GPIO_NUM 18
#define Y6_GPIO_NUM 12
#define Y5_GPIO_NUM 10
#define Y4_GPIO_NUM 8
#define Y3_GPIO_NUM 9
#define Y2_GPIO_NUM 11
#define VSYNC_GPIO_NUM 6
#define HREF_GPIO_NUM 7
#define PCLK_GPIO_NUM 13

// ==================== FACE ENGINE ====================
#define FACE_CX 120
#define FACE_CY 102
#define EYE_RX 30
#define EYE_RY 40
#define EYE_GAP 26
#define PUPIL_R 10
#define MOUTH_Y 162
#define TOP_BAR_H 20
#define BOTTOM_BAR_H 16
#define FACE_AREA_Y TOP_BAR_H
#define FACE_AREA_H (240 - TOP_BAR_H - BOTTOM_BAR_H)
#define SPEECH_Y 195
#define SPEECH_MAX_W 200

#define FACE_BG TFT_BLACK
#define EYE_WHITE 0xE71C
#define PUPIL_COLOR TFT_BLACK
#define MOUTH_COLOR 0x79E0
#define BLUSH_COLOR 0xF809
#define ACCENT_COLOR 0xF4D03F
#define BAR_BG 0x1082

enum Emotion {
  EM_NEUTRAL, EM_HAPPY, EM_SAD, EM_ANGRY, EM_SURPRISED,
  EM_SLEEPY, EM_CONFUSED, EM_LOVE, EM_DEAD, EM_THINKING,
  EM_EXCITED, EM_WORRIED, EM_COOL, EM_SHY, EM_DETERMINED, EM_CURIOUS
};

// ==================== GLOBALS ====================
Adafruit_PWMServoDriver pwm(0x40);
Adafruit_VL53L0X lox;
Adafruit_INA219 ina219(0x41);
TFT_eSPI tft;
WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
HTTPClient http;

enum RobotState { IDLE, MOVING, WAVING, ARMS_UP, AVOIDING, ERROR_STATE, SLEEPING, LISTENING, THINKING, PATROLLING, EXPLORING, INTERACTING, FOLLOWING };
RobotState robotState = IDLE;

enum AnimState { ANIM_NONE, ANIM_WAVE, ANIM_ARMS_UP_ANIM, ANIM_ARMS_DOWN, ANIM_LOOK_LEFT, ANIM_LOOK_RIGHT, ANIM_DANCE, ANIM_GREET, ANIM_THINK, ANIM_CONFUSED, ANIM_HAPPY };
AnimState currentAnim = ANIM_NONE;
int animStep = 0;
unsigned long animStartTime = 0;

unsigned long lastSensorUpdate = 0, lastSafetyCheck = 0, lastAnimationStep = 0;
unsigned long lastBatteryUpdate = 0, lastAutonomyUpdate = 0, lastVisionUpdate = 0;
unsigned long lastMemorySync = 0, lastActivityTime = 0, sleepEnterTime = 0, lastSpeakTime = 0;
unsigned long lastFaceUpdate = 0;

float batteryVoltage = 0, batteryPercent = 0, batteryCurrent = 0;
uint16_t distanceMm = 9999;
bool obstacleDetected = false, lowBattery = false;

int16_t audioBuffer[I2S_BUFFER_SIZE];
bool isRecording = false;
unsigned long recordingStart = 0;
String lastTranscription = "", lastAIResponse = "";

String lastSceneDescription = "", lastPersonDetected = "";
bool personInView = false;
unsigned long lastPersonSeenTime = 0;

struct PersonMemory { String name, description, preferences, lastGreeting; unsigned long lastSeen; int encounterCount; };
struct PlaceMemory { String name, description; float dangerLevel; unsigned long lastVisited; int visitCount; };
struct EventMemory { String timestamp, event, location, people; };

#define MAX_PEOPLE 20
#define MAX_PLACES 20
#define MAX_EVENTS 50

PersonMemory peopleMem[MAX_PEOPLE];
PlaceMemory placesMem[MAX_PLACES];
EventMemory eventsMem[MAX_EVENTS];
int peopleCount = 0, placesCount = 0, eventsCount = 0;

String robotName = "WALL-E", ownerName = "", currentLocation = "home";

enum GoalType { GOAL_NONE, GOAL_PATROL, GOAL_EXPLORE, GOAL_GREET, GOAL_FOLLOW, GOAL_REST };
GoalType currentGoal = GOAL_NONE;
unsigned long goalStartTime = 0;

// Face engine globals
Emotion currentEmotion = EM_NEUTRAL;
unsigned long lastBlinkTime = 0;
unsigned long nextBlinkInterval = 3000;
bool isBlinking = false;
int blinkPhase = 0;
float eyeOffsetX = 0, eyeOffsetY = 0;
float targetEyeOffsetX = 0, targetEyeOffsetY = 0;
String currentSpeechText = "";
unsigned long speechDisplayTime = 0;
bool faceNeedsRedraw = true;
Emotion lastDrawnEmotion = EM_NEUTRAL;
float lastDrawnEyeX = 999, lastDrawnEyeY = 999;
int lastDrawnBlink = -1;

// ==================== SETUP & LOOP ====================
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== WALL-E AI Brain v4.1 Boot ===");

  initDisplay();
  playBootAnimation();

  initI2C();
  initSD();
  initPCA9685();
  initServos();
  initMotors();
  initToF();
  initINA219();
  initCamera();
  initAudio();
  initWiFi();
  initWebServer();
  initWebSocket();
  initOTA();
  loadAllMemory();

  lastActivityTime = millis();
  Serial.println("=== WALL-E is ALIVE ===");
  setEmotion(EM_HAPPY);
  showSpeech("Hello! I am ready!");
  drawTopBar();
  drawBottomBar();
}

void loop() {
  unsigned long now = millis();

  server.handleClient();
  webSocket.loop();
  ArduinoOTA.handle();

  if (robotState == SLEEPING) {
    handleSleepMode(now);
    drawSleepBreathing(now);
    return;
  }

  if (now - lastSensorUpdate >= SENSOR_INTERVAL_MS) { updateSensors(); lastSensorUpdate = now; }
  if (now - lastSafetyCheck >= SAFETY_INTERVAL_MS) { safetyCheck(); lastSafetyCheck = now; }
  if (now - lastAnimationStep >= ANIMATION_INTERVAL_MS) { updateAnimations(now); lastAnimationStep = now; }
  if (now - lastBatteryUpdate >= BATTERY_INTERVAL_MS) { reportBattery(); drawTopBar(); drawBottomBar(); lastBatteryUpdate = now; }
  if (now - lastAutonomyUpdate >= AUTONOMY_INTERVAL_MS) { autonomyEngine(); lastAutonomyUpdate = now; }
  if (now - lastVisionUpdate >= VISION_INTERVAL_MS) { processVision(); lastVisionUpdate = now; }
  if (now - lastMemorySync >= MEMORY_SYNC_INTERVAL_MS) { saveAllMemory(); lastMemorySync = now; }
  if (now - lastFaceUpdate >= FACE_UPDATE_INTERVAL_MS) { updateFaceEngine(now); lastFaceUpdate = now; }

  if (now - lastActivityTime > 300000 && robotState == IDLE && currentGoal == GOAL_NONE) { enterSleep(); }

  // Clear old speech after 3 seconds
  if (currentSpeechText != "" && now - speechDisplayTime > 3000) {
    clearSpeech();
    currentSpeechText = "";
  }
}

// ==================== INIT FUNCTIONS ====================
void initDisplay() {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
}

void initI2C() {
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000);
  delay(100);
}

void initSD() {
  if (!SD_MMC.begin()) {
    Serial.println("SD FAIL");
  } else {
    if (!SD_MMC.exists("/memory")) SD_MMC.mkdir("/memory");
  }
}

void initPCA9685() {
  if (!pwm.begin()) {
    Serial.println("PCA9685 FAIL");
    robotState = ERROR_STATE;
    return;
  }
  pwm.setPWMFreq(50);
}

void initServos() {
  setServoAngle(PAN_CH, 90);
  setServoAngle(TILT_CH, 90);
  armsHome();
}

void initMotors() { stopMotors(); }

void initToF() {
  if (!lox.begin(0x29)) Serial.println("ToF FAIL");
}

void initINA219() {
  if (!ina219.begin()) Serial.println("INA219 FAIL");
}

void initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 12;
  config.fb_count = 2;
  config.grab_mode = CAMERA_GRAB_LATEST;
  if (esp_camera_init(&config) != ESP_OK) Serial.println("Camera FAIL");
}

void initAudio() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = I2S_SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = I2S_BUFFER_SIZE,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };
  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };
  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);
  i2s_set_clk(I2S_PORT, I2S_SAMPLE_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
}

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 30) { delay(500); retries++; }
}

void initWebServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/stream", HTTP_GET, handleStream);
  server.on("/api/move", HTTP_POST, handleMove);
  server.on("/api/stop", HTTP_POST, handleStop);
  server.on("/api/wave", HTTP_POST, handleWave);
  server.on("/api/armsup", HTTP_POST, handleArmsUp);
  server.on("/api/armshome", HTTP_POST, handleArmsHome);
  server.on("/api/look", HTTP_POST, handleLook);
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/sleep", HTTP_POST, handleSleepRequest);
  server.on("/api/wakeup", HTTP_POST, handleWakeRequest);
  server.on("/api/speak", HTTP_POST, handleSpeak);
  server.on("/api/ask", HTTP_POST, handleAsk);
  server.on("/api/vision", HTTP_GET, handleVisionRequest);
  server.on("/api/memory/people", HTTP_GET, handleGetPeople);
  server.on("/api/memory/places", HTTP_GET, handleGetPlaces);
  server.on("/api/memory/events", HTTP_GET, handleGetEvents);
  server.on("/api/memory/addperson", HTTP_POST, handleAddPerson);
  server.on("/api/memory/addplace", HTTP_POST, handleAddPlace);
  server.on("/api/memory/clear", HTTP_POST, handleClearMemory);
  server.begin();
}

void initWebSocket() {
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void initOTA() {
  ArduinoOTA.setHostname("walle-ai");
  ArduinoOTA.begin();
}

// ==================== HARDWARE CONTROL ====================
void setServoAngle(uint8_t ch, int angle) {
  angle = constrain(angle, 0, 180);
  int pulse = map(angle, 0, 180, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
  pwm.setPWM(ch, 0, pulse);
}

void setArmAngleSafe(uint8_t ch, int angle) {
  angle = constrain(angle, ARM_DOWN, ARM_UP);
  setServoAngle(ch, angle);
}

void setHeadAngleSafe(uint8_t ch, int angle) {
  angle = constrain(angle, HEAD_MIN, HEAD_MAX);
  setServoAngle(ch, angle);
}

void armsHome() {
  setArmAngleSafe(LEFT_ARM_CH, ARM_HOME);
  setArmAngleSafe(RIGHT_ARM_CH, ARM_HOME);
}

void driveMotor(bool left, int8_t direction, uint16_t speed) {
  uint8_t inA = left ? L_AIN1 : R_BIN1;
  uint8_t inB = left ? L_AIN2 : R_BIN2;
  speed = constrain(speed, 0, 4095);
  if (direction > 0) { pwm.setPWM(inA, 0, speed); pwm.setPWM(inB, 0, 0); }
  else if (direction < 0) { pwm.setPWM(inA, 0, 0); pwm.setPWM(inB, 0, speed); }
  else { pwm.setPWM(inA, 0, 0); pwm.setPWM(inB, 0, 0); }
}

void stopMotors() { driveMotor(true, 0, 0); driveMotor(false, 0, 0); }

void emergencyStop() {
  stopMotors();
  armsHome();
  setServoAngle(PAN_CH, 90);
  setServoAngle(TILT_CH, 90);
  robotState = IDLE;
  currentAnim = ANIM_NONE;
  currentGoal = GOAL_NONE;
  broadcastState();
}

// ==================== FACE ENGINE ====================
void setEmotion(Emotion em) {
  if (currentEmotion == em) return;
  currentEmotion = em;
  faceNeedsRedraw = true;
}

void lookAt(int x, int y) {
  targetEyeOffsetX = constrain(x * 0.25, -12, 12);
  targetEyeOffsetY = constrain(y * 0.25, -12, 12);
}

void lookCenter() { targetEyeOffsetX = 0; targetEyeOffsetY = 0; }
void lookLeft() { targetEyeOffsetX = -10; }
void lookRight() { targetEyeOffsetX = 10; }
void lookUp() { targetEyeOffsetY = -10; }
void lookDown() { targetEyeOffsetY = 10; }

void updateFaceEngine(unsigned long now) {
  // Smooth eye movement
  eyeOffsetX += (targetEyeOffsetX - eyeOffsetX) * 0.12;
  eyeOffsetY += (targetEyeOffsetY - eyeOffsetY) * 0.12;

  // Blinking
  if (!isBlinking && now - lastBlinkTime > nextBlinkInterval) {
    isBlinking = true;
    blinkPhase = 1;
    lastBlinkTime = now;
    nextBlinkInterval = random(2000, 5000);
    faceNeedsRedraw = true;
  }
  if (isBlinking) {
    if (blinkPhase == 1 && now - lastBlinkTime > 60) { blinkPhase = 2; lastBlinkTime = now; faceNeedsRedraw = true; }
    else if (blinkPhase == 2 && now - lastBlinkTime > 40) { blinkPhase = 3; lastBlinkTime = now; faceNeedsRedraw = true; }
    else if (blinkPhase == 3 && now - lastBlinkTime > 60) { blinkPhase = 0; isBlinking = false; faceNeedsRedraw = true; }
  }

  // Redraw only if needed
  if (faceNeedsRedraw || abs(eyeOffsetX - lastDrawnEyeX) > 0.5 || abs(eyeOffsetY - lastDrawnEyeY) > 0.5 ||
      currentEmotion != lastDrawnEmotion || blinkPhase != lastDrawnBlink) {
    drawFace(currentEmotion);
    lastDrawnEmotion = currentEmotion;
    lastDrawnEyeX = eyeOffsetX;
    lastDrawnEyeY = eyeOffsetY;
    lastDrawnBlink = blinkPhase;
    faceNeedsRedraw = false;
  }
}

void drawFace(Emotion em) {
  tft.fillRect(0, FACE_AREA_Y, 240, FACE_AREA_H, FACE_BG);
  int leftX = FACE_CX - EYE_RX - EYE_GAP/2;
  int rightX = FACE_CX + EYE_RX + EYE_GAP/2;

  switch (em) {
    case EM_NEUTRAL: drawNeutralFace(leftX, rightX); break;
    case EM_HAPPY: drawHappyFace(leftX, rightX); break;
    case EM_SAD: drawSadFace(leftX, rightX); break;
    case EM_ANGRY: drawAngryFace(leftX, rightX); break;
    case EM_SURPRISED: drawSurprisedFace(leftX, rightX); break;
    case EM_SLEEPY: drawSleepyFace(leftX, rightX); break;
    case EM_CONFUSED: drawConfusedFace(leftX, rightX); break;
    case EM_LOVE: drawLoveFace(leftX, rightX); break;
    case EM_DEAD: drawDeadFace(leftX, rightX); break;
    case EM_THINKING: drawThinkingFace(leftX, rightX); break;
    case EM_EXCITED: drawExcitedFace(leftX, rightX); break;
    case EM_WORRIED: drawWorriedFace(leftX, rightX); break;
    case EM_COOL: drawCoolFace(leftX, rightX); break;
    case EM_SHY: drawShyFace(leftX, rightX); break;
    case EM_DETERMINED: drawDeterminedFace(leftX, rightX); break;
    case EM_CURIOUS: drawCuriousFace(leftX, rightX); break;
  }

  // Redraw speech if active
  if (currentSpeechText != "" && millis() - speechDisplayTime <= 3000) {
    showSpeech(currentSpeechText);
  }
}

void drawEyeBase(int cx, int cy) {
  if (isBlinking && blinkPhase > 0) {
    int ry = EYE_RY;
    if (blinkPhase == 1) ry = EYE_RY * 3 / 4;
    else if (blinkPhase == 2) ry = 2;
    else if (blinkPhase == 3) ry = EYE_RY * 3 / 4;
    if (ry < 3) {
      tft.drawLine(cx - EYE_RX, cy, cx + EYE_RX, cy, 0x4208);
      tft.drawLine(cx - EYE_RX, cy+1, cx + EYE_RX, cy+1, 0x4208);
    } else {
      tft.fillEllipse(cx, cy, EYE_RX, ry, EYE_WHITE);
      tft.drawEllipse(cx, cy, EYE_RX, ry, 0x4208);
    }
  } else {
    tft.fillEllipse(cx, cy, EYE_RX, EYE_RY, EYE_WHITE);
    tft.drawEllipse(cx, cy, EYE_RX, EYE_RY, 0x4208);
  }
}

void drawPupil(int cx, int cy) {
  if (isBlinking && blinkPhase == 2) return;
  int px = cx + (int)eyeOffsetX;
  int py = cy + (int)eyeOffsetY;
  float dx = px - cx, dy = py - cy;
  float dist = sqrt(dx*dx + dy*dy);
  float maxDist = (EYE_RX - PUPIL_R - 3) * 0.5;
  if (dist > maxDist) {
    px = cx + (int)(dx * maxDist / dist);
    py = cy + (int)(dy * maxDist / dist);
  }
  tft.fillCircle(px, py, PUPIL_R, PUPIL_COLOR);
  tft.fillCircle(px - 3, py - 3, 3, TFT_WHITE);
}

void drawNeutralFace(int leftX, int rightX) {
  drawEyeBase(leftX, FACE_CY);
  drawEyeBase(rightX, FACE_CY);
  drawPupil(leftX, FACE_CY);
  drawPupil(rightX, FACE_CY);
  tft.drawLine(FACE_CX - 18, MOUTH_Y, FACE_CX + 18, MOUTH_Y, MOUTH_COLOR);
  tft.drawLine(FACE_CX - 18, MOUTH_Y+1, FACE_CX + 18, MOUTH_Y+1, MOUTH_COLOR);
}

void drawHappyFace(int leftX, int rightX) {
  drawEyeBase(leftX, FACE_CY - 4);
  drawEyeBase(rightX, FACE_CY - 4);
  drawPupil(leftX, FACE_CY - 4);
  drawPupil(rightX, FACE_CY - 4);
  for (int i = 0; i < 4; i++) tft.drawArc(FACE_CX, MOUTH_Y - 8, 22, 30, 205+i*2, 335-i*2, MOUTH_COLOR);
  tft.fillCircle(leftX - 22, FACE_CY + 18, 5, BLUSH_COLOR);
  tft.fillCircle(rightX + 22, FACE_CY + 18, 5, BLUSH_COLOR);
}

void drawSadFace(int leftX, int rightX) {
  drawEyeBase(leftX, FACE_CY + 4);
  drawEyeBase(rightX, FACE_CY + 4);
  drawPupil(leftX, FACE_CY + 4);
  drawPupil(rightX, FACE_CY + 4);
  for (int i = 0; i < 3; i++) tft.drawArc(FACE_CX, MOUTH_Y + 18, 18, 25, 25+i*2, 155-i*2, MOUTH_COLOR);
  tft.fillCircle(rightX + 12, FACE_CY + 22, 3, 0x5DDF);
}

void drawAngryFace(int leftX, int rightX) {
  tft.drawLine(leftX - 22, FACE_CY - 32, leftX + 12, FACE_CY - 18, 0x4208);
  tft.drawLine(rightX + 22, FACE_CY - 32, rightX - 12, FACE_CY - 18, 0x4208);
  drawEyeBase(leftX, FACE_CY);
  drawEyeBase(rightX, FACE_CY);
  drawPupil(leftX, FACE_CY);
  drawPupil(rightX, FACE_CY);
  tft.drawLine(FACE_CX - 22, MOUTH_Y, FACE_CX + 22, MOUTH_Y, MOUTH_COLOR);
  tft.drawLine(FACE_CX - 22, MOUTH_Y+1, FACE_CX + 22, MOUTH_Y+1, MOUTH_COLOR);
  tft.drawLine(FACE_CX - 22, MOUTH_Y+2, FACE_CX + 22, MOUTH_Y+2, MOUTH_COLOR);
}

void drawSurprisedFace(int leftX, int rightX) {
  drawEyeBase(leftX, FACE_CY - 8);
  drawEyeBase(rightX, FACE_CY - 8);
  drawPupil(leftX, FACE_CY - 8);
  drawPupil(rightX, FACE_CY - 8);
  tft.fillEllipse(FACE_CX, MOUTH_Y, 9, 13, MOUTH_COLOR);
  tft.drawArc(leftX, FACE_CY - 36, 18, 22, 200, 340, 0x4208);
  tft.drawArc(rightX, FACE_CY - 36, 18, 22, 200, 340, 0x4208);
}

void drawSleepyFace(int leftX, int rightX) {
  tft.drawLine(leftX - EYE_RX, FACE_CY, leftX + EYE_RX, FACE_CY, EYE_WHITE);
  tft.drawLine(leftX - EYE_RX, FACE_CY+1, leftX + EYE_RX, FACE_CY+1, EYE_WHITE);
  tft.drawLine(rightX - EYE_RX, FACE_CY, rightX + EYE_RX, FACE_CY, EYE_WHITE);
  tft.drawLine(rightX - EYE_RX, FACE_CY+1, rightX + EYE_RX, FACE_CY+1, EYE_WHITE);
  tft.drawLine(FACE_CX - 6, MOUTH_Y, FACE_CX + 6, MOUTH_Y, MOUTH_COLOR);
  int zOff = (millis() / 500) % 3;
  tft.setTextColor(0x867D, FACE_BG);
  tft.setTextSize(2);
  tft.setCursor(rightX + 25, FACE_CY - 18 - zOff*10);
  tft.print("Z");
  if (zOff > 0) { tft.setCursor(rightX + 38, FACE_CY - 28 - zOff*10); tft.print("z"); }
}

void drawConfusedFace(int leftX, int rightX) {
  drawEyeBase(leftX, FACE_CY);
  drawEyeBase(rightX, FACE_CY);
  int px1 = leftX + (int)eyeOffsetX - 5;
  int py1 = FACE_CY + (int)eyeOffsetY - 6;
  int px2 = rightX + (int)eyeOffsetX + 5;
  int py2 = FACE_CY + (int)eyeOffsetY + 4;
  tft.fillCircle(px1, py1, PUPIL_R, PUPIL_COLOR);
  tft.fillCircle(px2, py2, PUPIL_R, PUPIL_COLOR);
  tft.fillCircle(px1 - 3, py1 - 3, 3, TFT_WHITE);
  tft.fillCircle(px2 - 3, py2 - 3, 3, TFT_WHITE);
  tft.drawLine(FACE_CX - 4, MOUTH_Y, FACE_CX + 4, MOUTH_Y, MOUTH_COLOR);
  tft.drawLine(FACE_CX + 4, MOUTH_Y, FACE_CX + 4, MOUTH_Y - 6, MOUTH_COLOR);
  tft.drawLine(FACE_CX + 4, MOUTH_Y - 6, FACE_CX - 2, MOUTH_Y - 6, MOUTH_COLOR);
  tft.fillCircle(FACE_CX, MOUTH_Y + 5, 2, MOUTH_COLOR);
  tft.setTextColor(ACCENT_COLOR, FACE_BG);
  tft.setTextSize(1);
  tft.setCursor(leftX - 25, FACE_CY - 28);
  tft.print("?");
  tft.setCursor(rightX + 18, FACE_CY - 24);
  tft.print("?");
}

void drawLoveFace(int leftX, int rightX) {
  drawHeart(leftX, FACE_CY, 16, BLUSH_COLOR);
  drawHeart(rightX, FACE_CY, 16, BLUSH_COLOR);
  for (int i = 0; i < 3; i++) tft.drawArc(FACE_CX, MOUTH_Y - 6, 18, 25, 210+i*2, 330-i*2, MOUTH_COLOR);
  int hOff = (millis() / 300) % 4;
  tft.setTextColor(BLUSH_COLOR, FACE_BG);
  tft.setTextSize(1);
  int hx[4] = {leftX-30, rightX+25, leftX-18, rightX+18};
  int hy[4] = {FACE_CY-32, FACE_CY-36, FACE_CY-46, FACE_CY-42};
  for (int i = 0; i <= hOff && i < 4; i++) {
    tft.setCursor(hx[i], hy[i]);
    tft.print("\x03");
  }
}

void drawHeart(int cx, int cy, int size, uint16_t color) {
  int r = size / 2;
  tft.fillCircle(cx - r/2, cy - r/3, r/2, color);
  tft.fillCircle(cx + r/2, cy - r/3, r/2, color);
  tft.fillTriangle(cx - r, cy - r/3, cx + r, cy - r/3, cx, cy + r, color);
}

void drawDeadFace(int leftX, int rightX) {
  tft.drawLine(leftX - 14, FACE_CY - 14, leftX + 14, FACE_CY + 14, 0x4208);
  tft.drawLine(leftX + 14, FACE_CY - 14, leftX - 14, FACE_CY + 14, 0x4208);
  tft.drawLine(rightX - 14, FACE_CY - 14, rightX + 14, FACE_CY + 14, 0x4208);
  tft.drawLine(rightX + 14, FACE_CY - 14, rightX - 14, FACE_CY + 14, 0x4208);
  tft.drawLine(FACE_CX - 14, MOUTH_Y, FACE_CX + 14, MOUTH_Y, MOUTH_COLOR);
  drawBatteryIcon(195, 4, 0);
}

void drawThinkingFace(int leftX, int rightX) {
  drawEyeBase(leftX, FACE_CY);
  drawEyeBase(rightX, FACE_CY);
  int py = FACE_CY + (int)eyeOffsetY - 10;
  tft.fillCircle(leftX + (int)eyeOffsetX, py, PUPIL_R, PUPIL_COLOR);
  tft.fillCircle(rightX + (int)eyeOffsetX, py, PUPIL_R, PUPIL_COLOR);
  tft.fillCircle(leftX + (int)eyeOffsetX - 3, py - 3, 3, TFT_WHITE);
  tft.fillCircle(rightX + (int)eyeOffsetX - 3, py - 3, 3, TFT_WHITE);
  tft.fillCircle(FACE_CX, MOUTH_Y, 4, MOUTH_COLOR);
  tft.fillCircle(rightX + 32, FACE_CY - 26, 4, 0xC618);
  tft.fillCircle(rightX + 40, FACE_CY - 36, 6, 0xC618);
  tft.fillCircle(rightX + 48, FACE_CY - 48, 10, 0xC618);
  tft.setTextColor(TFT_BLACK, 0xC618);
  tft.setTextSize(1);
  tft.setCursor(rightX + 42, FACE_CY - 52);
  tft.print("...");
}

void drawExcitedFace(int leftX, int rightX) {
  drawStar(leftX, FACE_CY, 18, ACCENT_COLOR);
  drawStar(rightX, FACE_CY, 18, ACCENT_COLOR);
  for (int i = 0; i < 5; i++) tft.drawArc(FACE_CX, MOUTH_Y - 12, 26, 35, 200+i*2, 340-i*2, MOUTH_COLOR);
  tft.fillEllipse(FACE_CX, MOUTH_Y + 4, 7, 9, 0x9000);
}

void drawStar(int cx, int cy, int r, uint16_t color) {
  for (int i = 0; i < 5; i++) {
    float a1 = (i * 72 - 90) * PI / 180;
    float a2 = ((i+1) * 72 - 90) * PI / 180;
    float am = ((i * 72 + 36) - 90) * PI / 180;
    int x1 = cx + r * cos(a1) * 0.4;
    int y1 = cy + r * sin(a1) * 0.4;
    int x2 = cx + r * cos(am);
    int y2 = cy + r * sin(am);
    int x3 = cx + r * cos(a2) * 0.4;
    int y3 = cy + r * sin(a2) * 0.4;
    tft.fillTriangle(x1, y1, x2, y2, x3, y3, color);
  }
  tft.fillCircle(cx, cy, r * 0.3, color);
}

void drawWorriedFace(int leftX, int rightX) {
  tft.drawLine(leftX - 12, FACE_CY - 22, leftX + 22, FACE_CY - 32, 0x4208);
  tft.drawLine(rightX + 12, FACE_CY - 22, rightX - 22, FACE_CY - 32, 0x4208);
  drawEyeBase(leftX, FACE_CY);
  drawEyeBase(rightX, FACE_CY);
  drawPupil(leftX, FACE_CY);
  drawPupil(rightX, FACE_CY);
  tft.drawLine(FACE_CX - 18, MOUTH_Y, FACE_CX - 4, MOUTH_Y + 3, MOUTH_COLOR);
  tft.drawLine(FACE_CX - 4, MOUTH_Y + 3, FACE_CX + 4, MOUTH_Y - 2, MOUTH_COLOR);
  tft.drawLine(FACE_CX + 4, MOUTH_Y - 2, FACE_CX + 18, MOUTH_Y, MOUTH_COLOR);
}

void drawCoolFace(int leftX, int rightX) {
  tft.fillRoundRect(leftX - EYE_RX - 4, FACE_CY - 13, EYE_RX * 2 + 8, 26, 4, TFT_BLACK);
  tft.fillRoundRect(rightX - EYE_RX - 4, FACE_CY - 13, EYE_RX * 2 + 8, 26, 4, TFT_BLACK);
  tft.fillRect(FACE_CX - 4, FACE_CY - 4, 8, 8, TFT_BLACK);
  tft.drawLine(leftX - 8, FACE_CY - 6, leftX + 4, FACE_CY - 6, 0xC618);
  tft.drawLine(rightX - 8, FACE_CY - 6, rightX + 4, FACE_CY - 6, 0xC618);
  tft.drawLine(FACE_CX - 12, MOUTH_Y, FACE_CX + 4, MOUTH_Y, MOUTH_COLOR);
  tft.drawLine(FACE_CX + 4, MOUTH_Y, FACE_CX + 18, MOUTH_Y - 4, MOUTH_COLOR);
}

void drawShyFace(int leftX, int rightX) {
  drawEyeBase(leftX, FACE_CY);
  drawEyeBase(rightX, FACE_CY);
  tft.fillCircle(leftX + (int)eyeOffsetX - 7, FACE_CY + (int)eyeOffsetY + 6, PUPIL_R, PUPIL_COLOR);
  tft.fillCircle(rightX + (int)eyeOffsetX + 7, FACE_CY + (int)eyeOffsetY + 6, PUPIL_R, PUPIL_COLOR);
  for (int i = 0; i < 3; i++) tft.drawArc(FACE_CX, MOUTH_Y - 4, 14, 18, 215+i*2, 325-i*2, MOUTH_COLOR);
  tft.fillCircle(leftX - 22, FACE_CY + 12, 9, BLUSH_COLOR);
  tft.fillCircle(rightX + 22, FACE_CY + 12, 9, BLUSH_COLOR);
}

void drawDeterminedFace(int leftX, int rightX) {
  tft.drawLine(leftX - 18, FACE_CY - 28, leftX + 18, FACE_CY - 22, 0x4208);
  tft.drawLine(rightX + 18, FACE_CY - 28, rightX - 18, FACE_CY - 22, 0x4208);
  drawEyeBase(leftX, FACE_CY);
  drawEyeBase(rightX, FACE_CY);
  tft.fillCircle(leftX, FACE_CY, PUPIL_R, PUPIL_COLOR);
  tft.fillCircle(rightX, FACE_CY, PUPIL_R, PUPIL_COLOR);
  tft.fillCircle(leftX - 3, FACE_CY - 3, 3, TFT_WHITE);
  tft.fillCircle(rightX - 3, FACE_CY - 3, 3, TFT_WHITE);
  tft.drawLine(FACE_CX - 10, MOUTH_Y, FACE_CX + 10, MOUTH_Y, MOUTH_COLOR);
  tft.drawLine(FACE_CX - 10, MOUTH_Y+1, FACE_CX + 10, MOUTH_Y+1, MOUTH_COLOR);
}

void drawCuriousFace(int leftX, int rightX) {
  drawEyeBase(leftX, FACE_CY);
  drawEyeBase(rightX, FACE_CY);
  drawPupil(leftX, FACE_CY);
  drawPupil(rightX, FACE_CY);
  tft.drawLine(FACE_CX - 4, MOUTH_Y, FACE_CX + 4, MOUTH_Y, MOUTH_COLOR);
  tft.drawLine(FACE_CX + 4, MOUTH_Y, FACE_CX + 4, MOUTH_Y - 8, MOUTH_COLOR);
  tft.drawLine(FACE_CX + 4, MOUTH_Y - 8, FACE_CX - 2, MOUTH_Y - 8, MOUTH_COLOR);
  tft.fillCircle(FACE_CX, MOUTH_Y + 5, 2, MOUTH_COLOR);
  tft.setTextColor(ACCENT_COLOR, FACE_BG);
  tft.setTextSize(1);
  tft.setCursor(rightX + 18, FACE_CY - 28);
  tft.print("?");
}

// ==================== STATUS BARS & SPEECH ====================
void drawTopBar() {
  tft.fillRect(0, 0, 240, TOP_BAR_H, BAR_BG);
  tft.setTextColor(ACCENT_COLOR, BAR_BG);
  tft.setTextSize(1);
  tft.setCursor(4, 5);
  tft.print(robotName);

  int wifiX = 85;
  if (WiFi.status() == WL_CONNECTED) {
    tft.fillRect(wifiX, 12, 2, 4, 0x07E0);
    tft.fillRect(wifiX+3, 10, 2, 6, 0x07E0);
    tft.fillRect(wifiX+6, 7, 2, 9, 0x07E0);
    tft.fillRect(wifiX+9, 4, 2, 12, 0x07E0);
  } else {
    tft.drawLine(wifiX, 4, wifiX+12, 16, 0xF800);
    tft.drawLine(wifiX+12, 4, wifiX, 16, 0xF800);
  }

  tft.setTextColor(TFT_WHITE, BAR_BG);
  tft.setCursor(118, 5);
  tft.print(stateToString(robotState));

  drawBatteryIcon(198, 3, batteryPercent);
}

void drawBottomBar() {
  int y = 240 - BOTTOM_BAR_H;
  tft.fillRect(0, y, 240, BOTTOM_BAR_H, BAR_BG);
  tft.setTextColor(0x867D, BAR_BG);
  tft.setCursor(4, y + 3);
  tft.print("D:");
  tft.print(distanceMm);
  tft.print("mm");

  tft.setCursor(90, y + 3);
  if (personInView) {
    tft.setTextColor(0xF809, BAR_BG);
    tft.print("PERSON!");
  } else {
    tft.setTextColor(0x867D, BAR_BG);
    tft.print("Clear");
  }

  tft.setCursor(160, y + 3);
  tft.setTextColor(0x867D, BAR_BG);
  tft.print("G:");
  tft.print(goalToString(currentGoal));
}

void drawBatteryIcon(int x, int y, float percent) {
  tft.drawRect(x, y, 22, 10, TFT_WHITE);
  tft.fillRect(x + 22, y + 2, 2, 6, TFT_WHITE);
  int fillW = (int)((22 - 4) * percent / 100.0);
  uint16_t color = (percent > 50) ? 0x07E0 : (percent > 20) ? 0xFFE0 : 0xF800;
  tft.fillRect(x + 2, y + 2, fillW, 6, color);
  tft.setTextColor(TFT_WHITE, BAR_BG);
  tft.setCursor(x + 26, y + 1);
  tft.print((int)percent);
  tft.print("%");
}

void showSpeech(String text) {
  currentSpeechText = text;
  speechDisplayTime = millis();
  int padding = 5;
  tft.setTextSize(1);
  int textW = tft.textWidth(text);
  int bubbleW = min(textW + padding * 2, SPEECH_MAX_W);
  int bubbleX = (240 - bubbleW) / 2;
  int bubbleY = SPEECH_Y;

  tft.fillRect(bubbleX - 2, bubbleY - 2, bubbleW + 4, 16, FACE_BG);
  tft.fillRoundRect(bubbleX, bubbleY, bubbleW, 14, 3, 0x2104);
  tft.drawRoundRect(bubbleX, bubbleY, bubbleW, 14, 3, ACCENT_COLOR);
  tft.setTextColor(TFT_WHITE, 0x2104);
  tft.setCursor(bubbleX + padding, bubbleY + 3);
  if (textW > SPEECH_MAX_W - padding * 2) {
    tft.print(text.substring(0, 22));
    tft.print("...");
  } else {
    tft.print(text);
  }
}

void clearSpeech() {
  tft.fillRect(15, SPEECH_Y - 2, 210, 18, FACE_BG);
}

// ==================== BOOT ANIMATION ====================
void playBootAnimation() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(ACCENT_COLOR, TFT_BLACK);
  tft.setTextSize(3);
  String txt = "WALL-E";
  for (int i = 0; i <= txt.length(); i++) {
    tft.setCursor(55, 75);
    tft.print(txt.substring(0, i));
    delay(120);
  }
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(75, 115);
  tft.print("AI Brain v4.1");

  int barY = 145, barW = 150, barX = (240 - barW) / 2;
  tft.drawRect(barX, barY, barW, 10, ACCENT_COLOR);
  String stages[] = {"Memory", "Sensors", "Vision", "Network", "AI"};
  for (int i = 0; i < 5; i++) {
    tft.fillRect(barX + 2, barY + 2, (barW - 4) * (i + 1) / 5, 6, ACCENT_COLOR);
    tft.setCursor(barX + barW/2 - 22, barY + 14);
    tft.print(stages[i]);
    delay(350);
    tft.fillRect(barX + barW/2 - 25, barY + 14, 50, 10, TFT_BLACK);
  }
  delay(200);
  tft.fillScreen(TFT_BLACK);
  setEmotion(EM_SLEEPY);
  delay(600);
  setEmotion(EM_NEUTRAL);
  delay(200);
  setEmotion(EM_HAPPY);
  delay(400);
  setEmotion(EM_NEUTRAL);
}

void drawSleepBreathing(unsigned long now) {
  static int breath = 0;
  breath = (now / 800) % 2;
  tft.fillRect(0, FACE_AREA_Y, 240, FACE_AREA_H, FACE_BG);
  int leftX = FACE_CX - EYE_RX - EYE_GAP/2;
  int rightX = FACE_CX + EYE_RX + EYE_GAP/2;
  int off = breath * 2;
  tft.drawLine(leftX - EYE_RX, FACE_CY + off, leftX + EYE_RX, FACE_CY + off, EYE_WHITE);
  tft.drawLine(leftX - EYE_RX, FACE_CY + off + 1, leftX + EYE_RX, FACE_CY + off + 1, EYE_WHITE);
  tft.drawLine(rightX - EYE_RX, FACE_CY + off, rightX + EYE_RX, FACE_CY + off, EYE_WHITE);
  tft.drawLine(rightX - EYE_RX, FACE_CY + off + 1, rightX + EYE_RX, FACE_CY + off + 1, EYE_WHITE);
  tft.drawLine(FACE_CX - 5, MOUTH_Y + off, FACE_CX + 5, MOUTH_Y + off, MOUTH_COLOR);
  int zY = FACE_CY - 25 - ((now / 600) % 3) * 10;
  tft.setTextColor(0x867D, FACE_BG);
  tft.setTextSize(2);
  tft.setCursor(rightX + 22, zY);
  tft.print("Z");
}

// ==================== ANIMATIONS ====================
void startAnimation(AnimState anim) {
  if (robotState == ERROR_STATE || robotState == SLEEPING) return;
  currentAnim = anim;
  animStep = 0;
  animStartTime = millis();
  lastActivityTime = millis();
}

void updateAnimations(unsigned long now) {
  if (currentAnim == ANIM_NONE) return;
  unsigned long elapsed = now - animStartTime;
  switch (currentAnim) {
    case ANIM_WAVE:
      if (elapsed < 250) { setArmAngleSafe(RIGHT_ARM_CH, 120); robotState = WAVING; }
      else if (elapsed < 790) { int c = (elapsed - 250) / 180; int p = (elapsed - 250) % 180; if (c < 3) setArmAngleSafe(RIGHT_ARM_CH, p < 90 ? 75 : 120); }
      else if (elapsed < 1000) { setArmAngleSafe(RIGHT_ARM_CH, ARM_HOME); }
      else { currentAnim = ANIM_NONE; robotState = IDLE; broadcastState(); }
      break;
    case ANIM_ARMS_UP_ANIM:
      if (elapsed < 300) { setArmAngleSafe(LEFT_ARM_CH, ARM_UP); setArmAngleSafe(RIGHT_ARM_CH, ARM_UP); robotState = ARMS_UP; }
      else if (elapsed < 800) { }
      else if (elapsed < 1100) { armsHome(); }
      else { currentAnim = ANIM_NONE; robotState = IDLE; broadcastState(); }
      break;
    case ANIM_ARMS_DOWN:
      if (elapsed < 300) { setArmAngleSafe(LEFT_ARM_CH, ARM_DOWN); setArmAngleSafe(RIGHT_ARM_CH, ARM_DOWN); }
      else { currentAnim = ANIM_NONE; robotState = IDLE; broadcastState(); }
      break;
    case ANIM_LOOK_LEFT:
      if (elapsed < 400) { setHeadAngleSafe(PAN_CH, 30); setHeadAngleSafe(TILT_CH, 90); }
      else { currentAnim = ANIM_NONE; broadcastState(); }
      break;
    case ANIM_LOOK_RIGHT:
      if (elapsed < 400) { setHeadAngleSafe(PAN_CH, 150); setHeadAngleSafe(TILT_CH, 90); }
      else { currentAnim = ANIM_NONE; broadcastState(); }
      break;
    case ANIM_DANCE:
      { int beat = elapsed / 250;
        switch (beat % 8) {
          case 0: setArmAngleSafe(LEFT_ARM_CH, ARM_UP); break;
          case 1: setArmAngleSafe(RIGHT_ARM_CH, ARM_UP); break;
          case 2: setArmAngleSafe(LEFT_ARM_CH, ARM_HOME); break;
          case 3: setArmAngleSafe(RIGHT_ARM_CH, ARM_HOME); break;
          case 4: setHeadAngleSafe(PAN_CH, 60); break;
          case 5: setHeadAngleSafe(PAN_CH, 120); break;
          case 6: setHeadAngleSafe(PAN_CH, 90); break;
          case 7: currentAnim = ANIM_NONE; robotState = IDLE; broadcastState(); break;
        }
      }
      break;
    case ANIM_GREET:
      if (elapsed < 200) { setHeadAngleSafe(PAN_CH, 70); setArmAngleSafe(RIGHT_ARM_CH, 110); }
      else if (elapsed < 400) { setHeadAngleSafe(PAN_CH, 110); setArmAngleSafe(RIGHT_ARM_CH, 130); }
      else if (elapsed < 600) { setHeadAngleSafe(PAN_CH, 90); setArmAngleSafe(RIGHT_ARM_CH, 120); }
      else if (elapsed < 1000) { }
      else { armsHome(); currentAnim = ANIM_NONE; robotState = IDLE; broadcastState(); }
      break;
    case ANIM_THINK:
      if (elapsed < 500) { setHeadAngleSafe(TILT_CH, 60); }
      else if (elapsed < 1000) { setHeadAngleSafe(TILT_CH, 120); }
      else if (elapsed < 1500) { setHeadAngleSafe(TILT_CH, 90); }
      else { currentAnim = ANIM_NONE; broadcastState(); }
      break;
    case ANIM_CONFUSED:
      { int shake = elapsed / 150;
        if (shake % 2 == 0) setHeadAngleSafe(PAN_CH, 80);
        else setHeadAngleSafe(PAN_CH, 100);
        if (elapsed > 900) { setHeadAngleSafe(PAN_CH, 90); currentAnim = ANIM_NONE; broadcastState(); }
      }
      break;
    case ANIM_HAPPY:
      if (elapsed < 200) { setArmAngleSafe(LEFT_ARM_CH, 110); setArmAngleSafe(RIGHT_ARM_CH, 110); }
      else if (elapsed < 400) { setArmAngleSafe(LEFT_ARM_CH, 130); setArmAngleSafe(RIGHT_ARM_CH, 130); }
      else if (elapsed < 600) { armsHome(); }
      else { currentAnim = ANIM_NONE; broadcastState(); }
      break;
    default: break;
  }
}

// ==================== SENSORS & SAFETY ====================
void updateSensors() {
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false);
  if (measure.RangeStatus != 4) {
    distanceMm = measure.RangeMilliMeter;
    obstacleDetected = (distanceMm < SAFE_DISTANCE_MM);
  }
  batteryVoltage = ina219.getBusVoltage_V();
  batteryCurrent = ina219.getCurrent_mA();
  batteryPercent = constrain((batteryVoltage - 3.2f) / (4.2f - 3.2f) * 100.0f, 0.0f, 100.0f);
  lowBattery = (batteryPercent < BATTERY_MIN_PCT);
}

void safetyCheck() {
  if (robotState == ERROR_STATE) return;
  if (robotState == MOVING && obstacleDetected) {
    stopMotors();
    robotState = AVOIDING;
    setEmotion(EM_WORRIED);
    showSpeech("Oh!");
    broadcastState();
    driveMotor(true, -1, 1500);
    driveMotor(false, -1, 1500);
    delay(300);
    stopMotors();
    logEvent("Obstacle avoided", currentLocation, "");
    robotState = IDLE;
    setEmotion(EM_NEUTRAL);
    broadcastState();
  }
  if (lowBattery) {
    stopMotors();
    emergencyStop();
    robotState = ERROR_STATE;
    setEmotion(EM_DEAD);
    showSpeech("Battery critical...");
    broadcastAlert("LOW_BATTERY");
  }
}

// ==================== POWER ====================
void enterSleep() {
  robotState = SLEEPING;
  sleepEnterTime = millis();
  stopMotors();
  armsHome();
  setServoAngle(PAN_CH, 90);
  setServoAngle(TILT_CH, 45);
  setEmotion(EM_SLEEPY);
  drawTopBar();
  drawBottomBar();
  broadcastState();
  logEvent("Went to sleep", currentLocation, "");
}

void handleSleepMode(unsigned long now) {
  size_t bytesRead = 0;
  i2s_read(I2S_PORT, &audioBuffer, sizeof(audioBuffer), &bytesRead, 0);
  if (bytesRead > 0) {
    int32_t sum = 0;
    for (int i = 0; i < bytesRead / 2; i++) sum += abs(audioBuffer[i]);
    if ((sum / (bytesRead / 2)) > 5000) { wakeUp(); return; }
  }
  if (now - sleepEnterTime > 600000) {
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_2, 0);
    esp_deep_sleep_start();
  }
}

void wakeUp() {
  robotState = IDLE;
  lastActivityTime = millis();
  setEmotion(EM_SURPRISED);
  showSpeech("I'm awake!");
  setServoAngle(TILT_CH, 90);
  drawTopBar();
  drawBottomBar();
  broadcastState();
  logEvent("Woke up", currentLocation, "");
  delay(400);
  setEmotion(EM_HAPPY);
}

// ==================== VISION ====================
void processVision() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (robotState == SLEEPING || robotState == ERROR_STATE) return;
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) return;
  String base64Image = base64Encode(fb->buf, fb->len);
  esp_camera_fb_return(fb);

  String payload = "{\"model\":\"gpt-4o-mini\",\"messages\":[{\"role\":\"user\",\"content\":[{\"type\":\"text\",\"text\":\"Describe what you see in one sentence. Is there a person?\"},{\"type\":\"image_url\",\"image_url\":{\"url\":\"data:image/jpeg;base64," + base64Image + "\"}}]}],\"max_tokens\":80}";

  http.begin(OPENAI_CHAT_URL);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", String("Bearer ") + OPENAI_API_KEY);
  int httpCode = http.POST(payload);
  if (httpCode == 200) {
    String response = http.getString();
    lastSceneDescription = parseVisionResponse(response);
    personInView = (lastSceneDescription.indexOf("person") >= 0 || lastSceneDescription.indexOf("people") >= 0);
    if (personInView) { lastPersonSeenTime = millis(); lastPersonDetected = lastSceneDescription; }
    broadcastVision();
  }
  http.end();
}

String parseVisionResponse(String json) {
  DynamicJsonDocument doc(4096);
  deserializeJson(doc, json);
  const char* content = doc["choices"][0]["message"]["content"];
  return content ? String(content) : "I see something...";
}

String base64Encode(uint8_t* data, size_t len) {
  static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  String encoded;
  int i = 0, j = 0;
  uint8_t char_array_3[3], char_array_4[4];
  while (len--) {
    char_array_3[i++] = *(data++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;
      for (i = 0; i < 4; i++) encoded += base64_chars[char_array_4[i]];
      i = 0;
    }
  }
  if (i) {
    for (j = i; j < 3; j++) char_array_3[j] = '\0';
    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    for (j = 0; j < (i + 1); j++) encoded += base64_chars[char_array_4[j]];
    while (i++ < 3) encoded += '=';
  }
  return encoded;
}

// ==================== VOICE ====================
void startListening() {
  if (robotState == SLEEPING || robotState == ERROR_STATE) return;
  robotState = LISTENING;
  isRecording = true;
  recordingStart = millis();
  setEmotion(EM_NEUTRAL);
  lookAt(0, -5);
  broadcastState();
}

void speakTTS(String text) {
  Serial.println("WALL-E: " + text);
  lastSpeakTime = millis();
  showSpeech(text);
}

// ==================== AUTONOMY ====================
void autonomyEngine() {
  if (robotState == ERROR_STATE || robotState == SLEEPING) return;
  if (robotState == INTERACTING || robotState == LISTENING || robotState == THINKING) return;

  if (personInView && millis() - lastPersonSeenTime < 5000) {
    if (currentGoal != GOAL_GREET && currentGoal != GOAL_FOLLOW) {
      currentGoal = GOAL_GREET;
      goalStartTime = millis();
      robotState = INTERACTING;
      setEmotion(EM_HAPPY);
      startAnimation(ANIM_GREET);
      String greeting = generateGreeting();
      speakTTS(greeting);
      logEvent("Greeted person", currentLocation, lastPersonDetected);
    }
    return;
  }

  if (batteryPercent < 20.0f) { currentGoal = GOAL_REST; enterSleep(); return; }

  if (currentGoal == GOAL_NONE && millis() - lastActivityTime > 60000) {
    currentGoal = GOAL_PATROL;
    goalStartTime = millis();
    robotState = PATROLLING;
    setEmotion(EM_DETERMINED);
    broadcastState();
    logEvent("Started patrolling", currentLocation, "");
  }

  switch (currentGoal) {
    case GOAL_PATROL: executePatrol(); break;
    case GOAL_EXPLORE: executeExplore(); break;
    case GOAL_FOLLOW: executeFollow(); break;
    default: break;
  }
}

String generateGreeting() {
  for (int i = 0; i < peopleCount; i++) {
    if (millis() - peopleMem[i].lastSeen < 300000) {
      peopleMem[i].encounterCount++;
      peopleMem[i].lastSeen = millis();
      return "Hello " + peopleMem[i].name + "! Nice to see you again!";
    }
  }
  return "Hello! I'm " + robotName + ". What's your name?";
}

void executePatrol() {
  if (millis() - goalStartTime > 15000) {
    stopMotors();
    currentGoal = GOAL_NONE;
    robotState = IDLE;
    setEmotion(EM_NEUTRAL);
    broadcastState();
    logEvent("Finished patrolling", currentLocation, "");
    return;
  }
  if (!obstacleDetected) {
    driveMotor(true, 1, 1800);
    driveMotor(false, 1, 1800);
    robotState = PATROLLING;
  } else {
    stopMotors();
    setHeadAngleSafe(PAN_CH, random(30, 150));
    delay(500);
  }
}

void executeExplore() {
  if (millis() - goalStartTime > 20000) {
    stopMotors();
    currentGoal = GOAL_NONE;
    robotState = IDLE;
    return;
  }
  if (!obstacleDetected) {
    driveMotor(true, 1, 1500);
    driveMotor(false, 1, 1500);
  } else {
    stopMotors();
    setHeadAngleSafe(PAN_CH, 45); delay(300);
    setHeadAngleSafe(PAN_CH, 135); delay(300);
    setHeadAngleSafe(PAN_CH, 90);
  }
}

void executeFollow() {
  if (millis() - goalStartTime > 30000 || !personInView) {
    stopMotors();
    currentGoal = GOAL_NONE;
    robotState = IDLE;
    speakTTS("I lost you. I'll wait here.");
    return;
  }
  if (!obstacleDetected) {
    driveMotor(true, 1, 2000);
    driveMotor(false, 1, 2000);
  }
}

// ==================== MEMORY SYSTEM ====================
void loadAllMemory() {
  loadPeople();
  loadPlaces();
  loadEvents();
  loadConfig();
}

void saveAllMemory() {
  savePeople();
  savePlaces();
  saveEvents();
  saveConfig();
}

void loadPeople() {
  File f = SD_MMC.open("/memory/people.json", FILE_READ);
  if (!f) return;
  DynamicJsonDocument doc(8192);
  deserializeJson(doc, f);
  f.close();
  peopleCount = 0;
  JsonArray arr = doc["people"];
  for (JsonObject p : arr) {
    if (peopleCount >= MAX_PEOPLE) break;
    peopleMem[peopleCount].name = p["name"].as<String>();
    peopleMem[peopleCount].description = p["description"].as<String>();
    peopleMem[peopleCount].preferences = p["preferences"].as<String>();
    peopleMem[peopleCount].lastGreeting = p["lastGreeting"].as<String>();
    peopleMem[peopleCount].lastSeen = p["lastSeen"];
    peopleMem[peopleCount].encounterCount = p["encounterCount"];
    peopleCount++;
  }
}

void savePeople() {
  DynamicJsonDocument doc(8192);
  JsonArray arr = doc.createNestedArray("people");
  for (int i = 0; i < peopleCount; i++) {
    JsonObject p = arr.createNestedObject();
    p["name"] = peopleMem[i].name;
    p["description"] = peopleMem[i].description;
    p["preferences"] = peopleMem[i].preferences;
    p["lastGreeting"] = peopleMem[i].lastGreeting;
    p["lastSeen"] = peopleMem[i].lastSeen;
    p["encounterCount"] = peopleMem[i].encounterCount;
  }
  File f = SD_MMC.open("/memory/people.json", FILE_WRITE);
  if (f) { serializeJson(doc, f); f.close(); }
}

void loadPlaces() {
  File f = SD_MMC.open("/memory/places.json", FILE_READ);
  if (!f) return;
  DynamicJsonDocument doc(4096);
  deserializeJson(doc, f);
  f.close();
  placesCount = 0;
  JsonArray arr = doc["places"];
  for (JsonObject p : arr) {
    if (placesCount >= MAX_PLACES) break;
    placesMem[placesCount].name = p["name"].as<String>();
    placesMem[placesCount].description = p["description"].as<String>();
    placesMem[placesCount].dangerLevel = p["dangerLevel"];
    placesMem[placesCount].lastVisited = p["lastVisited"];
    placesMem[placesCount].visitCount = p["visitCount"];
    placesCount++;
  }
}

void savePlaces() {
  DynamicJsonDocument doc(4096);
  JsonArray arr = doc.createNestedArray("places");
  for (int i = 0; i < placesCount; i++) {
    JsonObject p = arr.createNestedObject();
    p["name"] = placesMem[i].name;
    p["description"] = placesMem[i].description;
    p["dangerLevel"] = placesMem[i].dangerLevel;
    p["lastVisited"] = placesMem[i].lastVisited;
    p["visitCount"] = placesMem[i].visitCount;
  }
  File f = SD_MMC.open("/memory/places.json", FILE_WRITE);
  if (f) { serializeJson(doc, f); f.close(); }
}

void loadEvents() {
  File f = SD_MMC.open("/memory/events.json", FILE_READ);
  if (!f) return;
  DynamicJsonDocument doc(8192);
  deserializeJson(doc, f);
  f.close();
  eventsCount = 0;
  JsonArray arr = doc["events"];
  for (JsonObject e : arr) {
    if (eventsCount >= MAX_EVENTS) break;
    eventsMem[eventsCount].timestamp = e["timestamp"].as<String>();
    eventsMem[eventsCount].event = e["event"].as<String>();
    eventsMem[eventsCount].location = e["location"].as<String>();
    eventsMem[eventsCount].people = e["people"].as<String>();
    eventsCount++;
  }
}

void saveEvents() {
  DynamicJsonDocument doc(8192);
  JsonArray arr = doc.createNestedArray("events");
  for (int i = 0; i < eventsCount; i++) {
    JsonObject e = arr.createNestedObject();
    e["timestamp"] = eventsMem[i].timestamp;
    e["event"] = eventsMem[i].event;
    e["location"] = eventsMem[i].location;
    e["people"] = eventsMem[i].people;
  }
  File f = SD_MMC.open("/memory/events.json", FILE_WRITE);
  if (f) { serializeJson(doc, f); f.close(); }
}

void loadConfig() {
  File f = SD_MMC.open("/memory/config.json", FILE_READ);
  if (!f) return;
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, f);
  f.close();
  robotName = doc["robotName"].as<String>();
  ownerName = doc["ownerName"].as<String>();
}

void saveConfig() {
  DynamicJsonDocument doc(1024);
  doc["robotName"] = robotName;
  doc["ownerName"] = ownerName;
  File f = SD_MMC.open("/memory/config.json", FILE_WRITE);
  if (f) { serializeJson(doc, f); f.close(); }
}

void logEvent(String event, String location, String people) {
  if (eventsCount >= MAX_EVENTS) {
    for (int i = 0; i < MAX_EVENTS - 1; i++) eventsMem[i] = eventsMem[i + 1];
    eventsCount = MAX_EVENTS - 1;
  }
  eventsMem[eventsCount].timestamp = String(millis());
  eventsMem[eventsCount].event = event;
  eventsMem[eventsCount].location = location;
  eventsMem[eventsCount].people = people;
  eventsCount++;
}

void addPerson(String name, String description, String preferences) {
  if (peopleCount >= MAX_PEOPLE) return;
  peopleMem[peopleCount].name = name;
  peopleMem[peopleCount].description = description;
  peopleMem[peopleCount].preferences = preferences;
  peopleMem[peopleCount].lastSeen = millis();
  peopleMem[peopleCount].encounterCount = 1;
  peopleCount++;
  savePeople();
}

void addPlace(String name, String description, float danger) {
  if (placesCount >= MAX_PLACES) return;
  placesMem[placesCount].name = name;
  placesMem[placesCount].description = description;
  placesMem[placesCount].dangerLevel = danger;
  placesMem[placesCount].lastVisited = millis();
  placesMem[placesCount].visitCount = 1;
  placesCount++;
  savePlaces();
}

void clearAllMemory() {
  peopleCount = 0;
  placesCount = 0;
  eventsCount = 0;
  saveAllMemory();
}

// ==================== WEB HANDLERS ====================
void handleRoot() { server.send(200, "text/html", getDashboardHTML()); }

void handleStream() {
  WiFiClient client = server.client();
  String response = "HTTP/1.1 200 OK\r\nContent-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
  client.print(response);
  while (client.connected()) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) continue;
    client.printf("--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %d\r\n\r\n", fb->len);
    client.write(fb->buf, fb->len);
    client.print("\r\n");
    esp_camera_fb_return(fb);
    delay(50);
  }
}

void handleMove() {
  if (robotState == ERROR_STATE || robotState == SLEEPING) {
    server.send(403, "application/json", "{\"error\":\"unavailable\"}");
    return;
  }
  String dir = server.arg("dir");
  int speed = server.arg("speed").toInt();
  if (speed == 0) speed = 2200;
  lastActivityTime = millis();
  if (dir == "forward") { if (!obstacleDetected) { driveMotor(true, 1, speed); driveMotor(false, 1, speed); robotState = MOVING; }}
  else if (dir == "backward") { driveMotor(true, -1, speed); driveMotor(false, -1, speed); robotState = MOVING; }
  else if (dir == "left") { driveMotor(true, -1, speed); driveMotor(false, 1, speed); robotState = MOVING; }
  else if (dir == "right") { driveMotor(true, 1, speed); driveMotor(false, -1, speed); robotState = MOVING; }
  broadcastState();
  server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void handleStop() { emergencyStop(); server.send(200, "application/json", "{\"status\":\"stopped\"}"); }
void handleWave() { startAnimation(ANIM_WAVE); server.send(200, "application/json", "{\"status\":\"waving\"}"); }
void handleArmsUp() { startAnimation(ANIM_ARMS_UP_ANIM); server.send(200, "application/json", "{\"status\":\"arms_up\"}"); }
void handleArmsHome() { startAnimation(ANIM_ARMS_DOWN); server.send(200, "application/json", "{\"status\":\"arms_home\"}"); }

void handleLook() {
  int pan = server.arg("pan").toInt();
  int tilt = server.arg("tilt").toInt();
  setHeadAngleSafe(PAN_CH, pan);
  setHeadAngleSafe(TILT_CH, tilt);
  lastActivityTime = millis();
  server.send(200, "application/json", "{\"status\":\"looking\"}");
}

void handleStatus() {
  String json = "{\"state\":\"" + stateToString(robotState) + "\",\"goal\":\"" + goalToString(currentGoal) + "\",";
  json += "\"battery\":" + String(batteryPercent, 1) + ",\"voltage\":" + String(batteryVoltage, 2) + ",";
  json += "\"current\":" + String(batteryCurrent, 1) + ",\"distance\":" + String(distanceMm) + ",";
  json += "\"obstacle\":" + String(obstacleDetected ? "true" : "false") + ",";
  json += "\"person\":" + String(personInView ? "true" : "false") + ",";
  json += "\"scene\":\"" + lastSceneDescription + "\",\"lastResponse\":\"" + lastAIResponse + "\",";
  json += "\"anim\":\"" + animToString(currentAnim) + "\",\"peopleCount\":" + String(peopleCount) + ",";
  json += "\"placesCount\":" + String(placesCount) + ",\"eventsCount\":" + String(eventsCount) + "}";
  server.send(200, "application/json", json);
}

void handleSleepRequest() { enterSleep(); server.send(200, "application/json", "{\"status\":\"sleeping\"}"); }
void handleWakeRequest() { wakeUp(); server.send(200, "application/json", "{\"status\":\"awake\"}"); }

void handleSpeak() {
  String text = server.arg("text");
  speakTTS(text);
  server.send(200, "application/json", "{\"status\":\"speaking\",\"text\":\"" + text + "\"}");
}

void handleAsk() {
  String question = server.arg("q");
  String response = askGPT(question);
  speakTTS(response);
  server.send(200, "application/json", "{\"question\":\"" + question + "\",\"answer\":\"" + response + "\"}");
}

void handleVisionRequest() {
  server.send(200, "application/json", "{\"scene\":\"" + lastSceneDescription + "\",\"person\":" + String(personInView ? "true" : "false") + "}");
}

void handleGetPeople() {
  String json = "{\"people\":[";
  for (int i = 0; i < peopleCount; i++) {
    if (i > 0) json += ",";
    json += "{\"name\":\"" + peopleMem[i].name + "\",\"description\":\"" + peopleMem[i].description + "\",\"preferences\":\"" + peopleMem[i].preferences + "\",\"encounterCount\":" + String(peopleMem[i].encounterCount) + "}";
  }
  json += "]}";
  server.send(200, "application/json", json);
}

void handleGetPlaces() {
  String json = "{\"places\":[";
  for (int i = 0; i < placesCount; i++) {
    if (i > 0) json += ",";
    json += "{\"name\":\"" + placesMem[i].name + "\",\"description\":\"" + placesMem[i].description + "\",\"dangerLevel\":" + String(placesMem[i].dangerLevel) + "}";
  }
  json += "]}";
  server.send(200, "application/json", json);
}

void handleGetEvents() {
  String json = "{\"events\":[";
  for (int i = 0; i < eventsCount; i++) {
    if (i > 0) json += ",";
    json += "{\"timestamp\":\"" + eventsMem[i].timestamp + "\",\"event\":\"" + eventsMem[i].event + "\",\"location\":\"" + eventsMem[i].location + "\"}";
  }
  json += "]}";
  server.send(200, "application/json", json);
}

void handleAddPerson() {
  addPerson(server.arg("name"), server.arg("description"), server.arg("preferences"));
  server.send(200, "application/json", "{\"status\":\"added\"}");
}

void handleAddPlace() {
  addPlace(server.arg("name"), server.arg("description"), server.arg("danger").toFloat());
  server.send(200, "application/json", "{\"status\":\"added\"}");
}

void handleClearMemory() {
  clearAllMemory();
  server.send(200, "application/json", "{\"status\":\"cleared\"}");
}

String askGPT(String question) {
  if (WiFi.status() != WL_CONNECTED) return "No internet.";
  String payload = "{\"model\":\"gpt-4o-mini\",\"messages\":[{\"role\":\"system\",\"content\":\"You are " + robotName + ", a small friendly robot. Be concise (max 2 sentences).\"},{\"role\":\"user\",\"content\":\"" + question + "\"}],\"max_tokens\":100}";
  http.begin(OPENAI_CHAT_URL);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", String("Bearer ") + OPENAI_API_KEY);
  int httpCode = http.POST(payload);
  String response = "Thinking...";
  if (httpCode == 200) {
    String respJson = http.getString();
    DynamicJsonDocument doc(4096);
    deserializeJson(doc, respJson);
    const char* content = doc["choices"][0]["message"]["content"];
    if (content) response = String(content);
  } else { response = "Error " + String(httpCode); }
  http.end();
  lastAIResponse = response;
  return response;
}

// ==================== WEBSOCKET ====================
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  if (type == WStype_TEXT) {
    String cmd = String((char*)payload);
    if (cmd == "wave") startAnimation(ANIM_WAVE);
    else if (cmd == "armsup") startAnimation(ANIM_ARMS_UP_ANIM);
    else if (cmd == "armshome") startAnimation(ANIM_ARMS_DOWN);
    else if (cmd == "stop") emergencyStop();
    else if (cmd == "dance") startAnimation(ANIM_DANCE);
    else if (cmd == "lookleft") startAnimation(ANIM_LOOK_LEFT);
    else if (cmd == "lookright") startAnimation(ANIM_LOOK_RIGHT);
    else if (cmd == "greet") { startAnimation(ANIM_GREET); speakTTS(generateGreeting()); }
    else if (cmd == "think") startAnimation(ANIM_THINK);
    else if (cmd == "happy") startAnimation(ANIM_HAPPY);
    else if (cmd == "confused") startAnimation(ANIM_CONFUSED);
    else if (cmd == "sleep") enterSleep();
    else if (cmd == "wakeup") wakeUp();
    else if (cmd == "listen") startListening();
    else if (cmd == "patrol") { currentGoal = GOAL_PATROL; goalStartTime = millis(); }
    else if (cmd == "explore") { currentGoal = GOAL_EXPLORE; goalStartTime = millis(); }
    else if (cmd == "status") {
      String json = "{\"type\":\"status\",\"state\":\"" + stateToString(robotState) + "\",\"battery\":" + String(batteryPercent, 1) + ",\"distance\":" + String(distanceMm) + ",\"scene\":\"" + lastSceneDescription + "\"}";
      webSocket.sendTXT(num, json);
    }
    lastActivityTime = millis();
  }
}

void broadcastState() {
  String json = "{\"type\":\"state\",\"state\":\"" + stateToString(robotState) + "\",\"goal\":\"" + goalToString(currentGoal) + "\"}";
  webSocket.broadcastTXT(json);
}

void broadcastTelemetry() {
  String json = "{\"type\":\"telemetry\",\"battery\":" + String(batteryPercent, 1) + ",\"voltage\":" + String(batteryVoltage, 2) + ",\"current\":" + String(batteryCurrent, 1) + ",\"distance\":" + String(distanceMm) + "}";
  webSocket.broadcastTXT(json);
}

void broadcastAlert(String alert) {
  String json = "{\"type\":\"alert\",\"message\":\"" + alert + "\"}";
  webSocket.broadcastTXT(json);
}

void broadcastVision() {
  String json = "{\"type\":\"vision\",\"scene\":\"" + lastSceneDescription + "\",\"person\":" + String(personInView ? "true" : "false") + "}";
  webSocket.broadcastTXT(json);
}

void reportBattery() { broadcastTelemetry(); }

// ==================== HELPERS ====================
String stateToString(RobotState s) {
  switch (s) {
    case IDLE: return "IDLE";
    case MOVING: return "MOVING";
    case WAVING: return "WAVING";
    case ARMS_UP: return "ARMS_UP";
    case AVOIDING: return "AVOIDING";
    case ERROR_STATE: return "ERROR";
    case SLEEPING: return "SLEEPING";
    case LISTENING: return "LISTENING";
    case THINKING: return "THINKING";
    case PATROLLING: return "PATROLLING";
    case EXPLORING: return "EXPLORING";
    case INTERACTING: return "INTERACTING";
    case FOLLOWING: return "FOLLOWING";
    default: return "UNKNOWN";
  }
}

String goalToString(GoalType g) {
  switch (g) {
    case GOAL_NONE: return "none";
    case GOAL_PATROL: return "patrol";
    case GOAL_EXPLORE: return "explore";
    case GOAL_GREET: return "greet";
    case GOAL_FOLLOW: return "follow";
    case GOAL_REST: return "rest";
    default: return "unknown";
  }
}

String animToString(AnimState a) {
  switch (a) {
    case ANIM_NONE: return "none";
    case ANIM_WAVE: return "wave";
    case ANIM_ARMS_UP_ANIM: return "arms_up";
    case ANIM_ARMS_DOWN: return "arms_down";
    case ANIM_LOOK_LEFT: return "look_left";
    case ANIM_LOOK_RIGHT: return "look_right";
    case ANIM_DANCE: return "dance";
    case ANIM_GREET: return "greet";
    case ANIM_THINK: return "think";
    case ANIM_CONFUSED: return "confused";
    case ANIM_HAPPY: return "happy";
    default: return "unknown";
  }
}

// ==================== DASHBOARD HTML ====================
String getDashboardHTML() {
  return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>WALL-E AI Brain v4.1</title>
<style>
  *{margin:0;padding:0;box-sizing:border-box}
  body{background:#0a0a0a;color:#e0e0e0;font-family:'Segoe UI',sans-serif;min-height:100vh;padding:15px}
  .header{text-align:center;margin-bottom:15px}
  .header h1{color:#f4d03f;font-size:1.8em;text-shadow:0 0 15px rgba(244,208,63,0.4)}
  .grid{display:grid;grid-template-columns:1fr 1fr;gap:12px;max-width:1000px;margin:0 auto}
  .panel{background:#141414;border-radius:12px;padding:14px;border:1px solid #2a2a2a}
  .panel h3{color:#f4d03f;margin-bottom:10px;font-size:0.85em;text-transform:uppercase;letter-spacing:1px}
  .full{grid-column:1 / -1}
  .camera-feed{text-align:center}
  .camera-feed img{width:100%;max-width:640px;border-radius:8px;border:2px solid #f4d03f}
  .status-bar{display:flex;justify-content:space-around;flex-wrap:wrap;gap:10px;margin:8px 0}
  .status-item{text-align:center;min-width:70px}
  .status-value{font-size:1.4em;font-weight:bold;color:#f4d03f}
  .status-label{font-size:0.65em;color:#888;text-transform:uppercase}
  .btn-grid{display:grid;grid-template-columns:repeat(3,1fr);gap:6px}
  button{background:#1f1f1f;color:#e0e0e0;border:1px solid #333;padding:10px;border-radius:8px;cursor:pointer;font-size:0.8em;transition:all 0.2s}
  button:hover{background:#f4d03f;color:#000;border-color:#f4d03f}
  button.danger{background:#3a1515;border-color:#733}
  button.danger:hover{background:#e44;color:#fff}
  button.primary{background:#152a3a;border-color:#357}
  button.primary:hover{background:#48a;color:#fff}
  button.ai{background:#2a1a3a;border-color:#648}
  button.ai:hover{background:#a4f;color:#fff}
  .dpad{display:grid;grid-template-columns:repeat(3,1fr);gap:4px;max-width:140px;margin:0 auto}
  .dpad button{padding:14px;font-size:1.1em}
  .chat-box{background:#0a0a0a;border-radius:8px;padding:10px;min-height:80px;font-size:0.85em}
  .chat-input{display:flex;gap:6px;margin-top:8px}
  .chat-input input{flex:1;background:#1a1a1a;border:1px solid #333;color:#e0e0e0;padding:8px;border-radius:6px}
  .memory-list{max-height:120px;overflow-y:auto;font-size:0.75em;background:#0a0a0a;padding:8px;border-radius:6px}
  .memory-item{padding:3px 0;border-bottom:1px solid #1a1a1a}
  .log{max-height:120px;overflow-y:auto;font-family:monospace;font-size:0.75em;background:#0a0a0a;padding:10px;border-radius:8px}
  .log-entry{margin:2px 0}
  .conn{display:inline-block;width:10px;height:10px;border-radius:50%;background:#e44;margin-right:6px}
  .conn.ok{background:#4e4}
  .scene-box{background:#0a0a0a;padding:10px;border-radius:6px;font-size:0.85em;color:#aaa;min-height:40px}
  @media(max-width:600px){.grid{grid-template-columns:1fr}}
</style>
</head>
<body>
  <div class="header">
    <h1>🤖 WALL-E AI Brain v4.1</h1>
    <p><span class="conn" id="conn"></span><span id="connText">Connecting...</span></p>
  </div>

  <div class="grid">
    <div class="panel full camera-feed">
      <h3>📷 Live Vision</h3>
      <img src="/stream" id="camStream" alt="Camera">
    </div>

    <div class="panel">
      <h3>📊 Body Telemetry</h3>
      <div class="status-bar">
        <div class="status-item"><div class="status-value" id="batVal">--</div><div class="status-label">Battery %</div></div>
        <div class="status-item"><div class="status-value" id="voltVal">--</div><div class="status-label">Voltage</div></div>
        <div class="status-item"><div class="status-value" id="distVal">--</div><div class="status-label">Distance</div></div>
      </div>
      <div class="status-bar">
        <div class="status-item"><div class="status-value" id="stateVal">--</div><div class="status-label">State</div></div>
        <div class="status-item"><div class="status-value" id="goalVal">--</div><div class="status-label">Goal</div></div>
        <div class="status-item"><div class="status-value" id="obsVal">--</div><div class="status-label">Obstacle</div></div>
      </div>
    </div>

    <div class="panel">
      <h3>👁️ AI Vision</h3>
      <div class="scene-box" id="sceneBox">Waiting for vision...</div>
      <div class="status-bar" style="margin-top:8px">
        <div class="status-item"><div class="status-value" id="personVal">--</div><div class="status-label">Person</div></div>
        <div class="status-item"><div class="status-value" id="peopleCount">--</div><div class="status-label">Known</div></div>
      </div>
    </div>

    <div class="panel">
      <h3>🎮 Movement</h3>
      <div class="dpad">
        <div></div><button onclick="move('forward')">▲</button><div></div>
        <button onclick="move('left')">◀</button>
        <button class="danger" onclick="sendCmd('stop')">⏹</button>
        <button onclick="move('right')">▶</button>
        <div></div><button onclick="move('backward')">▼</button><div></div>
      </div>
    </div>

    <div class="panel">
      <h3>🦾 Animations</h3>
      <div class="btn-grid">
        <button onclick="sendCmd('wave')">👋 Wave</button>
        <button onclick="sendCmd('armsup')">🙌 Arms Up</button>
        <button onclick="sendCmd('armshome')">🏠 Home</button>
        <button onclick="sendCmd('lookleft')">👈 Look L</button>
        <button onclick="sendCmd('lookright')">👉 Look R</button>
        <button onclick="sendCmd('dance')">💃 Dance</button>
        <button onclick="sendCmd('greet')">🤝 Greet</button>
        <button onclick="sendCmd('happy')">😊 Happy</button>
        <button onclick="sendCmd('confused')">😕 Huh?</button>
      </div>
    </div>

    <div class="panel">
      <h3>🧠 AI Chat</h3>
      <div class="chat-box" id="chatBox">Ask me anything...</div>
      <div class="chat-input">
        <input type="text" id="chatInput" placeholder="Ask WALL-E..." onkeypress="if(event.key==='Enter')askAI()">
        <button class="ai" onclick="askAI()">Ask</button>
      </div>
    </div>

    <div class="panel">
      <h3>🎤 Voice & Power</h3>
      <div class="btn-grid">
        <button class="ai" onclick="sendCmd('listen')">🎙️ Listen</button>
        <button class="primary" onclick="sendCmd('patrol')">🚶 Patrol</button>
        <button class="primary" onclick="sendCmd('explore')">🔍 Explore</button>
        <button class="primary" onclick="sendCmd('sleep')">😴 Sleep</button>
        <button class="primary" onclick="sendCmd('wakeup')">⏰ Wake</button>
      </div>
    </div>

    <div class="panel">
      <h3>💾 Memory</h3>
      <div class="btn-grid">
        <button onclick="loadMemory('people')">👥 People</button>
        <button onclick="loadMemory('places')">📍 Places</button>
        <button onclick="loadMemory('events')">📅 Events</button>
        <button class="danger" onclick="clearMemory()">🗑️ Clear</button>
      </div>
      <div class="memory-list" id="memoryList" style="margin-top:8px">Tap to view...</div>
    </div>

    <div class="panel full log" id="logPanel">
      <h3>📝 Event Log</h3>
    </div>
  </div>

<script>
  let ws;
  const logPanel = document.getElementById('logPanel');
  const connStatus = document.getElementById('conn');
  const connText = document.getElementById('connText');
  const chatBox = document.getElementById('chatBox');

  function log(msg, type='info') {
    const div = document.createElement('div');
    div.className = 'log-entry';
    div.style.color = type==='err'?'#e44':type==='warn'?'#fa4':'#48a';
    div.textContent = new Date().toLocaleTimeString() + ' | ' + msg;
    logPanel.appendChild(div);
    logPanel.scrollTop = logPanel.scrollHeight;
  }

  function connectWS() {
    ws = new WebSocket('ws://' + location.host + ':81');
    ws.onopen = () => { connStatus.classList.add('ok'); connText.textContent = 'Connected'; log('Connected'); };
    ws.onclose = () => { connStatus.classList.remove('ok'); connText.textContent = 'Disconnected'; log('Disconnected', 'warn'); setTimeout(connectWS, 3000); };
    ws.onmessage = (e) => {
      const data = JSON.parse(e.data);
      if (data.type === 'telemetry') {
        document.getElementById('batVal').textContent = data.battery.toFixed(1) + '%';
        document.getElementById('voltVal').textContent = data.voltage.toFixed(2) + 'V';
        document.getElementById('distVal').textContent = data.distance + 'mm';
      } else if (data.type === 'state') {
        document.getElementById('stateVal').textContent = data.state;
        document.getElementById('goalVal').textContent = data.goal;
      } else if (data.type === 'status') {
        document.getElementById('batVal').textContent = data.battery.toFixed(1) + '%';
        document.getElementById('stateVal').textContent = data.state;
        document.getElementById('obsVal').textContent = data.obstacle ? 'YES' : 'NO';
        document.getElementById('sceneBox').textContent = data.scene || 'No data';
        document.getElementById('personVal').textContent = data.person ? 'YES' : 'NO';
      } else if (data.type === 'vision') {
        document.getElementById('sceneBox').textContent = data.scene;
        document.getElementById('personVal').textContent = data.person ? 'YES' : 'NO';
      } else if (data.type === 'alert') {
        log('ALERT: ' + data.message, 'err');
      }
    };
  }

  function sendCmd(cmd) {
    if (ws && ws.readyState === 1) { ws.send(cmd); log('Sent: ' + cmd); }
    else log('Not connected', 'warn');
  }

  function move(dir) {
    fetch('/api/move?dir=' + dir + '&speed=2200', {method: 'POST'})
      .then(r => r.json()).then(d => log('Move: ' + dir)).catch(e => log('Move failed', 'err'));
  }

  function askAI() {
    const q = document.getElementById('chatInput').value;
    if (!q) return;
    chatBox.innerHTML += '<div style="color:#48a">You: ' + q + '</div>';
    document.getElementById('chatInput').value = '';
    fetch('/api/ask?q=' + encodeURIComponent(q), {method: 'POST'})
      .then(r => r.json()).then(d => {
        chatBox.innerHTML += '<div style="color:#f4d03f">WALL-E: ' + d.answer + '</div>';
        log('AI: ' + d.answer);
      }).catch(e => log('AI failed', 'err'));
  }

  function loadMemory(type) {
    fetch('/api/memory/' + type).then(r => r.json()).then(d => {
      const list = document.getElementById('memoryList');
      list.innerHTML = '';
      const arr = d[type] || [];
      if (arr.length === 0) { list.innerHTML = '<div class="memory-item">No entries</div>'; return; }
      arr.forEach(item => {
        const div = document.createElement('div');
        div.className = 'memory-item';
        div.textContent = (item.name || item.event || 'Entry') + ' - ' + (item.description || item.location || '');
        list.appendChild(div);
      });
    });
  }

  function clearMemory() {
    if (!confirm('Clear ALL memory?')) return;
    fetch('/api/memory/clear', {method: 'POST'}).then(r => r.json()).then(d => {
      log('Memory cleared');
      document.getElementById('memoryList').innerHTML = 'Cleared';
    });
  }

  setInterval(() => { if (ws && ws.readyState === 1) ws.send('status'); }, 2000);
  connectWS();
  log('Dashboard loaded');
</script>
</body>
</html>
)rawliteral";
}
