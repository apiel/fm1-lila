// LILA's browser build in node: boot the wasm, press a key, check that sound and a picture come out.
import { readFileSync } from "node:fs";
const wasm = readFileSync(new URL("../build/emu/lila.wasm", import.meta.url));
let mem;
const wasi = new Proxy({ clock_time_get: (id, p, out) => { new DataView(mem.buffer).setBigUint64(out, 0n, true); return 0; },
  fd_write: (fd, iov, n, out) => { new DataView(mem.buffer).setUint32(out, 0, true); return 0; }, proc_exit() {} },
  { get: (t, k) => (k in t ? t[k] : () => 0) });
const { instance } = await WebAssembly.instantiate(wasm, { wasi_snapshot_preview1: wasi, env: new Proxy({}, { get: () => () => 0 }) });
const ex = instance.exports; mem = ex.memory;
if (ex._initialize) ex._initialize();
new Uint8Array(mem.buffer, ex.web_log(), ex.web_log_size()).fill(0xFF);
ex.web_master(4000); ex.web_boot();
for (let i = 0; i < 40; i++) ex.web_render(128);          // the first frames
ex.web_keys(1 << 7);
let peak = 0;
for (let i = 0; i < 200; i++) { ex.web_render(128); for (const v of new Float32Array(mem.buffer, ex.web_out_l(), 128)) peak = Math.max(peak, Math.abs(v)); }
const fb = new Uint16Array(mem.buffer, ex.web_fb(), 240 * 240);
let colours = new Set(fb.slice(0, 240 * 120)); 
console.log(`peak ${peak.toFixed(3)}, ${colours.size} colours on screen, ${ex.web_blits()} blits`);
if (peak < 0.02 || colours.size < 20) { console.error("FAIL"); process.exit(1); }
console.log("web emu ok");
