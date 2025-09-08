# NOTICE — Waybeam firmware fork of Meshtastic

This repository is a **fork** of the Meshtastic firmware at:
<https://github.com/meshtastic/firmware> ("Upstream Project").

The Upstream Project is licensed under **GPL-3.0**. This fork preserves the GPL-3.0
license and all applicable copyright notices. See `LICENSE`.

We are **not affiliated with or endorsed by** the Meshtastic project or its maintainers.
"Meshtastic" and associated marks are trademarks of their respective owners. When
describing this fork, use phrasing like **"Compatible with Meshtastic."** Do not imply
endorsement. Do not reuse Meshtastic logos in a way that suggests affiliation.

---

## What we changed (high level)

> Last updated: 2025-01-08

- **Emergency LED feedback system:** Enhanced the existing white LED (next to RST button)
  to provide visual feedback for radio transmissions and received messages. Adds priority-based
  LED duration (8-30 seconds) to ensure emergency responders notice incoming communications.

- **Transmission LED indication:** LED turns solid ON during outgoing radio transmissions,
  providing immediate visual confirmation of message sending activity.

- **Receive LED alerts with priority detection:** LED stays solid ON for 8-30 seconds when
  receiving messages, with duration based on payload size (longer messages = longer LED duration).
  Critical emergency messages can trigger 30-second maximum visibility alerts.

- **Emergency responder focus:** All LED timing optimized for emergency response scenarios
  where responders need clear visual indication of radio activity and sufficient time to
  notice incoming communications.

**Technical implementation:**
- Modified LED GPIO combiner logic to support transmission/receive signals
- Added priority-based LED duration control system
- Enhanced LED blinker with timeout safety mechanisms
- Preserved all existing heartbeat and charging indicator functionality

**Hardware compatibility:** Uses existing hardware only - no additional LEDs or components required.
All standard Meshtastic radio platforms supported (ESP32, nRF52, STM32, RP2040).

Upstream radio stack and interoperability remain intact. All changes are additive and
non-breaking. Nodes operate identically to standard Meshtastic nodes with enhanced LED feedback.

---

## Corresponding Source for any binaries

If we publish release binaries (`.bin`/`.uf2`), the **corresponding source** for each
binary is this repository at the **matching tag/commit**. We provide exact build steps
and configs below so anyone can reproduce the bit-for-bit output.

- **Board(s):** All standard Meshtastic supported boards (Heltec_LoRa32_V3, TBeam_SX1262, RAK4631, etc.)
- **Toolchain:** Same as Upstream Meshtastic (ESP-IDF/Arduino-ESP32/PlatformIO)
- **Build command:**
  ```bash
  # Standard PlatformIO build process
  cd meshtastic-firmware
  pio run -e <board_variant>  # e.g., heltec_v3, tbeam, rak4631-nrf52840
  pio run -e <board_variant> -t upload  # to flash
  ```

- **LED Features:** Always enabled in this fork (no feature flags required)

---

## File modifications summary

**Files modified for LED enhancement:**
- `src/Led.h` - Added `ledTransmit`, `ledReceive` virtual pin declarations
- `src/Led.cpp` - Implemented 3-way GPIO combiner (heartbeat + transmit + receive)
- `src/main.h` - Added LED duration control function declarations
- `src/main.cpp` - Enhanced LED blinker with priority timing and safety timeouts
- `src/mesh/RadioLibInterface.cpp` - Added transmission/receive LED event triggers

**Total lines changed:** ~50 lines added across 5 files
**Breaking changes:** None - all modifications are additive

---

## Third-party notices

This project inherits third-party components and licenses from the Upstream Project.
See Upstream's `LICENSE`/`NOTICE`/`COPYING` files and any submodule LICENSE files.
We do not remove or restrict any rights granted by those licenses.

---

## Trademarks & naming

Use only descriptive phrases like "Waybeam firmware fork (compatible with Meshtastic)."
Do not present this fork as the official Meshtastic project. Do not use Meshtastic logos in a
manner that suggests affiliation or endorsement.

---

## Regulatory note (important)

Users are responsible for complying with local radio regulations (e.g., **EU_868**
duty-cycle limits and power constraints). Always attach a suitable antenna before
powering/transmitting. This firmware is designed for emergency response applications
but does not replace professional emergency communication systems.

**Emergency use disclaimer:** This firmware enhances visual feedback for emergency
communications but should not be the sole communication method in life-threatening
situations. Always maintain backup communication methods and follow established
emergency protocols.

---

## Contact

**Project:** Waybeam - Off-grid Emergency Coordination System
**Repository:** <https://github.com/ArrushC/Waybeam>
**Issues:** Use this repository's **Issues** tab
**Original Meshtastic Project:** <https://meshtastic.org>
