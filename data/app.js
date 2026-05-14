// CyberDash live dashboard
'use strict';

const $ = id => document.getElementById(id);
const fmt = (v, d) => (v === null || v === undefined || Number.isNaN(v)) ? '--' : v.toFixed(d);

const HIST = 90;                       // samples kept per sparkline
const series = { t: [], d: [] };
const boot = performance.now();

// ---------- WebSocket ----------
let ws, retry = 0;
function connect() {
  const proto = location.protocol === 'https:' ? 'wss' : 'ws';
  ws = new WebSocket(`${proto}://${location.host}/ws`);
  ws.onopen = () => { retry = 0; setPill('ws-pill', true, 'LIVE'); };
  ws.onclose = () => {
    setPill('ws-pill', false, 'OFFLINE');
    retry = Math.min(retry + 1, 6);
    setTimeout(connect, 500 * 2 ** retry);
  };
  ws.onerror = () => ws && ws.close();
  ws.onmessage = e => {
    try { apply(JSON.parse(e.data)); } catch {}
  };
}

// ---------- HTTP fallback ----------
async function poll() {
  try {
    const r = await fetch('/api/data', { cache: 'no-store' });
    if (r.ok) apply(await r.json());
  } catch {}
}
setInterval(poll, 5000);

// ---------- Apply state ----------
function apply(d) {
  const t  = num(d.t),  h  = num(d.h),  p  = num(d.p),  db = num(d.db);

  setVal('t', fmt(t, 1));
  setVal('h', fmt(h, 0));
  setVal('p', fmt(p, 0));
  setVal('d', db === null || db < 0 ? '--' : Math.round(db));

  // humidity gauge 0..100
  setBar('h-bar', clamp01(h / 100));
  // sound gauge 30..100 dB
  setBar('d-bar', clamp01(((db ?? 0) - 30) / 70));

  // history
  if (t !== null) push(series.t, t);
  if (db !== null && db >= 0) push(series.d, db);

  drawSpark('t-chart', series.t, getCss('--mag'));
  drawSpark('d-chart', series.d, getCss('--cyan'));

  // deltas + range
  const tr = range(series.t);
  $('t-range').textContent = tr ? `${tr.min.toFixed(1)} / ${tr.max.toFixed(1)}` : '— / —';
  const td = delta(series.t);
  setDelta('t-delta', td, 'Δ ', '°');

  // pressure trend
  const pd = trend(p);
  $('p-trend').textContent = pd.label;
  $('p-trend').className = 'delta ' + pd.cls;

  // status pills come from /api/sys
  $('last').textContent = new Date().toLocaleTimeString();
}

// ---------- helpers ----------
function num(v){ return (v === undefined || v === null || Number.isNaN(v)) ? null : Number(v); }
function clamp01(x){ return Math.max(0, Math.min(1, x ?? 0)); }
function push(arr, v){ arr.push(v); if (arr.length > HIST) arr.shift(); }
function range(a){ if (!a.length) return null; let mn=a[0],mx=a[0]; for (const v of a){ if (v<mn)mn=v; if(v>mx)mx=v; } return {min:mn,max:mx}; }
function delta(a){ if (a.length < 2) return 0; return a[a.length-1] - a[a.length-2]; }

let lastP = null;
function trend(p){
  if (p === null) return { label:'→ —', cls:'' };
  if (lastP === null) { lastP = p; return { label:'→ stable', cls:'' }; }
  const d = p - lastP; lastP = p;
  if (d >  0.3) return { label:'↗ rising',  cls:'up' };
  if (d < -0.3) return { label:'↘ falling', cls:'down' };
  return { label:'→ stable', cls:'' };
}

function setVal(id, txt){
  const el = $(id); if (!el) return;
  if (el.textContent !== String(txt)){
    el.textContent = txt;
    const card = el.closest('.value'); if (card){ card.classList.remove('flash'); void card.offsetWidth; card.classList.add('flash'); }
  }
}
function setBar(id, frac){ const el = $(id); if (el) el.style.width = (frac*100).toFixed(1) + '%'; }
function setDelta(id, d, prefix, suffix){
  const el = $(id); if (!el) return;
  el.textContent = `${prefix}${d>=0?'+':''}${d.toFixed(1)}${suffix||''}`;
  el.className = 'delta ' + (d > 0.05 ? 'up' : d < -0.05 ? 'down' : '');
}
function setPill(id, ok, text){
  const p = $(id); if (!p) return;
  p.classList.toggle('ok', !!ok);
  p.classList.toggle('bad', !ok);
  const t = id === 'ws-pill' ? $('ws-text') : null;
  if (t) t.textContent = text;
}
function getCss(v){ return getComputedStyle(document.documentElement).getPropertyValue(v).trim() || '#22e3ff'; }

// ---------- sparkline ----------
function drawSpark(id, data, color){
  const c = $(id); if (!c) return;
  const dpr = window.devicePixelRatio || 1;
  const w = c.clientWidth, h = c.clientHeight;
  if (c.width !== w*dpr || c.height !== h*dpr){ c.width = w*dpr; c.height = h*dpr; }
  const ctx = c.getContext('2d');
  ctx.setTransform(dpr,0,0,dpr,0,0);
  ctx.clearRect(0,0,w,h);
  if (data.length < 2) return;

  let mn = Infinity, mx = -Infinity;
  for (const v of data){ if (v<mn)mn=v; if(v>mx)mx=v; }
  if (mx-mn < 1e-6){ mx = mn+1; }

  const sx = w / (HIST - 1);
  const py = v => h - 4 - ((v - mn) / (mx - mn)) * (h - 8);

  // area fill
  const grad = ctx.createLinearGradient(0,0,0,h);
  grad.addColorStop(0, color + '55');
  grad.addColorStop(1, color + '00');
  ctx.beginPath();
  ctx.moveTo(0, h);
  data.forEach((v,i)=> ctx.lineTo(i*sx, py(v)));
  ctx.lineTo((data.length-1)*sx, h);
  ctx.closePath();
  ctx.fillStyle = grad; ctx.fill();

  // line
  ctx.beginPath();
  data.forEach((v,i)=>{ const x=i*sx, y=py(v); i?ctx.lineTo(x,y):ctx.moveTo(x,y); });
  ctx.lineWidth = 1.5;
  ctx.strokeStyle = color;
  ctx.shadowBlur = 10; ctx.shadowColor = color;
  ctx.stroke();
  ctx.shadowBlur = 0;
}

// ---------- system status ----------
async function sysPoll(){
  try {
    const r = await fetch('/api/sys', { cache:'no-store' });
    if (!r.ok) return;
    const s = await r.json();
    setPill('wifi-pill', !!s.wifi, 'WIFI');
    setPill('sd-pill', !!s.sd, 'SD');
    if (s.uptimeMs !== undefined) $('uptime').textContent = formatUptime(s.uptimeMs);
    if (s.ip) $('hostinfo').textContent = s.host ? `${s.host} · ${s.ip}` : s.ip;
  } catch {}
}
setInterval(sysPoll, 3000);
sysPoll();

function formatUptime(ms){
  const s = Math.floor(ms/1000);
  const d = Math.floor(s/86400), h = Math.floor(s%86400/3600), m = Math.floor(s%3600/60), ss = s%60;
  const pad = n => String(n).padStart(2,'0');
  return d ? `${d}d ${pad(h)}:${pad(m)}:${pad(ss)}` : `${pad(h)}:${pad(m)}:${pad(ss)}`;
}

// ---------- clock ----------
setInterval(()=>{
  const d = new Date();
  $('clock').textContent = d.toTimeString().slice(0,8);
}, 1000);

connect();
poll();
