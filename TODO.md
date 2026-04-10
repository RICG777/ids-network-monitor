# IDS Network Monitor — Improvement Tracker

## Hardware Versions
- **XBoard Relay (DFR0222)** — ATmega32U4 + W5100, current dev board
- **Olimex ESP32-EVB-IND** — ESP32 + LAN8710A, ordered, awaiting delivery

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

## In Progress

_None_

## Remaining — Current Hardware (XBoard)

### Firmware Changes
| ID | Item | Description | Priority |
|----|------|-------------|----------|
| C1 | Configurable relay mode | Pulse vs Latch per relay, stored in EEPROM | High |
| C6 | Configurable pulse duration | Per-relay: 5s/10s/30s/60s, stored in EEPROM | High |
| C7 | Manual relay reset command | Serial command to de-energize a latched relay | High |
| C9 | Multiple triggers per relay | 2-3 triggers per relay within EEPROM budget | Medium |
| C10 | ~~Configurable MAC address~~ | PARKED — wait for ESP32 boards to confirm unique MACs. Config should be manufacture-only, not user-facing | — |
| C10a | Read-only MAC address display | Report MAC in STATUS response for commissioning/O&M documentation | High |
| C11 | Subnet mask / gateway config | Full network config, not just IP | Medium |
| C12 | Debug log output control | Option to reduce serial debug verbosity | Low |

### Windows App — Must Have (hardware matching)
| ID | Item | Description | Depends On | Priority |
|----|------|-------------|------------|----------|
| W1 | Relay mode selector | Dropdown: Pulse / Latch per relay | C1 | High |
| W2 | Pulse duration setting | Numeric per relay: 5s, 10s, 30s, 60s | C6 | High |
| W3 | Manual relay reset buttons | "Reset" button per relay | C7 | High |
| W4 | Live relay status polling | Periodic STATUS poll to keep indicators current | — | Medium |
| W5 | Multiple triggers per relay | Add/remove list UI per relay | C9 | Medium |
| W6 | ~~MAC address configuration~~ | PARKED — manufacture-only function, not user-facing | — | — |
| W6a | Read-only MAC address display | Show MAC on Connection tab for commissioning/O&M | C10a | High |
| W7 | Subnet mask / gateway fields | UI is in place, firmware needs support | C11 | Medium |
| W8 | Log to file | Auto-save event log to timestamped text file | — | High |

### Windows App — Next Up (no firmware dependency)
| ID | Item | Description | Priority |
|----|------|-------------|----------|
| W26 | Relay test buttons | "Test Relay 1/2" — momentary pulse to verify wiring | High |
| W16 | About dialog | Version, copyright, support contact | Medium |
| W23 | Firmware version check | Read version from STATUS, warn on mismatch | Medium |

### Windows App — Polish
| ID | Item | Description | Priority |
|----|------|-------------|----------|
| W15 | Auto-reconnect | Detect disconnect, reconnect with backoff | Medium |
| W17 | Error handling polish | Clean messages for all failure modes | Medium |
| W18 | Resizable window | Anchored controls (partially done) | Low |
| W19 | Export/import config | Save/load board config as JSON | Medium |
| W21 | Connection heartbeat LED | Animated indicator showing comms are live | Low |
| W22 | Dark/light theme | Match Windows system theme | Low |
| W24 | Installer (MSI/MSIX) | Windows installer with Start Menu shortcut | High (pre-launch) |
| W25 | Log filtering & search | Filter by type, search box | Low |
| W27 | Multi-unit support | Manage multiple boards from one app | Low |
| W28 | System tray notifications | Minimise to tray, popup on relay fire | Medium |

## Future — ESP32 Board Only
| ID | Item | Description |
|----|------|-------------|
| E1 | ICMP ping heartbeat | Ping monitored devices, trigger relay on comms loss |
| E2 | SNMP trap parsing | Full ASN.1 decoding (needs 512KB SRAM) |
| E3 | Web-based config UI | Browser interface hosted on device, replace Windows app |
| E4 | 4 relays | Separate security / environmental / comms-fail / spare |
| E5 | Disable WiFi/BLE | Security hardening for deployments |
| E6 | LiPo mains fail detection | Alert on power loss using built-in UPS |
| E7 | OTA firmware updates | Update over Ethernet without USB |
| E8 | Event logging to microSD | Persistent log of triggers and state changes |

## Build & Deploy Notes
- **Arduino CLI**: `arduino-cli compile --fqbn arduino:avr:leonardoeth <sketch_dir>`
- **Arduino upload**: `arduino-cli upload --fqbn arduino:avr:leonardoeth --port COM6 <sketch_dir>`
- **MSBuild**: `"C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\MSBuild\Current\Bin\MSBuild.exe" <sln> -p:Configuration=Release`
- **Exe location**: `windows app\source\repos\IDS NW Monitor v1a\bin\Release\IDS NW Monitor v1a.exe`
- **GitHub**: https://github.com/RICG777/ids-network-monitor
