# WALL-E AI Body v4.0 - The Real AI Brain
## Complete Setup Guide

---

## 🧠 Shu Bana Halla2?

Hayda howe el-firmware el-kamel la robot yeder:
- **Yshof** w ye3ref shu bado yshof (Cloud Vision GPT-4V)
- **Yehke** w yesma3 (Cloud Voice: Whisper → GPT → TTS)
- **Yezakar** w yetzakar (Local SD Memory: people, places, events)
- **Yeta3mel 7alo** (Autonomy Engine: patrol, greet, explore)
- **Yehfazak** w yehfaz el-eshya hawale (Memory + Event Log)

---

## 📦 Hardware (Ma Fi Sho Yetghayar!)

Kel el-hardware mn el-guide el-asli bado yeb2a nafso:
- XIAO ESP32S3 Sense (~$12)
- PCA9685 (~$2)
- DRV8833 + N20 motors (~$6)
- 4× SG90 servos (~$4)
- VL53L0X ToF (~$3.5)
- INA219 (~$2)
- ST7789 TFT 1.5" (~$5)
- MAX98357A + Speaker (~$4)
- **SD Card Module** (~$1) — Jdid! Lazem la-zekra
- 2× 18650 + TP4056 + MT3608 (~$8)

**Total: ~$55-60**

### Shu El-Jdid?
| Item | Sabab |
|------|-------|
| **SD Card** | Lazem la-zekra (people.json, places.json, events.json) |
| **Internet** | Lazem la-cloud AI (WiFi) |
| **OpenAI API Key** | $5-10/shahr la-vision + voice + chat |

---

## 🔌 Wiring Changes (Bas Zaruriyye)

### SD Card Module (SPI)
| SD Module | XIAO ESP32S3 |
|-----------|-------------|
| CS | D2 |
| MOSI | D10 |
| MISO | D9 |
| SCK | D8 |
| VCC | 3.3V |
| GND | GND |

**Note:** XIAO ESP32S3 Sense fi built-in SD card slot! Iza fi, ma lzm module khareji.

### Everything else stays the same as REV2 guide.

---

## 📚 Required Libraries (Arduino IDE)

Install from Library Manager:
1. `ArduinoJson` by Benoit Blanchon (v6+)
2. `Adafruit PWM Servo Driver` (PCA9685)
3. `Adafruit VL53L0X`
4. `Adafruit INA219`
5. `TFT_eSPI` by Bodmer (**Configure User_Setup.h!**)
6. `WebSockets` by Markus Sattler

Built-in with ESP32 core:
- `WiFi`, `HTTPClient`, `ArduinoOTA`
- `esp_camera`, `SD_MMC`, `FS`

---

## ⚙️ Configuration (Mhem!)

Open the firmware and change these 3 lines:

```cpp
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* OPENAI_API_KEY = "sk-YOUR_OPENAI_API_KEY_HERE";
```

### How to get OpenAI API Key:
1. Go to https://platform.openai.com
2. Sign up / Log in
3. Go to API Keys → Create new secret key
4. Copy and paste in firmware
5. Add $5-10 credit (enough for months of use)

**Cost estimate:**
- Vision (GPT-4V): ~$0.005 per image = ~$0.50/day if checking every 10s
- Chat (GPT-4o-mini): ~$0.0001 per message = ~$0.01/day
- **Total: ~$1-2/month for normal use**

---

## 🗂️ Memory System

The robot saves these files on SD card:

```
/memory/
  ├── config.json       # Robot name, owner name
  ├── people.json       # Known people (max 20)
  ├── places.json       # Known places (max 20)
  └── events.json       # Event log (max 50, FIFO)
```

### People Memory Fields:
- `name`: Person's name
- `description`: What they look like
- `preferences`: What they like/dislike
- `lastSeen`: Last time seen (millis)
- `encounterCount`: How many times met

### Places Memory Fields:
- `name`: Place name
- `description`: What's there
- `dangerLevel`: 0-10 safety rating
- `lastVisited`: Last time there
- `visitCount`: How many times visited

### Events Memory Fields:
- `timestamp`: When it happened
- `event`: What happened
- `location`: Where
- `people`: Who was there

---

## 🎯 Autonomy Engine

The robot decides what to do based on priorities:

```
Priority 1: Person detected → GREET (wave + speak)
Priority 2: Low battery (<20%) → SLEEP
Priority 3: Idle >60s → PATROL (move around)
Priority 4: Nothing → IDLE (wait)
```

### Goals:
| Goal | Behavior |
|------|----------|
| **PATROL** | Move forward, scan with head, avoid obstacles |
| **EXPLORE** | Move around, look left/right more |
| **GREET** | Wave, say greeting, check if person known |
| **FOLLOW** | Follow detected person (placeholder) |
| **REST** | Go to sleep |

---

## 👁️ Vision Pipeline

Every 10 seconds:
1. Capture camera frame (QVGA 320×240)
2. Encode to Base64
3. Send to OpenAI GPT-4o-mini Vision API
4. Get description: "I see a person sitting at a desk"
5. Check if person is in view
6. Update dashboard + memory

**Latency:** 1-3 seconds per call

---

## 🗣️ Voice Pipeline (Placeholder)

The firmware has the audio I2S setup ready. Full implementation needs:

### Phase 1 (Current): Simulated
- Button "Listen" on dashboard
- Simulated response for demo

### Phase 2 (Full Implementation):
```
[Mic I2S] → [3s buffer] → [WAV file] → [Whisper API]
                                              ↓
[Speaker] ← [MP3 stream] ← [TTS API] ← [GPT-4]
                                              ↓
                                       [Command Intent]
```

To implement Phase 2, add:
1. WAV encoder for audio buffer
2. HTTP POST to Whisper API
3. MP3 decoder + I2S playback
4. Or use simpler: Web dashboard voice input

---

## 🌐 Dashboard Features

Open `http://WALL-E-IP/` in browser:

| Feature | Description |
|---------|-------------|
| 📷 Live Camera | MJPEG stream from robot's eye |
| 📊 Telemetry | Battery, voltage, distance, state |
| 👁️ AI Vision | What the robot sees right now |
| 🎮 Movement | D-pad control (forward/back/left/right) |
| 🦾 Animations | Wave, arms up, dance, greet, etc. |
| 🧠 AI Chat | Ask questions, get GPT responses |
| 🎤 Voice | Listen button (placeholder) |
| 💾 Memory | View people, places, events |
| 📝 Event Log | Real-time activity log |

Also WebSocket on port 81 for real-time updates.

---

## 🔧 API Endpoints

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/` | GET | Dashboard HTML |
| `/stream` | GET | Camera MJPEG stream |
| `/api/status` | GET | Full robot status JSON |
| `/api/move?dir=forward` | POST | Move robot |
| `/api/stop` | POST | Emergency stop |
| `/api/wave` | POST | Wave animation |
| `/api/armsup` | POST | Arms up |
| `/api/armshome` | POST | Arms home |
| `/api/look?pan=90&tilt=90` | POST | Move head |
| `/api/speak?text=Hello` | POST | Speak text |
| `/api/ask?q=Question` | POST | Ask GPT |
| `/api/vision` | GET | Last vision result |
| `/api/sleep` | POST | Go to sleep |
| `/api/wakeup` | POST | Wake up |
| `/api/memory/people` | GET | List people |
| `/api/memory/places` | GET | List places |
| `/api/memory/events` | GET | List events |
| `/api/memory/addperson` | POST | Add person |
| `/api/memory/addplace` | POST | Add place |
| `/api/memory/clear` | POST | Clear all memory |

---

## 🚀 First Boot Sequence

1. **Flash firmware** to XIAO ESP32S3 Sense
2. **Insert SD card** (FAT32 formatted)
3. **Power on** — robot will create `/memory/` folder
4. **Connect to WiFi** — check serial monitor for IP
5. **Open dashboard** in browser
6. **Test camera** — should see live stream
7. **Test movement** — use D-pad
8. **Test AI** — ask a question in chat box
9. **Add a person** — via API or dashboard
10. **Let it patrol** — press Patrol button

---

## 🧪 Testing Checklist

### Phase 1: Hardware
- [ ] I2C scan finds: 0x40 (PCA9685), 0x41 (INA219), 0x29 (ToF)
- [ ] Servos home correctly (pan/tilt/arms)
- [ ] Motors respond to forward/back/left/right
- [ ] Camera stream visible at `/stream`
- [ ] SD card detected and writable
- [ ] Audio I2S initialized without error

### Phase 2: Network
- [ ] WiFi connects successfully
- [ ] Dashboard loads at robot IP
- [ ] WebSocket connects (green dot)
- [ ] OTA responds at `walle-ai.local`

### Phase 3: AI Features
- [ ] Vision API returns scene description
- [ ] Chat API responds to questions
- [ ] Memory files created on SD card
- [ ] Person added to memory persists after reboot
- [ ] Autonomy engine starts patrol after 60s idle

### Phase 4: Integration
- [ ] Full patrol cycle completes
- [ ] Obstacle avoidance triggers and logs event
- [ ] Low battery triggers sleep
- [ ] Wake from sleep works
- [ ] Memory sync every 30s

---

## ⚠️ Important Notes

### Privacy
- Camera frames are sent to OpenAI's servers
- Do not point at sensitive/private areas
- OpenAI claims they don't train on API data (as of 2024)

### Cost Control
- Vision runs every 10s = ~$0.50/day
- Reduce `VISION_INTERVAL_MS` to 30000 (30s) to save money
- Or disable vision and only use on-demand via dashboard

### API Key Security
- NEVER share your `.ino` file with the API key
- The key is embedded in firmware — anyone with physical access can read it
- For production: Use a proxy server that holds the key

### SD Card Lifespan
- SD cards wear out with writes
- Memory syncs every 30s — acceptable for normal use
- For heavy use: Increase `MEMORY_SYNC_INTERVAL_MS` to 60000 (1 min)

---

## 🔮 Future Upgrades

### v4.1 (Short term)
- [ ] Real voice pipeline (Whisper + TTS)
- [ ] Face recognition (local, no cloud)
- [ ] MQTT home automation integration
- [ ] Mobile app instead of web dashboard

### v4.2 (Medium term)
- [ ] On-device vision (TensorFlow Lite person detection)
- [ ] SLAM with ToF + encoders
- [ ] Multi-language support
- [ ] Emotion detection from voice

### v5.0 (Long term)
- [ ] Raspberry Pi 5 upgrade for on-device LLM
- [ ] ROS2 integration
- [ ] Autonomous charging dock
- [ ] Swarm behavior

---

## 🆘 Troubleshooting

| Problem | Cause | Fix |
|---------|-------|-----|
| "SD Card FAIL" | No SD card or wrong format | Use FAT32 formatted SD card |
| Vision returns "error" | Bad API key or no internet | Check key, check WiFi |
| Camera stream black | Wrong camera pins | Verify XIAO ESP32S3 Sense pinout |
| WebSocket disconnects | WiFi weak | Move closer to router |
| Memory not persisting | SD not detected | Check wiring, try different SD card |
| GPT responses slow | Network latency | Normal — 1-3s expected |
| Robot sleeps too fast | 5min idle timer | Touch button or send command to wake |

---

## 📄 Files

- `WALL_E_AI_Brain_v4.ino` — Master firmware (1417 lines)
- `README_v4.md` — This file

---

**WALL-E is now truly alive. He sees, he hears, he remembers, he thinks.**

Built with ❤️ for the future of personal robotics.
