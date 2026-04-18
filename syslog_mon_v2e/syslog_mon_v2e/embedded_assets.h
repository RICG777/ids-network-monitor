// Static web assets embedded in firmware. For v2f.2 this is just a minimal
// stub served at / — enough to confirm HTTP is alive. The full web app
// lives at web_app/ in the repo and is served over USB Web Serial or
// (when wired up) HTTP via ?transport=http&host=<board-ip>.
//
// Migration to LittleFS-served assets is Phase 5 / v2f.5, optional.

#ifndef IDS_EMBEDDED_ASSETS_H
#define IDS_EMBEDDED_ASSETS_H

#include <pgmspace.h>

const char INDEX_HTML[] PROGMEM = R"HTML(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<title>IDS Network Monitor</title>
<style>
  body { font-family: -apple-system, Segoe UI, sans-serif; background:#1e1e1e; color:#e0e0e0;
         max-width: 42rem; margin: 2rem auto; padding: 0 1rem; }
  h1 { font-size: 1.1rem; color:#4a9eff; }
  code { background:#2a2a2a; padding:0.1rem 0.3rem; border-radius:3px; color:#e0e0e0; }
  pre  { background:#121212; padding:0.6rem; border-radius:4px; overflow-x:auto; }
  a    { color:#4a9eff; }
  .pill { display:inline-block; padding:0.1rem 0.5rem; border-radius:10px;
          border:1px solid #4caf50; color:#4caf50; font-size:0.8rem; }
  .hint { color:#999; font-size: 0.9rem; }
</style>
</head>
<body>
<h1>IDS Network Monitor <span class="pill">HTTP online</span></h1>
<p>This board is serving its config API over HTTP. Raw endpoints:</p>
<ul>
  <li><a href="/api/status"><code>GET /api/status</code></a> — full config + runtime state as JSON.</li>
</ul>
<p class="hint">The rich browser UI lives in the <code>web_app/</code> folder of the project repo. Open it
from any machine on this subnet and point it at this board with
<code>?transport=http&amp;host=&lt;this-board-ip&gt;</code>.</p>
<p class="hint">USB serial config remains available regardless — plug a laptop into the board's
USB port and the same commands work over COM at 115200 baud.</p>
<p class="hint">Web UI requires the UEXT jumper fitted (GPIO&nbsp;16 ↔ GPIO&nbsp;13). Without the
jumper, <code>/api/*</code> returns HTTP 403.</p>
<pre id="status">Loading /api/status...</pre>
<script>
fetch('/api/status').then(r => r.json()).then(j => {
  document.getElementById('status').textContent = JSON.stringify(j, null, 2);
}).catch(e => { document.getElementById('status').textContent = 'Error: ' + e; });
</script>
</body>
</html>)HTML";

#endif
