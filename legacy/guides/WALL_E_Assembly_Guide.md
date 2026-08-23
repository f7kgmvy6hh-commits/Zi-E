# WALL-E Assembly Guide — Complete Physical Build
## المرجع النهائي للتجميع الميكانيكي والكهربائي

---

## 🔧 القطع المطلوب طباعتها (STL Files)

### 1. Trapezoidal Body (`body.stl`)
| Parameter | Value |
|-----------|-------|
| **Base** | 63mm (bottom) → 29mm (top) |
| **Height** | ~45mm |
| **Wall thickness** | 3mm (minimum) |
| **Infill** | 25% Gyroid |
| **Layer height** | 0.2mm |
| **Material** | PLA (rigid, easy to print) |

**Cutouts needed (after printing or in CAD):**
- Rectangular slot for TFT screen (front face)
- Round hole for camera lens (front face, centered)
- Small holes for speaker sound (bottom)
- Wire routing holes (back, 5mm diameter)
- SD card slot (side, if using external module)

### 2. Head Box (`head_box.stl`)
| Parameter | Value |
|-----------|-------|
| **Dimensions** | 60×35×45mm |
| **Wall thickness** | 2.5mm |
| **Infill** | 20% |

**Cutouts:**
- Camera: 8.5×8.5mm square (OV2640 module)
- TFT: 28×35mm rectangle (1.5" ST7789)
- Speaker: 28mm diameter hole (bottom)
- Neck mount: 2× M2 holes, 15mm spacing

### 3. Neck Platform (`neck_platform.stl`)
| Parameter | Value |
|-----------|-------|
| **Dimensions** | 29×20×3mm |
| **Mount holes** | 2× Ø2.2mm for M2 screws |
| **Servo mount** | SG90 flange pattern |

### 4. Arm Brackets (Left + Right) (`arm_L.stl`, `arm_R.stl`)
| Parameter | Value |
|-----------|-------|
| **Dimensions** | 24×20×9mm |
| **Wall thickness** | 3mm |
| **Mount holes** | 2× Ø2.2mm (M2 screws) |
| **Servo pocket** | SG90 body fit |

### 5. Arm Links (`arm_link.stl`) ×2
| Parameter | Value |
|-----------|-------|
| **Dimensions** | 32×8×3mm |
| **End holes** | Ø2.2mm |
| **Material** | PLA or brass wire reinforcement |

---

## 📐 زوايا التركيب والأبعاد الحرجة

### زاوية الكاميرا
```
الكاميرا يجب أن تكون:
  • عمودية تماماً (90° على الأفق)
  • مركزة على محور الجسم (centerline)
  • عدسة بارزة 1mm عن سطح الصندوق
  • لا انعكاس ضوء داخلي (paint inside black)
```

### زاوية الشاشة (TFT)
```
الشاشة يجب أن تكون:
  • مائلة 10° للأعلى (تجاه وجه المستخدم)
  • مركزة أفقياً
  • مسافة 2mm من الحافة العلوية
  • سلك FPC (إذا وجد) يخرج من الخلف
```

### زاوية المحركات (N20)
```
المحرك يجب أن يكون:
  • عمودي تماماً على قاعدة الجسم
  • عمود محرك موازٍ لمحور الحركة
  • مسافة 5mm من الجدار الداخلي
  • مسامير M2×8mm لتثبيت الأذرع
```

### زاوية الذراع (Servo Horn)
```
الذراع عند HOME (90°):
  • عمودية على جانب الجسم
  • مسافة 7mm من جانب الجسم
  • ارتفاع 24mm عن قاعدة الجسم
  • حركة آمنة: 45° (للأسفل) ← 90° (HOME) → 135° (للأعلى)
  • مسافة أمان ≥5mm من المسارات والشاشة
```

---

## 🔌 دليل الأسلاك الكامل (Wire Routing)

### ألوان الأسلاك الموصى بها
| اللون | الدالة | متصل بـ |
|-------|--------|---------|
| 🔴 أحمر | بطارية خام (3.0-4.2V) | TP4056 → DRV8833 VM |
| ⚫ أسود | GND مشترك | ALL modules |
| 🟡 أصفر | 5V منظم | MT3608 → XIAO, PCA9685, sensors |
| 🟢 أخضر | 3.3V I/O | XIAO pins |
| 🔵 أزرق | I2C SDA | D0 |
| ⚪ أبيض | I2C SCL | D1 |
| 🟣 بنفسجي | PWM signal | PCA9685 → servos |
| 🟠 برتقالي | Motor A | DRV8833 → N20 |
| 🩶 رمادي | Motor B | DRV8833 → N20 |

### ترتيب التوصيل خطوة بخطوة

#### المرحلة 1: القوة (Power Backbone)
```
[بطارية 18650 ×2 parallel]
         │
    [TP4056]
    │      │
    │      ├──→ [INA219] (measurement)
    │      │
    │      ├──→ [DRV8833 VM pin] (motors)
    │      │
    │      └──→ [MT3608 Boost]
    │              │
    │              └──→ [5V BUS]
    │                      │
    │                      ├──→ [XIAO ESP32S3] 5V pin
    │                      ├──→ [PCA9685] V+ pin
    │                      ├──→ [ST7789] VCC
    │                      ├──→ [MAX98357A] VIN
    │                      ├──→ [VL53L0X] VIN
    │                      └──→ [470µF Capacitor]
    │
    └──→ [Common GND rail] ←── ALL GND pins connect here
```

**⚠️ تحذيرات القوة:**
1. **لا** توصل محركات من 5V المنظم — استخدم بطارية خام
2. **لا** تشارك GND محركات مع GND حساسة بدون نقطة مرجعية واحدة
3. **ضع** المكثف 470µF بالقرب من PCA9685 (أقل من 2cm)
4. **اضبط** MT3608 بمقياس متعدد قبل توصيل أي شيء

#### المرحلة 2: I2C Bus
```
XIAO D0 (SDA) ──┬──→ PCA9685 SDA
                ├──→ VL53L0X SDA
                ├──→ INA219 SDA
                ├──→ AHT20 SDA (optional)
                └──→ MPU6050 SDA (optional)

XIAO D1 (SCL) ──┬──→ PCA9685 SCL
                ├──→ VL53L0X SCL
                ├──→ INA219 SCL
                ├──→ AHT20 SCL (optional)
                └──→ MPU6050 SCL (optional)
```

**⚠️ I2C Requirements:**
- Pull-up resistors 4.7kΩ on SDA and SCL (if modules don't have them)
- Max wire length: 30cm per segment
- Use twisted pair or keep wires close together

#### المرحلة 3: PCA9685 → Servos
```
PCA9685 CH0 ──→ Head Pan SG90 (signal wire)
PCA9685 CH1 ──→ Head Tilt SG90
PCA9685 CH2 ──→ DRV8833 AIN1 (left motor logic)
PCA9685 CH3 ──→ DRV8833 AIN2
PCA9685 CH4 ──→ DRV8833 BIN1 (right motor logic)
PCA9685 CH5 ──→ DRV8833 BIN2
PCA9685 CH6 ──→ Left Arm SG90
PCA9685 CH7 ──→ Right Arm SG90

PCA9685 V+ ──→ 5V BUS (servo power)
PCA9685 GND ──→ Common GND
PCA9685 VCC ──→ 3.3V (logic power)
```

**⚠️ Servo Wiring:**
- Brown/Black = GND
- Red = 5V (from PCA9685 V+, NOT from XIAO!)
- Orange/Yellow = Signal (from PCA9685 CHx)

#### المرحلة 4: Camera & Audio
```
XIAO ESP32S3 Sense (built-in camera connector):
  → OV2640 module (direct connection, no soldering needed)

Audio I2S:
  XIAO D5 (BCK/SCK) ──→ MAX98357A BCLK
  XIAO D6 (DATA) ──→ MAX98357A DIN
  XIAO D7 (WS/LRCK) ──→ MAX98357A LRC
  MAX98357A GAIN ──→ GND (9dB gain)
  MAX98357A SD ──→ 3.3V (shutdown disable)
  MAX98357A OUT+ ──→ Speaker+
  MAX98357A OUT- ──→ Speaker-
```

#### المرحلة 5: SD Card
```
If using external SD module:
  XIAO D2 ──→ CS
  XIAO D8 ──→ SCK
  XIAO D9 ──→ MISO
  XIAO D10 ──→ MOSI
  3.3V ──→ VCC
  GND ──→ GND

If using XIAO built-in SD slot:
  → Just insert SD card, no wiring needed!
```

---

## 🧪 تسلسل الاختبار (Test Sequence)

### ⚠️ قبل كل شيء: اختبار البريدبورد
**لا تركب أي شيء في الجسم قبل الاختبار الكامل على بريدبورد!**

#### Test 1: Power Only (No Load)
1. وصل TP4056 + MT3608 فقط
2. لا شيء متصل على 5V output
3. قيس الجهد: يجب أن يكون 5.00V ±0.05V
4. إذا لم يكن 5V: اضبط MT3608 بالمفك الصغير

#### Test 2: I2C Scan
1. وصل XIAO + PCA9685 فقط
2. ارفع الكود مع Serial Monitor
3. يجب أن ترى:
   ```
   I2C Scan:
     Found: 0x40
   ```
4. أضف INA219 → يجب أن ترى `0x41`
5. أضف VL53L0X → يجب أن ترى `0x29`

#### Test 3: One Servo
1. وصل SG90 واحد على CH0 (Pan)
2. الكود يحركه: 90° → 0° → 180° → 90°
3. يجب أن يكون الحركة سلسة
4. إذا اهتز: أضف 470µF مكثف

#### Test 4: All Servos
1. وصل كل 4 servos
2. شغل home sequence
3. تحقق من: Pan, Tilt, Left Arm, Right Arm
4. **يدويًا**: حرك كل ذراع من 45° إلى 135° بدون قوة

#### Test 5: Motors (Robot Lifted!)
1. **ارفع الروبوت عن الأرض!**
2. وصل DRV8833 + N20 motors
3. اختبر: forward, backward, left, right
4. إذا ضعيف: قيس VM voltage تحت الحمل

#### Test 6: Sensors
1. ToF: يجب أن يقرأ distance بشكل صحيح
2. INA219: يجب أن يقرأ voltage/current
3. Camera: افتح `/stream` في المتصفح

#### Test 7: Audio
1. ارسل `speakTTS("test")`
2. يجب أن تسمع صوت من السماعة
3. إذا مشوش: افصل GND السماعة عن GND الميكروفون

#### Test 8: Full Integration
1. ركب كل شيء في الجسم
2. شغل البرنامج الكامل
3. افتح Dashboard
4. اختبر كل button

---

## 🛠️ نصائح التركيب العملية

### ترتيب التركيب الميكانيكي
```
1. اطبع كل القطع
2. ركب المحركات في القاعدة (لا تربط الأسلاك بعد)
3. ركب مسارات العجلات
4. ركب منصة الرقبة
5. ركب servos الرأس (Pan/Tilt)
6. ركب صندوق الرأس
7. ركب الكاميرا والشاشة في الرأس
8. ركب أذرع السيرفو في الجانبين
9. ركب الذراعات
10. اختبر الحركة اليدوية لكل شيء
11. الآن ابدأ في الأسلاك!
```

### إدارة الأسلاك داخل الجسم
```
• استخدم شريط لاصق (kapton tape) لتثبيت الأسلاك
• اترك 2cm extra wire عند كل servo (للحركة)
• جمع الأسلاك في حزم (bundles) حسب الوظيفة
  - حزمة القوة (أحمر/أسود/أصفر)
  - حزمة I2C (أزرق/أبيض)
  - حزمة السيرفو (بنفسجي)
• استخدم zip ties كل 3cm
• اترك "service loop" عند كل servo
```

### تجنب الضوضاء الكهربائية
```
• حافظ على أسلاك المحركات بعيدة عن I2C
• لف أسلاك المحركات حول حلقة Ferrite (إن وجد)
• المكثف 470µF يجب أن يكون أقرب ما يمكن من PCA9685
• Common GND = نقطة واحدة فقط!
```

---

## 📋 قائمة التحقق النهائية قبل التشغيل

### ميكانيكي
- [ ] كل المسامير مشدودة (M2, M2.5)
- [ ] لا احتكاك بين الأجزاء المتحركة
- [ ] ≥5mm مسافة أمان للذراعين
- [ ] المسارات تدور بحرية
- [ ] الرأس يتحرك في نطاقه الكامل

### كهربائي
- [ ] 5V = 5.00V ±0.05V (بالمقياس)
- [ ] VM (محركات) > 3.5V تحت الحمل
- [ ] GND مشترك في نقطة واحدة
- [ ] لا أسلاك عارية تلامس بعضها
- [ ] المكثف 470µF مثبت بشكل صحيح
- [ ] SD card موضوعة

### برمجي
- [ ] WiFi متصل
- [ ] I2C scan يجد كل الأجهزة
- [ ] Camera stream يعمل
- [ ] Dashboard يفتح
- [ ] كل animation يعمل
- [ ] Obstacle stop يعمل
- [ ] Battery reading منطقي

---

## 🚨 استكشاف الأخطاء الشائعة

| المشكلة | السبب المحتمل | الحل |
|---------|--------------|------|
| ESP32 يعيد التشغيل | Brownout (قلة جهد) | أضف مكثف 470µF، افصل المحركات |
| السيرفو يهتز | جهد ضعيف أو ضوضاء | تحقق من 5V، أضف مكثف |
| السيرفو ساخن | احتكاك ميكانيكي | توقف! تحقق من Clearance |
| I2C لا يعمل | لا pull-up أو wiring خاطئ | أضف 4.7kΩ على SDA/SCL |
| المحركات ضعيفة | VM منخفض تحت الحمل | استخدم بطارية خام، لا 5V |
| الكاميرا سوداء | wiring خاطئ أو config | تحقق من pins في firmware |
| الصوت مشوش | Ground loop | عزل GND السماعة |
| WiFi لا يتصل | بعيد عن الراوتر | أضف هوائي خارجي |
| الشاشة بيضاء | Driver خاطئ | تحقق من TFT_eSPI User_Setup.h |

---

**بعد ما تكمل هالقائمة، WALL-E صار جاهز يحيا! 🎉**
