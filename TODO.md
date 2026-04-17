# IDS Network Monitor — Improvement Tracker

## Hardware Versions
- **XBoard Relay (DFR0222)** — ATmega32U4 + W5100, current dev board
- **Olimex ESP32-EVB-IND** — ESP32-D0WD-V3 + LAN8710A, connected and verified
  - COM8 (CH340 USB-Serial), 240MHz dual core, 4MB flash, 291KB free heap
  - Ethernet MAC: EC:C9:FF:BC:A7:C4
  - Toolchain: arduino-cli + esp32:esp32 v3.3.8, FQBN: `esp32:esp32:esp32-evb`

## Completed

### Firmware
| ID | Item | Version |
|----|------|---------|
| A1 | Trigger buffer overflow fix (char[80]) | v1d |
| B1 | packetBuffer reduced 512→256 | v1d |
| B2 | String() objects replaced with F() macro | v1d |
| B3 | Case-insensitive trigger matching | v1d |
| B4 | Trigger length validation before EEPROM write | v1d |
| C2 | ACK/ERR protocol for config commands | v1d |
| C3 | STATUS command (IP, triggers, relay states) | v1d |
| C1 | Configurable relay mode (Pulse/Latch per relay) | v1d.2 |
| C6 | Configurable pulse duration per relay | v1d.2 |
| C7 | Manual relay reset command | v1d.2 |
| C10a | Read-only MAC address display | v1d.2 |
| C9 | Multiple triggers per relay (semicolon-separated) | v1d.4 |
| C11 | Subnet mask / gateway config | v1d.3 |
| C12 | Debug log output control (verbosity 0-2) | v1d.3 |

### Windows App
| ID | Item | Version |
|----|------|---------|
| A2 | Serial receive handler fix (was destroying data) | v0.3.0 |
| A3 | COM port null crash fix | v0.3.0 |
| B5 | Settings persistence (COM port, IP, triggers) | v0.3.0 |
| C4 | Form title ("IDS Network Monitor") | v0.3.0 |
| C5 | Timestamps in debug console | v0.3.0 |
| W9 | App name, Tier8 icon, taskbar branding | v0.4.0 |
| W10 | Tabbed layout (Connection, Relay Config, Event Log) | v0.4.0 |
| W11 | Status bar (connection, COM port, board IP, version) | v0.4.0 |
| W12 | Confirmation dialogs before writing to board | v0.4.0 |
| W13 | Inline validation (IP, trigger length) | v0.4.0 |
| W14 | Controls greyed out when disconnected | v0.4.0 |
| W20 | COM port refresh button | v0.4.0 |
| W1 | Relay mode selector (Pulse/Latch dropdown) | v0.6.0 |
| W2 | Pulse duration setting per relay | v0.6.0 |
| W3 | Manual relay reset buttons | v0.6.0 |
| W6a | Read-only MAC address display | v0.6.0 |
| W26 | Relay test buttons (Quick 3s + Timed) | v0.6.0 |
| W5 | Multiple triggers per relay (semicolon UI) | v0.8.0 |
| W7 | Subnet mask / gateway fields | v0.7.0 |
| W8 | Log to file | v0.5.0 |
| W16 | About dialog | v0.5.0 |
| W23 | Firmware version check | v0.5.0 |

## In Progress

### Web UI (E3, local mode) — v0.1.0-dev
| ID | Item | Status |
|----|------|--------|
| WEB0 | Skeleton: HTML/CSS/JS scaffold, tab router, state store, mock transport stub | Done |
| WEB1 | Protocol parser, mock firmware simulator, live Event Log | Done |
| WEB2 | Web Serial adapter (USB CDC, 115200 8N1, flowControl none) | Done — needs first-hardware-test |
| WEB3 | Connection tab: connect/disconnect, IP/subnet/gateway write, MAC + FW display, mismatch banner | Done |
| WEB4 | Relay Config tab: triggers/mode/duration write, Reset, Quick Test, Timed Test + blocking UX | Done |
| WEB5 | Ping Config tab: global settings + target list (up to 20) with per-target OK/FAIL | Done |
| WEB6 | About tab with Tier8 branding, transport switcher, URL-param reference | Done |
| WEB7 | HTTP transport stub + `web_app/README.md` | Done |

Still to do for the web app before it can retire the WinForms app:
- Real-hardware validation of Web Serial adapter (first click from Ric).
- HTTP transport implementation, arrives alongside firmware v2f.2.
- Migrate assets into firmware PROGMEM / LittleFS (firmware-side work).

## Completed — ESP32 Port (Phase 1)

### Firmware v2e.1
| ID | Item | Status |
|----|------|--------|
| P1 | Library port (ETH.h, NetworkUdp, Preferences) | Done |
| P2 | Pin remapping (GPIO32, GPIO33) | Done |
| P3 | Event-driven Ethernet (RMII/LAN8720) | Done |
| P4 | EEPROM → NVS Preferences storage | Done |
| P5 | Serial protocol (identical to v1d.4) | Done |
| P6 | WiFi/BLE disable on boot (E5) | Done |
| P7 | Hardware MAC address (dynamic) | Done |
| P8 | Relay tests (TEST/TTEST/RESET) | Done |
| P9 | Config persistence across reboot | Done |
| P10 | Error validation (duration/verbosity/triggers) | Done |

### Windows App v0.9.0
| ID | Item | Status |
|----|------|--------|
| P11 | Baud rate 9600 → 115200 | Done |
| P12 | Expected FW version → 2e.1 | Done |
| P13 | App version → 0.9.0 | Done |

## Remaining — Current Hardware (XBoard)

### Firmware Changes

All firmware features complete for XBoard (v1d.4). No remaining items.

### Windows App — Polish
| ID | Item | Description | Priority |
|----|------|-------------|----------|
| W15 | Auto-reconnect | Detect disconnect, reconnect with backoff | Medium |
| W17 | Error handling polish | Clean messages for all failure modes | Medium |
| W18 | Resizable window | Anchored controls (partially done) | Low |
| W19 | Export/import config | Save/load board config as JSON | Medium |
| W21 | Connection heartbeat LED | Animated indicator showing comms are live | Medium |
| W24 | Installer (MSI/MSIX) | Windows installer with Start Menu shortcut | High (pre-launch) |
| W25 | Log filtering & search | Filter by type, search box | Low |
| W27 | Multi-unit support | Manage multiple boards from one app | Low |

## Future — ESP32 Board Only
| ID | Item | Description |
|----|------|-------------|
| E1 | ICMP ping heartbeat | Ping up to 20 devices, trigger relay on comms loss. Configurable threshold (default 3), target relay (default R1), mode (auto-reset/latch/pulse) |
| E3 | Web-based config UI | Browser interface. Local mode (Web Serial over USB) v0.1.0 in `web_app/` — see WEB0-7 above. Remote mode (hosted on ESP32 over HTTP) lands with firmware v2f.2: jumper-gated (UEXT pins 5+6, GPIO 16+13) — secure by default |
| E5 | ~~Disable WiFi/BLE~~ | Done in v2e.1 |
| E9 | Failsafe/watchdog relay | Dedicated relay energised when running, drops on power loss or board lockup. Hardware watchdog resets board if loop stalls. Alarm panel wires to NC contact |

### Deferred — Post First Release
| ID | Item | Description |
|----|------|-------------|
| E2 | SNMP trap parsing | Full ASN.1 decoding (complex) |
| E4 | 4 relays | Separate security / environmental / comms-fail / spare |
| E7 | OTA firmware updates | Update over Ethernet without USB |
| E8 | Event logging to microSD | Persistent log of triggers and state changes |

## Build & Deploy Notes

### XBoard (ATmega32U4)
- **Compile**: `arduino-cli compile --fqbn arduino:avr:leonardoeth <sketch_dir>`
- **Upload**: `arduino-cli upload --fqbn arduino:avr:leonardoeth --port COM6 <sketch_dir>`

### ESP32-EVB (Olimex)
- **Compile**: `arduino-cli compile --fqbn esp32:esp32:esp32-evb <sketch_dir>`
- **Upload**: `arduino-cli upload --fqbn esp32:esp32:esp32-evb --port COM8 <sketch_dir>`
- **Board core**: esp32:esp32 v3.3.8
- **Serial baud**: 115200

### Windows App
- **MSBuild**: `"C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\MSBuild\Current\Bin\MSBuild.exe" <sln> -p:Configuration=Release`
- **Exe location**: `windows app\source\repos\IDS NW Monitor v1a\bin\Release\IDS NW Monitor v1a.exe`

- **GitHub**: https://github.com/RICG777/ids-network-monitor
