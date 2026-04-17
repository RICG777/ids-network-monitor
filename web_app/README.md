# IDS Network Monitor — Web UI

Browser-based configuration UI for the IDS Network Monitor ESP32 firmware. Replaces the WinForms desktop app (`windows app/`, v0.9.0) with a single codebase that works over both USB (Web Serial) and, later, HTTP (when firmware v2f.2 ships the in-board API).

## Quick start

```bash
cd web_app
python dev-server.py 8000
# then open http://localhost:8000
```

Chrome or Edge 89+ required for USB (Web Serial). Firefox and Safari can open the app but only in Mock mode.

## Serving options

| Server | Caveat |
|---|---|
| `python dev-server.py` (bundled) | Adds `Cache-Control: no-store` so module edits land on reload without a browser restart. Recommended for dev. |
| `python -m http.server` | Works but Chrome caches ES modules aggressively — edits to a module may require closing the tab. |
| Any static host | Must serve over HTTPS or `localhost` for Web Serial to work. |

## Transports

| Kind | When to use | Current status |
|---|---|---|
| `webserial` | Real ESP32 on USB. Default on Chrome/Edge. | Working (v2f.1 target). |
| `mock` | UI dev, demos, CI-style testing. Simulates a v2f.1 firmware in memory. | Working. |
| `http` | Remote config via the ESP32's HTTP server. | Stub — ships with firmware v2f.2. |

Switch via the About tab, or use URL param:

- `?transport=webserial`
- `?transport=mock`
- `?transport=http` (will throw — not implemented yet)

Selection is remembered in `localStorage` (`idsnm.transport`).

## URL parameters

For mock-mode testing, the following fault modes can be activated (comma-separate to combine):

- `?mock=slow` — add 200 ms latency per response
- `?mock=offline` — `connect()` rejects
- `?mock=reboot` — emit simulated boot storm 10 s after connect (exercises the reboot-sentinel + auto-resync path)
- `?mock=timeout` — TEST1 never emits completion msg (exercises the timeout path)

Mix-and-match: `?transport=mock&mock=slow,reboot`.

## File layout

```
web_app/
├── dev-server.py           Local no-cache static server
├── index.html
├── app.css                 Single stylesheet, dark theme
├── assets/
│   └── tier8-logo.png
├── README.md
└── js/
    ├── app.js              Entry: wiring, tab router, status bar, poll
    ├── state.js            Single-source-of-truth pub/sub store
    ├── protocol.js         parseLine + per-command metadata
    ├── validators.js       Client-side input validation
    ├── log.js              2000-entry event ring buffer
    ├── transport/
    │   ├── index.js        Factory + URL/localStorage resolver
    │   ├── base.js         Abstract EventTarget + FIFO queue + correlation
    │   ├── webserial.js    Web Serial API adapter
    │   ├── mock.js         In-memory firmware simulator
    │   └── http.js         Stub — v2f.2
    └── ui/
        ├── banner.js       Blocking banner for TEST/TTEST
        ├── confirm.js      Promise-based <dialog> confirm
        ├── toast.js        Transient notifications
        ├── tab-connection.js
        ├── tab-relays.js
        ├── tab-ping.js
        ├── tab-events.js
        └── tab-about.js
```

## Protocol notes (firmware target v2f.1)

The app speaks the same line-based serial protocol the firmware exposes on USB CDC at 115200 8N1:

- Commands sent with `\r\n` terminators (firmware accepts `\n` too, but CRLF matches what real serial terminals send).
- Responses prefixed with:
  - `ArdACK:<key>:<value>` — success echo
  - `ArdERR:<key>:<message>` — validation/processing error
  - `ArdMsg: <text>` — informational log, including unsolicited events (relay energised, ping timeout, tamper, boot)
  - `ArdSTATUS:<key>:<value>` — one line per key in a STATUS dump (no end-marker; parser resolves on ~150 ms idle)
- `STATUS` and `PING:LIST` have no ACK — they dump status lines only.
- `TEST1/2` and `TTEST1/2` block the board for the test duration. The transport emits a `blocking` event during these; the UI locks all action buttons and shows a progress banner until the `Relay N [timed] test complete.` message lands.
- Keys can contain colons (`PING:ADD`, `PING:INT`, `PING:T0`). Parser uses longest-known-prefix match.

## Testing without hardware

Open `http://localhost:8000/?transport=mock` and drive every feature from the UI. The mock simulates the full firmware dispatch path including error responses, persists its "NVS" for the session, and fires the right `msg:` completion events for TEST/TTEST.

For automated smoke tests, instantiate the same app and script interactions via Playwright (see earlier commits for patterns).

## Browser support

- Chrome 89+, Edge 89+ — full support including USB (Web Serial).
- Firefox, Safari — app loads, Mock mode works, USB does not (no `navigator.serial`).
- `<dialog>` element required (Chrome/Edge 89+ — aligns with Web Serial minimum).

## Known limitations (v0.1)

- No HTTPS transport (use the ESP32's future HTTP API instead of remote HTTPS).
- No multi-unit support — one board per browser tab.
- No OTA, no microSD event log, no SNMP parsing (those are firmware-side, see `TODO.md`).
- Ship as HTML/JS only — no installer yet. Host via dev-server.py or on a small LAN intranet.

## Development tips

- **Cache pain**: If module edits don't seem to land, close the tab and reopen. The bundled `dev-server.py` minimises this.
- **"port.readable is locked"** errors on reconnect: usually from an incomplete disconnect. `js/transport/webserial.js` does writer.close → reader.cancel → await pipeline → port.close in that order. If you edit it, preserve that order.
- **Mock firmware doesn't persist across reloads** — that's intentional. Every reload starts from the defaults in `mock.js`.
