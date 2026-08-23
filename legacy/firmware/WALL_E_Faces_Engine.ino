
// ==================== WALL-E FACE ENGINE + SCREEN MAXIMIZER ====================
// Add this section to WALL_E_AI_Brain_v4.ino
// Replaces showFace() and adds full emotional face system

// Face dimensions (centered on 240x240 screen)
#define FACE_CX       120
#define FACE_CY       110
#define EYE_RX        35      // Eye radius X
#define EYE_RY        45      // Eye radius Y
#define EYE_GAP       30      // Gap between eyes
#define PUPIL_R       12      // Pupil radius
#define MOUTH_Y       175     // Mouth Y position

// Emotion states
enum Emotion {
  EM_NEUTRAL, EM_HAPPY, EM_SAD, EM_ANGRY, EM_SURPRISED,
  EM_SLEEPY, EM_CONFUSED, EM_LOVE, EM_DEAD, EM_THINKING,
  EM_EXCITED, EM_WORRIED, EM_COOL, EM_SHY, EM_DETERMINED
};
Emotion currentEmotion = EM_NEUTRAL;
Emotion targetEmotion = EM_NEUTRAL;
unsigned long emotionStartTime = 0;
unsigned long lastBlinkTime = 0;
unsigned long nextBlinkInterval = 3000;
bool isBlinking = false;
int blinkPhase = 0; // 0=open, 1=closing, 2=closed, 3=opening
float eyeOffsetX = 0;
float eyeOffsetY = 0;
float targetEyeOffsetX = 0;
float targetEyeOffsetY = 0;

// Screen sections
#define TOP_BAR_H     22
#define BOTTOM_BAR_H  20
#define FACE_AREA_Y   TOP_BAR_H
#define FACE_AREA_H   (240 - TOP_BAR_H - BOTTOM_BAR_H)

// Colors
#define FACE_BG       TFT_BLACK
#define EYE_WHITE     0xE71C    // Slightly warm white
#define EYE_COLOR     0x5AEB    // WALL-E yellow-brown eye
#define PUPIL_COLOR   TFT_BLACK
#define MOUTH_COLOR   0x79E0    // Light grey
#define BLUSH_COLOR   0xF809    // Pink
#define ACCENT_COLOR  0xF4D03F  // Yellow accent

// ==================== FACE DRAWING ENGINE ====================
void drawFace(Emotion em) {
  // Clear face area only (preserve bars)
  tft.fillRect(0, FACE_AREA_Y, 240, FACE_AREA_H, FACE_BG);

  switch (em) {
    case EM_NEUTRAL:    drawNeutralFace(); break;
    case EM_HAPPY:      drawHappyFace(); break;
    case EM_SAD:        drawSadFace(); break;
    case EM_ANGRY:      drawAngryFace(); break;
    case EM_SURPRISED:  drawSurprisedFace(); break;
    case EM_SLEEPY:     drawSleepyFace(); break;
    case EM_CONFUSED:   drawConfusedFace(); break;
    case EM_LOVE:       drawLoveFace(); break;
    case EM_DEAD:       drawDeadFace(); break;
    case EM_THINKING:   drawThinkingFace(); break;
    case EM_EXCITED:    drawExcitedFace(); break;
    case EM_WORRIED:    drawWorriedFace(); break;
    case EM_COOL:       drawCoolFace(); break;
    case EM_SHY:        drawShyFace(); break;
    case EM_DETERMINED: drawDeterminedFace(); break;
  }
}

// --- Eye Drawing Helpers ---
void drawEyeBase(int cx, int cy, int rx, int ry) {
  tft.fillEllipse(cx, cy, rx, ry, EYE_WHITE);
  tft.drawEllipse(cx, cy, rx, ry, 0x4208); // Dark outline
}

void drawPupil(int cx, int cy, int r, float offsetX, float offsetY) {
  int px = cx + (int)offsetX;
  int py = cy + (int)offsetY;
  // Limit pupil movement inside eye
  float dx = px - cx;
  float dy = py - cy;
  float dist = sqrt(dx*dx + dy*dy);
  float maxDist = (EYE_RX - PUPIL_R - 4) * 0.6;
  if (dist > maxDist) {
    px = cx + (int)(dx * maxDist / dist);
    py = cy + (int)(dy * maxDist / dist);
  }
  tft.fillCircle(px, py, r, PUPIL_COLOR);
  // Eye shine
  tft.fillCircle(px - 3, py - 3, 3, TFT_WHITE);
}

void drawBlinkingEye(int cx, int cy, int rx, int phase) {
  if (phase == 0) { // Open - draw normally
    drawEyeBase(cx, cy, rx, ry);
  } else if (phase == 1) { // Closing
    int ry_closed = ry * (100 - phase * 25) / 100;
    tft.fillEllipse(cx, cy, rx, ry_closed, EYE_WHITE);
    tft.drawEllipse(cx, cy, rx, ry_closed, 0x4208);
  } else if (phase == 2) { // Closed
    tft.drawLine(cx - rx, cy, cx + rx, cy, 0x4208);
    tft.drawLine(cx - rx, cy + 1, cx + rx, cy + 1, 0x4208);
  } else { // Opening
    int ry_open = ry * (phase - 2) * 25 / 100;
    tft.fillEllipse(cx, cy, rx, ry_open, EYE_WHITE);
    tft.drawEllipse(cx, cy, rx, ry_open, 0x4208);
  }
}

// --- Individual Face Emotions ---
void drawNeutralFace() {
  int leftX = FACE_CX - EYE_RX - EYE_GAP/2;
  int rightX = FACE_CX + EYE_RX + EYE_GAP/2;
  drawEyeBase(leftX, FACE_CY, EYE_RX, EYE_RY);
  drawEyeBase(rightX, FACE_CY, EYE_RX, EYE_RY);
  drawPupil(leftX, FACE_CY, PUPIL_R, eyeOffsetX, eyeOffsetY);
  drawPupil(rightX, FACE_CY, PUPIL_R, eyeOffsetX, eyeOffsetY);
  // Neutral mouth
  tft.drawLine(FACE_CX - 20, MOUTH_Y, FACE_CX + 20, MOUTH_Y, MOUTH_COLOR);
  tft.drawLine(FACE_CX - 20, MOUTH_Y + 1, FACE_CX + 20, MOUTH_Y + 1, MOUTH_COLOR);
}

void drawHappyFace() {
  int leftX = FACE_CX - EYE_RX - EYE_GAP/2;
  int rightX = FACE_CX + EYE_RX + EYE_GAP/2;
  drawEyeBase(leftX, FACE_CY - 5, EYE_RX, EYE_RY);
  drawEyeBase(rightX, FACE_CY - 5, EYE_RX, EYE_RY);
  drawPupil(leftX, FACE_CY - 5, PUPIL_R, eyeOffsetX, eyeOffsetY - 5);
  drawPupil(rightX, FACE_CY - 5, PUPIL_R, eyeOffsetX, eyeOffsetY - 5);
  // Happy smile (arc)
  for (int i = 0; i < 5; i++) {
    tft.drawArc(FACE_CX, MOUTH_Y - 10, 25, 35, 200 + i*2, 340 - i*2, MOUTH_COLOR);
  }
  // Cheeks
  tft.fillCircle(leftX - 25, FACE_CY + 20, 6, BLUSH_COLOR);
  tft.fillCircle(rightX + 25, FACE_CY + 20, 6, BLUSH_COLOR);
}

void drawSadFace() {
  int leftX = FACE_CX - EYE_RX - EYE_GAP/2;
  int rightX = FACE_CX + EYE_RX + EYE_GAP/2;
  // Droopy eyes
  drawEyeBase(leftX, FACE_CY + 5, EYE_RX, EYE_RY);
  drawEyeBase(rightX, FACE_CY + 5, EYE_RX, EYE_RY);
  drawPupil(leftX, FACE_CY + 5, PUPIL_R, eyeOffsetX, eyeOffsetY + 5);
  drawPupil(rightX, FACE_CY + 5, PUPIL_R, eyeOffsetX, eyeOffsetY + 5);
  // Sad mouth (inverted arc)
  for (int i = 0; i < 4; i++) {
    tft.drawArc(FACE_CX, MOUTH_Y + 20, 20, 30, 20 + i*2, 160 - i*2, MOUTH_COLOR);
  }
  // Tear
  tft.fillCircle(rightX + 15, FACE_CY + 25, 3, 0x5DDF);
}

void drawAngryFace() {
  int leftX = FACE_CX - EYE_RX - EYE_GAP/2;
  int rightX = FACE_CX + EYE_RX + EYE_GAP/2;
  // Angry eyebrows (slanted lines)
  tft.drawLine(leftX - 25, FACE_CY - 35, leftX + 15, FACE_CY - 20, 0x4208);
  tft.drawLine(rightX + 25, FACE_CY - 35, rightX - 15, FACE_CY - 20, 0x4208);
  drawEyeBase(leftX, FACE_CY, EYE_RX, EYE_RY);
  drawEyeBase(rightX, FACE_CY, EYE_RX, EYE_RY);
  drawPupil(leftX, FACE_CY, PUPIL_R, eyeOffsetX, eyeOffsetY);
  drawPupil(rightX, FACE_CY, PUPIL_R, eyeOffsetX, eyeOffsetY);
  // Angry mouth (flat line)
  tft.drawLine(FACE_CX - 25, MOUTH_Y, FACE_CX + 25, MOUTH_Y, MOUTH_COLOR);
  tft.drawLine(FACE_CX - 25, MOUTH_Y + 1, FACE_CX + 25, MOUTH_Y + 1, MOUTH_COLOR);
  tft.drawLine(FACE_CX - 25, MOUTH_Y + 2, FACE_CX + 25, MOUTH_Y + 2, MOUTH_COLOR);
  // Red tint on cheeks
  tft.fillCircle(leftX - 20, FACE_CY + 15, 5, 0xD809);
  tft.fillCircle(rightX + 20, FACE_CY + 15, 5, 0xD809);
}

void drawSurprisedFace() {
  int leftX = FACE_CX - EYE_RX - EYE_GAP/2;
  int rightX = FACE_CX + EYE_RX + EYE_GAP/2;
  // Wide eyes
  drawEyeBase(leftX, FACE_CY - 10, EYE_RX + 5, EYE_RY + 5);
  drawEyeBase(rightX, FACE_CY - 10, EYE_RX + 5, EYE_RY + 5);
  drawPupil(leftX, FACE_CY - 10, PUPIL_R - 2, eyeOffsetX, eyeOffsetY);
  drawPupil(rightX, FACE_CY - 10, PUPIL_R - 2, eyeOffsetX, eyeOffsetY);
  // O mouth
  tft.fillEllipse(FACE_CX, MOUTH_Y, 10, 15, MOUTH_COLOR);
  // Eyebrows raised
  tft.drawArc(leftX, FACE_CY - 40, 20, 25, 200, 340, 0x4208);
  tft.drawArc(rightX, FACE_CY - 40, 20, 25, 200, 340, 0x4208);
}

void drawSleepyFace() {
  int leftX = FACE_CX - EYE_RX - EYE_GAP/2;
  int rightX = FACE_CX + EYE_RX + EYE_GAP/2;
  // Half-closed eyes (drawn as lines)
  tft.drawLine(leftX - EYE_RX, FACE_CY, leftX + EYE_RX, FACE_CY, EYE_WHITE);
  tft.drawLine(leftX - EYE_RX, FACE_CY + 1, leftX + EYE_RX, FACE_CY + 1, EYE_WHITE);
  tft.drawLine(rightX - EYE_RX, FACE_CY, rightX + EYE_RX, FACE_CY, EYE_WHITE);
  tft.drawLine(rightX - EYE_RX, FACE_CY + 1, rightX + EYE_RX, FACE_CY + 1, EYE_WHITE);
  // Small mouth
  tft.drawLine(FACE_CX - 8, MOUTH_Y, FACE_CX + 8, MOUTH_Y, MOUTH_COLOR);
  // Zzz animation
  static int zOffset = 0;
  zOffset = (millis() / 500) % 3;
  tft.setTextColor(0x867D, FACE_BG);
  tft.setTextSize(2);
  tft.setCursor(rightX + 30, FACE_CY - 20 - zOffset * 10);
  tft.print("Z");
  if (zOffset > 0) { tft.setCursor(rightX + 40, FACE_CY - 30 - zOffset * 10); tft.print("z"); }
  if (zOffset > 1) { tft.setCursor(rightX + 50, FACE_CY - 40 - zOffset * 10); tft.print("z"); }
}

void drawConfusedFace() {
  int leftX = FACE_CX - EYE_RX - EYE_GAP/2;
  int rightX = FACE_CX + EYE_RX + EYE_GAP/2;
  drawEyeBase(leftX, FACE_CY, EYE_RX, EYE_RY);
  drawEyeBase(rightX, FACE_CY, EYE_RX, EYE_RY);
  // One pupil up, one down
  drawPupil(leftX, FACE_CY, PUPIL_R, eyeOffsetX - 5, eyeOffsetY - 8);
  drawPupil(rightX, FACE_CY, PUPIL_R, eyeOffsetX + 5, eyeOffsetY + 5);
  // Question mark mouth
  tft.drawLine(FACE_CX - 5, MOUTH_Y, FACE_CX + 5, MOUTH_Y, MOUTH_COLOR);
  tft.drawLine(FACE_CX + 5, MOUTH_Y, FACE_CX + 5, MOUTH_Y - 8, MOUTH_COLOR);
  tft.drawLine(FACE_CX + 5, MOUTH_Y - 8, FACE_CX - 2, MOUTH_Y - 8, MOUTH_COLOR);
  tft.fillCircle(FACE_CX, MOUTH_Y + 5, 2, MOUTH_COLOR);
  // Question marks around head
  tft.setTextColor(ACCENT_COLOR, FACE_BG);
  tft.setTextSize(1);
  tft.setCursor(leftX - 30, FACE_CY - 30);
  tft.print("?");
  tft.setCursor(rightX + 20, FACE_CY - 25);
  tft.print("?");
}

void drawLoveFace() {
  int leftX = FACE_CX - EYE_RX - EYE_GAP/2;
  int rightX = FACE_CX + EYE_RX + EYE_GAP/2;
  // Heart eyes
  drawHeart(leftX, FACE_CY, 18, BLUSH_COLOR);
  drawHeart(rightX, FACE_CY, 18, BLUSH_COLOR);
  // Happy mouth
  for (int i = 0; i < 4; i++) {
    tft.drawArc(FACE_CX, MOUTH_Y - 10, 20, 30, 200 + i*2, 340 - i*2, MOUTH_COLOR);
  }
  // Hearts floating
  static int hOffset = 0;
  hOffset = (millis() / 300) % 4;
  tft.setTextColor(BLUSH_COLOR, FACE_BG);
  tft.setTextSize(1);
  int hx[4] = {leftX - 35, rightX + 35, leftX - 20, rightX + 20};
  int hy[4] = {FACE_CY - 35, FACE_CY - 40, FACE_CY - 50, FACE_CY - 45};
  for (int i = 0; i <= hOffset && i < 4; i++) {
    tft.setCursor(hx[i], hy[i]);
    tft.print("\x03"); // Heart char
  }
}

void drawHeart(int cx, int cy, int size, uint16_t color) {
  // Simple heart shape using arcs and lines
  int r = size / 2;
  tft.fillCircle(cx - r/2, cy - r/3, r/2, color);
  tft.fillCircle(cx + r/2, cy - r/3, r/2, color);
  tft.fillTriangle(cx - r, cy - r/3, cx + r, cy - r/3, cx, cy + r, color);
}

void drawDeadFace() {
  int leftX = FACE_CX - EYE_RX - EYE_GAP/2;
  int rightX = FACE_CX + EYE_RX + EYE_GAP/2;
  // X eyes
  tft.drawLine(leftX - 15, FACE_CY - 15, leftX + 15, FACE_CY + 15, 0x4208);
  tft.drawLine(leftX + 15, FACE_CY - 15, leftX - 15, FACE_CY + 15, 0x4208);
  tft.drawLine(rightX - 15, FACE_CY - 15, rightX + 15, FACE_CY + 15, 0x4208);
  tft.drawLine(rightX + 15, FACE_CY - 15, rightX - 15, FACE_CY + 15, 0x4208);
  // Flat mouth
  tft.drawLine(FACE_CX - 15, MOUTH_Y, FACE_CX + 15, MOUTH_Y, MOUTH_COLOR);
  // Battery icon showing empty
  drawBatteryIcon(200, 30, 0);
}

void drawThinkingFace() {
  int leftX = FACE_CX - EYE_RX - EYE_GAP/2;
  int rightX = FACE_CX + EYE_RX + EYE_GAP/2;
  drawEyeBase(leftX, FACE_CY, EYE_RX, EYE_RY);
  drawEyeBase(rightX, FACE_CY, EYE_RX, EYE_RY);
  // Pupils looking up
  drawPupil(leftX, FACE_CY, PUPIL_R, eyeOffsetX, eyeOffsetY - 12);
  drawPupil(rightX, FACE_CY, PUPIL_R, eyeOffsetX, eyeOffsetY - 12);
  // Small o mouth
  tft.fillCircle(FACE_CX, MOUTH_Y, 5, MOUTH_COLOR);
  // Thought bubble
  tft.fillCircle(rightX + 35, FACE_CY - 30, 4, 0xC618);
  tft.fillCircle(rightX + 42, FACE_CY - 40, 6, 0xC618);
  tft.fillCircle(rightX + 50, FACE_CY - 52, 10, 0xC618);
  tft.setTextColor(TFT_BLACK, 0xC618);
  tft.setTextSize(1);
  tft.setCursor(rightX + 44, FACE_CY - 56);
  tft.print("...");
}

void drawExcitedFace() {
  int leftX = FACE_CX - EYE_RX - EYE_GAP/2;
  int rightX = FACE_CX + EYE_RX + EYE_GAP/2;
  // Star eyes
  drawStar(leftX, FACE_CY, 20, ACCENT_COLOR);
  drawStar(rightX, FACE_CY, 20, ACCENT_COLOR);
  // Big smile
  for (int i = 0; i < 6; i++) {
    tft.drawArc(FACE_CX, MOUTH_Y - 15, 30, 40, 200 + i*2, 340 - i*2, MOUTH_COLOR);
  }
  // Open mouth
  tft.fillEllipse(FACE_CX, MOUTH_Y + 5, 8, 10, 0x9000);
}

void drawStar(int cx, int cy, int r, uint16_t color) {
  for (int i = 0; i < 5; i++) {
    float angle1 = (i * 72 - 90) * PI / 180;
    float angle2 = ((i + 1) * 72 - 90) * PI / 180;
    float angleM = ((i * 72 + 36) - 90) * PI / 180;
    int x1 = cx + r * cos(angle1) * 0.4;
    int y1 = cy + r * sin(angle1) * 0.4;
    int x2 = cx + r * cos(angleM);
    int y2 = cy + r * sin(angleM);
    int x3 = cx + r * cos(angle2) * 0.4;
    int y3 = cy + r * sin(angle2) * 0.4;
    tft.fillTriangle(x1, y1, x2, y2, x3, y3, color);
  }
  tft.fillCircle(cx, cy, r * 0.3, color);
}

void drawWorriedFace() {
  int leftX = FACE_CX - EYE_RX - EYE_GAP/2;
  int rightX = FACE_CX + EYE_RX + EYE_GAP/2;
  // Worried eyebrows (inverse of angry)
  tft.drawLine(leftX - 15, FACE_CY - 25, leftX + 25, FACE_CY - 35, 0x4208);
  tft.drawLine(rightX + 15, FACE_CY - 25, rightX - 25, FACE_CY - 35, 0x4208);
  drawEyeBase(leftX, FACE_CY, EYE_RX, EYE_RY);
  drawEyeBase(rightX, FACE_CY, EYE_RX, EYE_RY);
  drawPupil(leftX, FACE_CY, PUPIL_R, eyeOffsetX, eyeOffsetY);
  drawPupil(rightX, FACE_CY, PUPIL_R, eyeOffsetX, eyeOffsetY);
  // Wavy mouth
  tft.drawLine(FACE_CX - 20, MOUTH_Y, FACE_CX - 5, MOUTH_Y + 3, MOUTH_COLOR);
  tft.drawLine(FACE_CX - 5, MOUTH_Y + 3, FACE_CX + 5, MOUTH_Y - 2, MOUTH_COLOR);
  tft.drawLine(FACE_CX + 5, MOUTH_Y - 2, FACE_CX + 20, MOUTH_Y, MOUTH_COLOR);
}

void drawCoolFace() {
  int leftX = FACE_CX - EYE_RX - EYE_GAP/2;
  int rightX = FACE_CX + EYE_RX + EYE_GAP/2;
  // Sunglasses
  tft.fillRoundRect(leftX - EYE_RX - 5, FACE_CY - 15, EYE_RX * 2 + 10, 30, 5, TFT_BLACK);
  tft.fillRoundRect(rightX - EYE_RX - 5, FACE_CY - 15, EYE_RX * 2 + 10, 30, 5, TFT_BLACK);
  tft.fillRect(FACE_CX - 5, FACE_CY - 5, 10, 10, TFT_BLACK);
  // Reflection on glasses
  tft.drawLine(leftX - 10, FACE_CY - 8, leftX + 5, FACE_CY - 8, 0xC618);
  tft.drawLine(rightX - 10, FACE_CY - 8, rightX + 5, FACE_CY - 8, 0xC618);
  // Smirk
  tft.drawLine(FACE_CX - 15, MOUTH_Y, FACE_CX + 5, MOUTH_Y, MOUTH_COLOR);
  tft.drawLine(FACE_CX + 5, MOUTH_Y, FACE_CX + 20, MOUTH_Y - 5, MOUTH_COLOR);
}

void drawShyFace() {
  int leftX = FACE_CX - EYE_RX - EYE_GAP/2;
  int rightX = FACE_CX + EYE_RX + EYE_GAP/2;
  drawEyeBase(leftX, FACE_CY, EYE_RX, EYE_RY);
  drawEyeBase(rightX, FACE_CY, EYE_RX, EYE_RY);
  // Pupils looking down and away
  drawPupil(leftX, FACE_CY, PUPIL_R, eyeOffsetX - 8, eyeOffsetY + 8);
  drawPupil(rightX, FACE_CY, PUPIL_R, eyeOffsetX + 8, eyeOffsetY + 8);
  // Small smile
  for (int i = 0; i < 3; i++) {
    tft.drawArc(FACE_CX, MOUTH_Y - 5, 15, 20, 220 + i*2, 320 - i*2, MOUTH_COLOR);
  }
  // Heavy blush
  tft.fillCircle(leftX - 25, FACE_CY + 15, 10, BLUSH_COLOR);
  tft.fillCircle(rightX + 25, FACE_CY + 15, 10, BLUSH_COLOR);
}

void drawDeterminedFace() {
  int leftX = FACE_CX - EYE_RX - EYE_GAP/2;
  int rightX = FACE_CX + EYE_RX + EYE_GAP/2;
  // Sharp eyebrows
  tft.drawLine(leftX - 20, FACE_CY - 30, leftX + 20, FACE_CY - 25, 0x4208);
  tft.drawLine(rightX + 20, FACE_CY - 30, rightX - 20, FACE_CY - 25, 0x4208);
  drawEyeBase(leftX, FACE_CY, EYE_RX, EYE_RY);
  drawEyeBase(rightX, FACE_CY, EYE_RX, EYE_RY);
  // Pupils focused forward
  drawPupil(leftX, FACE_CY, PUPIL_R, 0, 0);
  drawPupil(rightX, FACE_CY, PUPIL_R, 0, 0);
  // Determined mouth (small flat)
  tft.drawLine(FACE_CX - 12, MOUTH_Y, FACE_CX + 12, MOUTH_Y, MOUTH_COLOR);
  tft.drawLine(FACE_CX - 12, MOUTH_Y + 1, FACE_CX + 12, MOUTH_Y + 1, MOUTH_COLOR);
}

// ==================== FACE ANIMATION SYSTEM ====================

void updateFace(unsigned long now) {
  // Smooth eye movement
  eyeOffsetX += (targetEyeOffsetX - eyeOffsetX) * 0.15;
  eyeOffsetY += (targetEyeOffsetY - eyeOffsetY) * 0.15;

  // Blinking
  if (!isBlinking && now - lastBlinkTime > nextBlinkInterval) {
    isBlinking = true;
    blinkPhase = 1;
    lastBlinkTime = now;
    nextBlinkInterval = random(2000, 6000); // Random blink interval
  }

  if (isBlinking) {
    static unsigned long blinkStart = 0;
    if (blinkPhase == 1) { // Closing
      if (now - lastBlinkTime > 80) { blinkPhase = 2; lastBlinkTime = now; }
    } else if (blinkPhase == 2) { // Closed
      if (now - lastBlinkTime > 50) { blinkPhase = 3; lastBlinkTime = now; }
    } else if (blinkPhase == 3) { // Opening
      if (now - lastBlinkTime > 80) { blinkPhase = 0; isBlinking = false; }
    }
  }

  // Redraw face with current emotion (only if changed or blinking)
  static Emotion lastDrawnEmotion = EM_NEUTRAL;
  static float lastEyeX = 999, lastEyeY = 999;
  static int lastBlink = -1;

  if (currentEmotion != lastDrawnEmotion || 
      abs(eyeOffsetX - lastEyeX) > 1 || abs(eyeOffsetY - lastEyeY) > 1 ||
      blinkPhase != lastBlink) {
    drawFace(currentEmotion);
    lastDrawnEmotion = currentEmotion;
    lastEyeX = eyeOffsetX;
    lastEyeY = eyeOffsetY;
    lastBlink = blinkPhase;
  }
}

void setEmotion(Emotion em) {
  if (currentEmotion == em) return;
  targetEmotion = em;
  currentEmotion = em;
  emotionStartTime = millis();
  drawFace(em);
}

void lookAt(int x, int y) {
  // Convert screen coordinates to eye offset
  // x,y are relative to screen center (0,0 = center)
  targetEyeOffsetX = constrain(x * 0.3, -15, 15);
  targetEyeOffsetY = constrain(y * 0.3, -15, 15);
}

void lookCenter() {
  targetEyeOffsetX = 0;
  targetEyeOffsetY = 0;
}

void lookLeft() { targetEyeOffsetX = -12; }
void lookRight() { targetEyeOffsetX = 12; }
void lookUp() { targetEyeOffsetY = -12; }
void lookDown() { targetEyeOffsetY = 12; }

// Map robot state to emotion
void updateEmotionFromState() {
  switch (robotState) {
    case IDLE:        setEmotion(EM_NEUTRAL); break;
    case MOVING:      setEmotion(EM_DETERMINED); break;
    case WAVING:      setEmotion(EM_HAPPY); break;
    case ARMS_UP:     setEmotion(EM_EXCITED); break;
    case AVOIDING:    setEmotion(EM_WORRIED); break;
    case ERROR_STATE: setEmotion(EM_DEAD); break;
    case SLEEPING:    setEmotion(EM_SLEEPY); break;
    case LISTENING:   setEmotion(EM_NEUTRAL); lookAt(0, -5); break;
    case THINKING:    setEmotion(EM_THINKING); break;
    case PATROLLING:  setEmotion(EM_DETERMINED); break;
    case EXPLORING:   setEmotion(EM_CURIOUS); break;
    case INTERACTING: setEmotion(EM_HAPPY); break;
    case FOLLOWING:   setEmotion(EM_DETERMINED); break;
  }
}

// ==================== STATUS BARS ====================

void drawTopBar() {
  tft.fillRect(0, 0, 240, TOP_BAR_H, 0x1082); // Dark blue-grey

  // Robot name (left)
  tft.setTextColor(ACCENT_COLOR, 0x1082);
  tft.setTextSize(1);
  tft.setCursor(4, 6);
  tft.print(robotName);

  // WiFi indicator (center-left)
  int wifiX = 90;
  if (WiFi.status() == WL_CONNECTED) {
    tft.fillRect(wifiX, 14, 3, 4, 0x07E0);
    tft.fillRect(wifiX + 4, 11, 3, 7, 0x07E0);
    tft.fillRect(wifiX + 8, 8, 3, 10, 0x07E0);
    tft.fillRect(wifiX + 12, 5, 3, 13, 0x07E0);
  } else {
    tft.drawLine(wifiX, 5, wifiX + 15, 18, 0xF800);
    tft.drawLine(wifiX + 15, 5, wifiX, 18, 0xF800);
  }

  // State text (center)
  tft.setTextColor(TFT_WHITE, 0x1082);
  tft.setCursor(125, 6);
  tft.print(stateToString(robotState));

  // Battery icon (right)
  drawBatteryIcon(200, 4, batteryPercent);
}

void drawBottomBar() {
  int y = 240 - BOTTOM_BAR_H;
  tft.fillRect(0, y, 240, BOTTOM_BAR_H, 0x1082);

  // Distance indicator
  tft.setTextColor(0x867D, 0x1082);
  tft.setCursor(4, y + 4);
  tft.print("D:");
  tft.print(distanceMm);
  tft.print("mm");

  // Person indicator
  tft.setCursor(90, y + 4);
  if (personInView) {
    tft.setTextColor(0xF809, 0x1082);
    tft.print("PERSON!");
  } else {
    tft.setTextColor(0x867D, 0x1082);
    tft.print("Clear");
  }

  // Goal
  tft.setCursor(160, y + 4);
  tft.setTextColor(0x867D, 0x1082);
  tft.print("G:");
  tft.print(goalToString(currentGoal));
}

void drawBatteryIcon(int x, int y, float percent) {
  // Battery outline
  tft.drawRect(x, y, 24, 12, TFT_WHITE);
  tft.fillRect(x + 24, y + 3, 3, 6, TFT_WHITE);

  // Fill level
  int fillW = (int)((24 - 4) * percent / 100.0);
  uint16_t color = (percent > 50) ? 0x07E0 : (percent > 20) ? 0xFFE0 : 0xF800;
  tft.fillRect(x + 2, y + 2, fillW, 8, color);

  // Percent text
  tft.setTextColor(TFT_WHITE, 0x1082);
  tft.setTextSize(1);
  tft.setCursor(x + 30, y + 2);
  tft.print((int)percent);
  tft.print("%");
}

// ==================== SPEECH DISPLAY ====================

#define SPEECH_Y      215
#define SPEECH_MAX_W  220

void showSpeech(String text) {
  // Draw speech bubble at bottom of face area
  int bubbleY = SPEECH_Y;
  int padding = 6;

  // Calculate text width
  tft.setTextSize(1);
  int textW = tft.textWidth(text);
  int bubbleW = min(textW + padding * 2, SPEECH_MAX_W);
  int bubbleX = (240 - bubbleW) / 2;

  // Clear area
  tft.fillRect(bubbleX - 2, bubbleY - 2, bubbleW + 4, 18, FACE_BG);

  // Draw bubble
  tft.fillRoundRect(bubbleX, bubbleY, bubbleW, 16, 4, 0x2104);
  tft.drawRoundRect(bubbleX, bubbleY, bubbleW, 16, 4, ACCENT_COLOR);

  // Text
  tft.setTextColor(TFT_WHITE, 0x2104);
  tft.setCursor(bubbleX + padding, bubbleY + 4);

  // Scroll if too long
  if (textW > SPEECH_MAX_W - padding * 2) {
    // Show scrolling text (simplified: just show first part)
    tft.print(text.substring(0, 25));
    tft.print("...");
  } else {
    tft.print(text);
  }
}

void clearSpeech() {
  tft.fillRect(10, SPEECH_Y - 2, 220, 20, FACE_BG);
}

// ==================== BOOT ANIMATION ====================

void playBootAnimation() {
  tft.fillScreen(TFT_BLACK);

  // WALL-E text animation
  tft.setTextColor(ACCENT_COLOR, TFT_BLACK);
  tft.setTextSize(3);
  String bootText = "WALL-E";
  for (int i = 0; i <= bootText.length(); i++) {
    tft.setCursor(60, 80);
    tft.print(bootText.substring(0, i));
    delay(150);
  }

  // Loading bar
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(80, 120);
  tft.print("AI Brain v4.0");

  int barY = 150;
  int barW = 160;
  int barX = (240 - barW) / 2;
  tft.drawRect(barX, barY, barW, 12, ACCENT_COLOR);

  String stages[] = {"Memory", "Sensors", "Vision", "Network", "AI"};
  for (int i = 0; i < 5; i++) {
    int fillW = (barW - 4) * (i + 1) / 5;
    tft.fillRect(barX + 2, barY + 2, fillW, 8, ACCENT_COLOR);
    tft.setCursor(barX + barW/2 - 20, barY + 18);
    tft.print(stages[i]);
    delay(400);
    tft.fillRect(barX + barW/2 - 25, barY + 18, 50, 10, TFT_BLACK);
  }

  delay(300);
  tft.fillScreen(TFT_BLACK);

  // Wake up face sequence
  setEmotion(EM_SLEEPY);
  delay(800);
  setEmotion(EM_NEUTRAL);
  delay(300);
  setEmotion(EM_HAPPY);
  delay(500);
  setEmotion(EM_NEUTRAL);

  drawTopBar();
  drawBottomBar();
}

// ==================== SLEEP BREATHING ANIMATION ====================

void drawSleepBreathing(unsigned long now) {
  static int breathPhase = 0;
  breathPhase = (now / 800) % 2;

  tft.fillRect(0, FACE_AREA_Y, 240, FACE_AREA_H, FACE_BG);

  int leftX = FACE_CX - EYE_RX - EYE_GAP/2;
  int rightX = FACE_CX + EYE_RX + EYE_GAP/2;
  int breathOffset = breathPhase * 3;

  // Closed eyes with slight movement
  tft.drawLine(leftX - EYE_RX, FACE_CY + breathOffset, leftX + EYE_RX, FACE_CY + breathOffset, EYE_WHITE);
  tft.drawLine(rightX - EYE_RX, FACE_CY + breathOffset, rightX + EYE_RX, FACE_CY + breathOffset, EYE_WHITE);

  // Small mouth
  tft.drawLine(FACE_CX - 6, MOUTH_Y + breathOffset, FACE_CX + 6, MOUTH_Y + breathOffset, MOUTH_COLOR);

  // Zzz
  int zY = FACE_CY - 30 - ((now / 600) % 3) * 12;
  tft.setTextColor(0x867D, FACE_BG);
  tft.setTextSize(2);
  tft.setCursor(rightX + 25, zY);
  tft.print("Z");
}

// ==================== REACTION SYSTEM ====================

void reactToEvent(String eventType) {
  if (eventType == "person_seen") {
    setEmotion(EM_SURPRISED);
    lookAt(0, 0);
    delay(400);
    setEmotion(EM_HAPPY);
    showSpeech("Hello!");
  } else if (eventType == "obstacle") {
    setEmotion(EM_WORRIED);
    showSpeech("Oh!");
    delay(500);
    setEmotion(EM_DETERMINED);
  } else if (eventType == "greeting") {
    setEmotion(EM_HAPPY);
    showSpeech("Nice to meet you!");
  } else if (eventType == "low_battery") {
    setEmotion(EM_SAD);
    showSpeech("I'm tired...");
  } else if (eventType == "wake_up") {
    setEmotion(EM_SURPRISED);
    delay(300);
    setEmotion(EM_HAPPY);
    showSpeech("I'm awake!");
  } else if (eventType == "thinking") {
    setEmotion(EM_THINKING);
    showSpeech("Hmm...");
  } else if (eventType == "confused") {
    setEmotion(EM_CONFUSED);
    showSpeech("I don't understand");
  } else if (eventType == "love") {
    setEmotion(EM_LOVE);
    showSpeech("I love you!");
  } else if (eventType == "angry") {
    setEmotion(EM_ANGRY);
    showSpeech("Hey!");
    delay(500);
    setEmotion(EM_NEUTRAL);
  }
}

// ==================== SCREEN REFRESH ====================

void refreshScreen() {
  drawTopBar();
  drawBottomBar();
  updateFace(millis());
}

// ==================== REPLACEMENT FOR showFace() ====================
// Use this instead of the old text-based showFace()
void showFace(String faceText) {
  // Map text faces to emotions
  if (faceText == "^_^") setEmotion(EM_HAPPY);
  else if (faceText == "O_O" || faceText == "o_o") setEmotion(EM_SURPRISED);
  else if (faceText == "-_-") setEmotion(EM_SLEEPY);
  else if (faceText == "X_X" || faceText == "!_!") setEmotion(EM_DEAD);
  else if (faceText == "T_T") setEmotion(EM_SAD);
  else if (faceText == ">_<") setEmotion(EM_ANGRY);
  else if (faceText == "@_@") setEmotion(EXCITED);
  else setEmotion(EM_NEUTRAL);

  drawTopBar();
  drawBottomBar();
}
