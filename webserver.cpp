#include "webserver.h"
#include "config.h"
#include "state.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <SD.h>

static AsyncWebServer server(80);
static AsyncWebSocket ws("/ws");

static const char INDEX_HTML[] PROGMEM = R"HTML(
<!DOCTYPE html><html><head><meta charset=utf-8>
<meta name=viewport content="width=device-width,initial-scale=1">
<title>CyberDash</title>
<style>
  :root{--bg:#0a0a14;--cyan:#0ff;--mag:#f0f;--grn:#0f0;--dim:#445}
  *{box-sizing:border-box}
  body{background:var(--bg);color:var(--cyan);font-family:'Courier New',monospace;
       margin:0;padding:20px;min-height:100vh}
  h1{color:var(--mag);letter-spacing:6px;text-align:center;margin:0 0 30px;text-shadow:0 0 12px var(--mag)}
  .grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(180px,1fr));gap:16px;max-width:900px;margin:auto}
  .card{border:1px solid var(--cyan);padding:18px;background:rgba(0,255,255,.04);box-shadow:0 0 12px rgba(0,255,255,.2)}
  .l{font-size:12px;color:var(--dim);letter-spacing:2px}
  .v{font-size:42px;color:var(--grn);text-shadow:0 0 8px var(--grn);margin-top:6px}
  .u{font-size:14px;color:var(--dim);margin-left:4px}
  .bar{height:10px;background:#001;border:1px solid var(--cyan);margin-top:14px;position:relative;overflow:hidden}
  .bar>i{display:block;height:100%;background:linear-gradient(90deg,var(--grn),var(--cyan),var(--mag));width:0;transition:width .3s}
  footer{text-align:center;margin-top:30px;color:var(--dim);font-size:12px}
  a{color:var(--cyan)}
</style></head><body>
<h1>// CYBERDASH //</h1>
<div class=grid>
  <div class=card><div class=l>TEMPERATURE</div><div class=v><span id=t>--</span><span class=u>&deg;C</span></div></div>
  <div class=card><div class=l>HUMIDITY</div><div class=v><span id=h>--</span><span class=u>%</span></div></div>
  <div class=card><div class=l>PRESSURE</div><div class=v><span id=p>--</span><span class=u>hPa</span></div></div>
  <div class=card><div class=l>SOUND LEVEL</div><div class=v><span id=d>--</span><span class=u>dB</span></div>
       <div class=bar><i id=db></i></div></div>
</div>
<footer>WS:<span id=ws>...</span> &middot; <a href="/log.csv">log.csv</a> &middot; <a href="/update">OTA</a></footer>
<script>
const $=id=>document.getElementById(id);
function apply(d){
  $('t').textContent=d.t.toFixed(1);
  $('h').textContent=d.h.toFixed(0);
  $('p').textContent=d.p.toFixed(0);
  $('d').textContent=d.db<0?'--':d.db;
  $('db').style.width=(d.db<0?0:Math.min(100,(d.db-30)*1.4))+'%';
}
const ws=new WebSocket(`ws://${location.host}/ws`);
ws.onopen =()=>$('ws').textContent='live';
ws.onclose=()=>$('ws').textContent='offline';
ws.onmessage=e=>apply(JSON.parse(e.data));
setInterval(()=>fetch('/api/data').then(r=>r.json()).then(apply),5000);
</script></body></html>)HTML";

static String jsonState() {
  SensorState s = stateSnapshot();
  String j = "{";
  j += "\"t\":"  + String(s.temperatureC, 2) + ",";
  j += "\"h\":"  + String(s.humidity,    2) + ",";
  j += "\"p\":"  + String(s.pressureHpa, 2) + ",";
  j += "\"db\":" + String(s.soundDb);
  j += "}";
  return j;
}

static void onWsEvent(AsyncWebSocket*, AsyncWebSocketClient* c, AwsEventType type,
                      void*, uint8_t*, size_t) {
  if (type == WS_EVT_CONNECT) c->text(jsonState());
}

bool webInit() {
  if (WiFi.status() != WL_CONNECTED) return false;

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* r){
    r->send_P(200, "text/html", INDEX_HTML);
  });
  server.on("/api/data", HTTP_GET, [](AsyncWebServerRequest* r){
    r->send(200, "application/json", jsonState());
  });
  server.on("/log.csv", HTTP_GET, [](AsyncWebServerRequest* r){
    if (SD.exists("/log.csv")) r->send(SD, "/log.csv", "text/csv", true);
    else r->send(404, "text/plain", "no log");
  });
  server.onNotFound([](AsyncWebServerRequest* r){ r->send(404, "text/plain", "404"); });
  server.begin();
  return true;
}

void webBroadcastState() {
  if (ws.count() == 0) return;
  ws.textAll(jsonState());
}
