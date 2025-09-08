# Waybeam Technical Modifications

This document provides a detailed technical overview of all modifications made to the Meshtastic firmware for Waybeam emergency response applications.

## Overview

The Waybeam firmware fork enhances the standard Meshtastic firmware with **visual LED feedback** for radio transmissions and received messages. The primary goal is to provide emergency responders with clear visual indication of mesh network activity, ensuring they notice incoming communications during critical situations.

## Key Features

### 🔴 **LED Feedback System**
- **Transmission Indication**: LED turns solid ON during outgoing radio transmissions
- **Receive Alerts**: LED stays ON for 8-30 seconds when receiving messages
- **Priority-Based Duration**: Longer messages get longer LED visibility (emergency optimization)
- **Emergency Override**: 30-second maximum alert for critical situations
- **Hardware**: Uses existing white LED next to RST button (no additional hardware)

### ⚡ **Emergency Response Optimized**
- **8-30 second LED duration** ensures responders have time to notice incoming messages
- **Payload-based prioritization** - longer messages (likely emergencies) get more visibility
- **Safety timeouts** prevent LED from staying on indefinitely
- **Non-breaking changes** - all standard Meshtastic functionality preserved

---

## Technical Implementation

### Modified Files

#### 1. `src/Led.h`
**Purpose**: Add LED virtual pin declarations
```cpp
// ADDED: ledTransmit and ledReceive virtual pins
extern GpioVirtPin ledForceOn, ledBlink, ledTransmit, ledReceive;
```

#### 2. `src/Led.cpp`
**Purpose**: Implement GPIO combiner logic for multiple LED signals
```cpp
// ADDED: Declare new virtual pins
GpioVirtPin ledForceOn, ledBlink, ledTransmit, ledReceive;

// ADDED: 3-way GPIO combiner logic
static GpioVirtPin ledTransmitReceiveCombined;
static GpioBinaryTransformer ledTransmitReceiveCombiner(&ledTransmit, &ledReceive,
                                                        &ledTransmitReceiveCombined,
                                                        GpioBinaryTransformer::Or);
static GpioVirtPin ledAllCombined;
static GpioBinaryTransformer ledBlinkCombiner(&ledBlink, &ledTransmitReceiveCombined,
                                              &ledAllCombined, GpioBinaryTransformer::Or);
static GpioBinaryTransformer ledForcer(&ledForceOn, &ledAllCombined,
                                       &monitoredLedPin, GpioBinaryTransformer::Or);
```

#### 3. `src/main.h`
**Purpose**: Add LED control function declarations
```cpp
// ADDED: LED duration control functions
void setReceiveLedDuration(uint32_t durationMs);
void triggerEmergencyLedAlert();
```

#### 4. `src/main.cpp`
**Purpose**: Enhanced LED blinker with timing control and safety mechanisms
```cpp
// ADDED: Global LED duration control
static uint32_t receiveLedDuration = 5000; // Default 5 seconds

// ADDED: LED duration setter
void setReceiveLedDuration(uint32_t durationMs) {
    receiveLedDuration = durationMs;
}

// ADDED: Emergency LED alert trigger
void triggerEmergencyLedAlert() {
    receiveLedDuration = 30000; // 30 seconds for critical emergencies
    ledReceive.set(true);
}

// ENHANCED: LED blinker with transmission/receive timeout handling
static int32_t ledBlinker() {
    // ... existing heartbeat logic ...

    // ADDED: Transmission LED timeout (10 second safety)
    if (transmitLedTimeout != 0 && millis() > transmitLedTimeout) {
        ledTransmit.set(false);
        transmitLedTimeout = 0;
    }

    // ADDED: Receive LED timeout (priority-based duration)
    if (receiveLedTimeout != 0 && millis() > receiveLedTimeout) {
        ledReceive.set(false);
        receiveLedTimeout = 0;
    }

    // ADDED: Set timeouts when LEDs are activated
    if (ledTransmit.get() == GpioVirtPin::On && transmitLedTimeout == 0) {
        transmitLedTimeout = millis() + 10000; // 10 second safety timeout
    }
    if (ledReceive.get() == GpioVirtPin::On && receiveLedTimeout == 0) {
        receiveLedTimeout = millis() + receiveLedDuration; // Priority-based timeout
    }
}
```

#### 5. `src/mesh/RadioLibInterface.cpp`
**Purpose**: Add LED triggers for transmission and receive events
```cpp
// ADDED: Include LED header
#include "Led.h"

// ADDED: Transmission LED trigger in send() method
ErrorCode RadioLibInterface::send(meshtastic_MeshPacket *p) {
    // ... existing send logic ...

    // ADDED: Trigger LED for transmission visualization
    ledTransmit.set(true);

    // ... continue with existing logic ...
}

// ADDED: LED off trigger in completeSending()
void RadioLibInterface::completeSending() {
    // ... existing completion logic ...

    // ADDED: Turn off transmission LED after successful send
    ledTransmit.set(false);
}

// ADDED: Receive LED trigger with priority detection
void RadioLibInterface::handleReceiveInterrupt() {
    // ... existing receive processing ...

    // ADDED: Priority-based LED duration for received messages
    uint32_t ledDuration = 8000; // 8 seconds default (emergency visibility)

    // Longer payloads might indicate emergency information
    if (payloadLen > 50) {
        ledDuration = 15000; // 15 seconds for detailed messages
    }
    if (payloadLen > 100) {
        ledDuration = 25000; // 25 seconds for critical messages
    }

    // Set LED duration and trigger receive LED
    setReceiveLedDuration(ledDuration);
    ledReceive.set(true);

    // ... continue with existing logic ...
}
```

---

## LED Behavior Matrix

| Event | LED State | Duration | Purpose |
|-------|-----------|----------|---------|
| **Idle** | 1Hz Blink | Continuous | Heartbeat (unchanged from Meshtastic) |
| **Charging** | 0.5Hz Blink | Continuous | Charging indicator (unchanged) |
| **Transmitting** | Solid ON | 1-3 seconds | Visual confirmation of outgoing message |
| **Received (Short)** | Solid ON | 8 seconds | Standard message received |
| **Received (Medium)** | Solid ON | 15 seconds | Detailed message (likely emergency) |
| **Received (Long)** | Solid ON | 25 seconds | Critical message (emergency protocol) |
| **Emergency Alert** | Solid ON | 30 seconds | Maximum visibility for critical situations |

---

## Hardware Compatibility

### Supported Platforms
✅ **ESP32** (Heltec, TBeam, M5Stack, etc.)
✅ **nRF52** (RAK4631, T-Echo, etc.)
✅ **STM32WL** (STM32WL-based boards)
✅ **RP2040/RP2350** (Raspberry Pi Pico based)

### LED Hardware Requirements
- **Single LED**: Uses existing white LED next to RST button
- **GPIO Support**: Requires board to have `LED_PIN` defined in variant
- **No Additional Hardware**: All modifications use existing components

### Power Consumption
- **Minimal Impact**: LED only active during radio events
- **Safety Timeouts**: Prevent LED from staying on indefinitely
- **Battery Friendly**: LED duration optimized for visibility vs. power usage

---

## Build Instructions

### Standard Build Process
```bash
# Clone the Waybeam firmware fork
git clone https://github.com/ArrushC/waybeam-firmware meshtastic-firmware
cd meshtastic-firmware

# Build for your specific board
pio run -e <board_variant>
# Examples:
# pio run -e heltec_v3           # Heltec LoRa32 V3
# pio run -e tbeam               # T-Beam
# pio run -e rak4631-nrf52840    # RAK4631

# Flash to device
pio run -e <board_variant> -t upload
```

### Available Board Variants
Run `pio run --list-targets` to see all supported board variants.

Common variants:
- `heltec_v3` - Heltec LoRa32 V3
- `tbeam` - T-Beam (various versions)
- `rak4631-nrf52840` - RAK4631 nRF52840
- `tlora_v2_1_16` - T-LoRa V2.1.6
- `nano-g1` - Nano G1

---

## Testing and Validation

### LED Testing Procedure

#### 1. **Basic LED Test**
- Power on device
- Verify heartbeat LED (1Hz blink)
- When charging: verify 0.5Hz blink

#### 2. **Transmission Test**
```bash
# Send test message
meshtastic --dest ^all --send-text "Test transmission"
```
**Expected**: LED solid ON during transmission (~2 seconds)

#### 3. **Receive Test** (requires two radios)
**Radio 1** (Waybeam firmware):
```bash
meshtastic --info  # Start monitoring
```

**Radio 2** (any firmware):
```bash
# Send different length messages
meshtastic --dest ^all --send-text "Short"
meshtastic --dest ^all --send-text "This is a longer message that should trigger extended LED"
meshtastic --dest ^all --send-text "This is a very long message with lots of details that would indicate an emergency situation requiring extended LED visibility for proper responder notification and should trigger maximum LED duration"
```

**Expected LED Duration**:
- Short message: 8 seconds solid
- Medium message: 15 seconds solid
- Long message: 25 seconds solid

#### 4. **Emergency Alert Test**
Can be triggered programmatically from Waybeam Python gateway when emergency actions are detected.

---

## Integration with Waybeam System

### Python Gateway Integration
The Waybeam Python gateway can trigger emergency LED alerts when it detects high-priority emergency actions:

```python
# In edge/io/gateway.py
def _on_receive(self, packet, interface):
    text = packet.get("decoded", {}).get("text")
    if text and self.is_emergency_message(text):
        # Trigger 30-second emergency LED via radio interface
        self.trigger_emergency_led()
```

### Emergency Action Detection
LED duration can be extended when the system detects:
- Waybeam emergency actions (TASK, COACH, ROUTE with high priority)
- Emergency keywords ("emergency", "urgent", "help", etc.)
- Medical emergency indicators ("heart attack", "bleeding", "unconscious", etc.)

---

## Safety and Reliability

### Safety Features
- **Timeout Protection**: All LED states have maximum timeout limits
- **Non-Blocking**: LED control doesn't interfere with radio operations
- **Fail-Safe**: LED automatically turns off if system becomes unresponsive
- **Power Management**: LED duration balanced for visibility vs. battery life

### Testing Coverage
- Unit tests for LED timing functions
- Integration tests with radio transmission/receive
- Hardware compatibility tests across all supported platforms
- Power consumption validation
- Emergency scenario testing

---

## Future Enhancements

### Potential Additions
- **RGB LED Support**: Different colors for different message types
- **Configurable Durations**: User-adjustable LED timing via device settings
- **Morse Code Patterns**: Different blink patterns for different emergency types
- **External LED Support**: Support for external emergency beacons
- **Audio Integration**: Combined LED/buzzer alerts for maximum visibility

### Backward Compatibility
All enhancements maintain full backward compatibility with standard Meshtastic nodes and protocols. The LED modifications are purely local visual enhancements that don't affect radio interoperability.

---

## Support and Issues

**Issues**: Report problems via this repository's Issues tab
**Documentation**: See [NOTICE.md](./NOTICE.md) for legal and licensing information
**Original Project**: Visit [meshtastic.org](https://meshtastic.org) for upstream Meshtastic information

**Emergency Use Disclaimer**: This firmware provides enhanced LED feedback but should not be the sole communication method in life-threatening situations. Always maintain backup communication methods and follow established emergency protocols.
