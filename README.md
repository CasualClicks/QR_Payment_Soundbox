<!-- Generated using AI to save time -->

# 🔊 QR Payment Soundbox

A DIY **QR Payment Soundbox** built using an ESP32 that announces payment confirmations in real-time — matching the performance of commercial UPI soundboxes, but engineered for both **personal (non-merchant) and scalable future use cases**.

---

## 🚀 Overview

This project enables real-time, dynamic **audio announcements for UPI payments** by:

- Subscribing to instant payment payloads via MQTT.
- Parsing numerical amounts and currency fragments asynchronously.
- Outputting clean audio pacing structures through a local hardware decoding matrix.

### Current Support Status:
- 🧑‍💻 **Non-Merchant Mode (Fully Working):** Mobile Phone (SMS Notification App) ➡️ MQTT Broker ➡️ ESP32 WiFi/GPRS Node.
- 🏪 **Merchant Standalone Mode (Hardware Ready / Code Pending):** Circuit architecture accommodates an onboard GSM module for future standalone decoding, currently awaiting firmware implementation.

---

## 📺 Project Demonstration

<div align="center">
  <a href="YOUR_YOUTUBE_VIDEO_URL_HERE">
    <img src="https://img.youtube.com/vi/YOUR_YOUTUBE_VIDEO_ID/0.jpg" alt="Watch the Soundbox Demo" width="80%" style="border-radius: 10px;">
    <br>
    <b>▶️ Click to watch the Soundbox boot up, handle cellular authentication, and process a live transaction payload test</b>
  </a>
</div>

---

## 🧩 Features

- 🔊 **Dynamic Audio Stitching:** Glitch-free compounding playback (e.g., *"Amount received: One Hundred and Fifty Rupees and Fifty Paise"*).
- 📶 **Network Tunnelling:** ESP32 establishes internet access over cellular GPRS (SIM800L) to pull cloud events without local WiFi infrastructure.
- 🎛️ **Compact Amplification Loop:** Minimalist, knobless audio pipeline maximizing space-efficiency.
- 🧠 **Non-Blocking Queue:** Dedicated software state machine managing precise multi-track spacing using dummy timer folders.
- 🧱 **Industrial Form Factor:** Custom-fitted, high-stability 3D printed enclosure.

---

## 🛠️ Hardware Components

| Component | Functional Purpose |
|----------|--------|
| **ESP32 Core Module** | Central processing unit handling network state machines and audio flow control. |
| **DFPlayer Mini** | Hardware MP3/WAV storage subsystem and local DAC decoder. |
| **PAM8403 Module** | Knobless Mini Class-D Audio Power Amplifier delivering maximum power density. |
| **3W 4Ω Speaker** | Acoustically matched low-impedance audio driver. |
| **SIM800L Core** | GSM/GPRS module providing cellular data connectivity to the MQTT server. |
| **MT3608 DC-DC Boost** | High-transient voltage rail stabilization (Step-up to distinct 4V/5V system domains). |
| **18650 Battery Array** | Core high-discharge portable power cell layout. |
| **Decoupling Capacitors** | Focused $1000\mu F$ low-ESR buffer sitting directly across the cellular power rail. |

---

## 🧠 System Architecture

### 🔹 Non-Merchant Mode (Active Infrastructure)
Payment SMS → Mobile Phone → MQTT Server → ESP32 → Audio Output
1. Payment landing triggers a transactional SMS on a personal smartphone.
2. A background application on the phone reads the broadcast notification and posts a raw string token to the Adafruit IO MQTT broker.
3. The standalone soundbox connects via SIM800L GPRS, extracts the payload value, sanitizes the float bounds, and initiates the local speech thread.

### 🔹 Standalone Merchant Mode (Future Capability)
- SIM800 receives SMS directly  
- ESP32 processes message  
- Fully independent system  
- **Current State:** The wiring matrix accommodates hardware mapping to capture cellular broadcasts natively, but firmware parsing logic is currently placeholder code slated for a future update.
---

## 🔌 Wiring Overview
### 🎛️ Microcontroller Routing Matrix
```cpp
// Hardware Serial 1 Allocation (Cellular Core)
#define ESP_RX_SIM_TX 4  // Connect to SIM800L TX
#define ESP_TX_SIM_RX 5  // Connect to SIM800L RX (Use 1k resistor inline)

// Hardware Serial 2 Allocation (Audio Core)
#define ESP_RX_DF_TX  27 // Connect to DFPlayer Mini TX
#define ESP_TX_DF_RX  25 // Connect to ESP32 TX ---> DFPlayer RX (Use 1k resistor inline)
```
---

## 🔊 Knobless Audio Execution Path
To maximize space and reduce manual physical tuning inside the housing, a knobless PAM8403 module is deployed. Volume adjustments are configured 100% via firmware down to the DFPlayer DAC line (df.volume(15) to 25).

DFPlayer DAC_R / DAC_L -> Routed directly to the PAM8403 Left / Right Input terminals.

DFPlayer GND ➡️ Tied directly to PAM8403 Audio Input Ground.

PAM8403 Output (L+/L- or R+/R-) ➡️ Wired directly to the 3W 4Ω Speaker.

---

## 🔊 Audio Strategy

- Uses **pre-recorded audio clips**
  
SD CARD ROOT/
├── 01/             # Core Numeric Elements [F_NUM]
│   ├── 001.wav     # "One"
│   ├── 020.wav     # "Twenty"
│   └── 030.wav     # "Hundred / Thousand" markers
├── 02/             # Device Status Alerts [F_SYS]
│   ├── 001.wav     # System Booting Sound (S_BOOTING)
│   ├── 002.wav     # Network Registered Sound (S_NETWORK)
│   ├── 003.wav     # Incoming Message Alert Ding (S_DING)
│   └── 004.wav     # Device Online & MQTT Ready (S_READY)
├── 03/             # Sentence Joining Fragments [F_WORDS]
│   ├── 001.wav     # "Paise" (W_PAISE)
│   ├── 002.wav     # "Rupees" (W_RUPEES)
│   ├── 003.wav     # "And" (W_AND)
│   ├── 004.wav     # "Amount" (W_AMOUNT)
│   └── 005.wav     # "Received" (W_RECEIVED)
└── 04/             # Exception Warnings [F_ERR]
    ├── 001.wav     # No Cellular Service Alert (E_NO_SRV)
    ├── 002.wav     # Failed Payment Status (E_NO_PAY)
    └── 003.wav     # General System Hardware Error (E_ERROR)
    
Supports:
- Dynamic composition (number + words)
- OR pre-generated full phrases (recommended)

👉 Recommended format:
- WAV (16-bit, 16kHz, mono)

---

## 🧪 Prototyping & Testing

Before full enclosure integration, individual modules were rigorously unit-tested across power draw cycles.

### 🔬 Evolution Stages
<div align="center">
  <img src="path/to/your/circuit_only_image.jpg" width="45%" alt="Breadboard Circuit Layout">
  <img src="path/to/your/working_internals_image.jpg" width="45%" alt="Assembled Hardware Matrix">
  <p><i>Left: Prototype breadboard validation circuit. Right: Main components integrated with power rails inside the final housing chassis.</i></p>
</div>

### ✅ Unit Tests Conducted
- ESP32 ↔ DFPlayer serial command continuity  
- SIM800L registration stabilization under $2\text{A}$ transient load bursts  
- Cellular GPRS session persistence & MQTT heartbeat logic (`state == -1` mitigation)  

---

### ✅ Unit Tests Conducted
- ESP32 ↔ DFPlayer communication  
- ESP32 ↔ SIM800 communication  
- Audio playback stability  
- Power supply reliability  
- MQTT message handling  

### 🔬 Prototype Iterations
- Breadboard setup  
- Perfboard integration  
- Full system assembly  

📸 Images of each stage are included in the repository.

---

## 🧱 Enclosure Design & CAD

- Custom designed multi-piece shell engineered within Fusion 360 to optimize acoustic resonance and component packaging constraints.

<div align="center">
  <img src="path/to/your/fusion360_screenshot.jpg" width="45%" alt="Fusion 360 Model">
  <img src="path/to/your/finished_pieces_image.jpg" width="45%" alt="3D Printed Components">
  <br>
  <img src="path/to/your/pieces_assembled_image.jpg" width="60%" alt="Mechanical Assembly Fit Test">
  <p><i>Top-Left: 3D modeling and layout parameters. Top-Right: Raw printed shells. Bottom: Structural test alignment before final adhesive lock.</i></p>
</div>

Designed to:
- Securely mount the 18650 cell cluster and MT3608 buck-boost converter safely.
- Feature internal guide tracks for optimal cellular antenna separation from audio traces.
- Support interchangeable front branding faceplates.

---

## ⚡ Power Design

- Battery-powered system  

Using DC-DC boosters for:
- Stable 5V (ESP32, DFPlayer, PAM)
- Stable ~4V (SIM800)

👉 Decoupling capacitors added for stability

---

## 🌐 MQTT Integration

- **Broker Target Host:** `io.adafruit.com` (Port: `1883`)
- **Subscription Feed:** `CasualClicks/feeds/amount`
- **Handshake Tuning:** Buffer depth is explicitly extended to `512` bytes with a robust `120s` keep-alive threshold to smoothly navigate high-latency cellular network environments.

### Expected Metric Payload Object:
```json
{
  "amount": "250.75"
}
```
---

## 📈 Scalability

- Supports multiple devices via MQTT  
- Can be deployed across shops  
- SIM-based version removes dependency on phone  

---

## 🔮 Future Improvements

- App-based dashboard  
- Multi-language support  
- Voice customization  
- Better audio stitching  
- OTA updates  
- Battery optimization  

---

## 🌍 Project Showcase

This project will also be showcased on:  
👉 https://www.officialshivam.in/

---

## 🤝 Contributions

Feel free to:

- Open issues  
- Suggest improvements  
- Fork and build your own version  

---

## 📜 License

MIT License

---

## 🙌 Acknowledgements

Inspired by real-world UPI soundboxes used in retail environments.

---

## 📬 Contact

**Shivam Singhal**

For queries, collaborations, or improvements — feel free to reach out!

---

⭐ If you found this useful, consider starring the repo!
