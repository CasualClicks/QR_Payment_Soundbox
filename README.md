<!-- Generated using AI to save time -->

# 🔊 QR Payment Soundbox

A DIY **QR Payment Soundbox** built using ESP32 that announces payment confirmations in real-time — similar to commercial UPI soundboxes, but designed for **personal (non-merchant) and scalable merchant use cases**.

---

## 🚀 Overview

This project enables real-time **audio announcements for UPI payments** by:

- Receiving payment confirmation messages (SMS or server)
- Processing them via MQTT
- Playing clear audio feedback through a speaker

### Supports:

- 🧑‍💻 **Non-merchant accounts** (via phone → MQTT)
- 🏪 **Merchant/scalable setup** (via SIM module directly on device)

---

## 🧩 Features

- 🔊 Real-time voice announcements (e.g., *"Amount received 100 rupees"*)
- 📶 Works over WiFi (MQTT-based communication)
- 📱 Supports SMS-based triggers
- 📡 SIM-based standalone mode (no phone dependency)
- 🧠 Modular architecture (easy to scale & extend)
- 🧪 Unit-tested hardware modules
- 🧱 Custom 3D printed enclosure

---

## 🛠️ Hardware Components

| Component | Purpose |
|----------|--------|
| ESP32 | Main controller (WiFi + logic) |
| DFPlayer Mini | Audio playback module |
| PAM8403 Amplifier | Amplifies audio output |
| 3W Speaker | Sound output |
| SIM800L | GSM module for SMS-based payments |
| DC-DC Boost Converters | Voltage regulation (3.7V → 4V/5V) |
| Battery (Li-ion) | Portable power |
| 3D Printed Enclosure | Housing and mounting |

---

## 🧠 System Architecture

### 🔹 Non-Merchant Mode (Phone-Based)
Payment SMS → Mobile Phone → MQTT Server → ESP32 → Audio Output
- SMS received on phone  
- Parsed and pushed to MQTT  
- ESP32 subscribes and plays audio
- 
---

### 🔹 Merchant Mode (Standalone SIM)
- SIM800 receives SMS directly  
- ESP32 processes message  
- Fully independent system  

---

## 🔌 Wiring Overview

### ESP32 ↔ DFPlayer Mini
- TX → RX (via 1k resistor)
- RX ← TX
- VCC → 5V
- GND → GND

### DFPlayer → PAM8403
- DAC_L → L
- DAC_R → R (or B)
- GND → GND

### PAM8403 → Speaker
- L+/L- → Speaker

### SIM800 ↔ ESP32
- TX ↔ RX
- RX ↔ TX
- ⚠️ Proper 4V regulated supply required

---

## 🔊 Audio Strategy

- Uses **pre-recorded audio clips**

Supports:
- Dynamic composition (number + words)
- OR pre-generated full phrases (recommended)

👉 Recommended format:
- WAV (16-bit, 16kHz, mono)

---

## 🧪 Prototyping & Testing

Before full integration, individual modules were tested:

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

## 🧱 Enclosure Design

- Custom designed using Fusion 360  
- 3D printed shell  

Designed to:
- Fit all components snugly  
- Allow proper sound output  
- Provide antenna clearance  
- Support mounting & durability  

---

## ⚡ Power Design

- Battery-powered system  

Using DC-DC boosters for:
- Stable 5V (ESP32, DFPlayer, PAM)
- Stable ~4V (SIM800)

👉 Decoupling capacitors added for stability

---

## 🌐 MQTT Integration

**Topic:**
### **Payload example:**
```json
{
  "amount": 100
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

MIT License (or your preferred license)

---

## 🙌 Acknowledgements

Inspired by real-world UPI soundboxes used in retail environments.

---

## 📬 Contact

**Shivam Singhal**

For queries, collaborations, or improvements — feel free to reach out!

---

⭐ If you found this useful, consider starring the repo!
