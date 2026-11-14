// Runs the ESP32 dashboard page (extracted from esp32-dashboard/src/main.cpp)
// in headless Chromium with a fake WebSocket, and checks the diagnostics UI:
// ECU scan list, DTC read/decode, CAN stats, live PIDs. Exit 0 = pass.
// The last output line is the reason on failure (read by testresult.py).
const fs = require('fs');
const path = require('path');
// Chromium is installed inside firmware/node_modules (PLAYWRIGHT_BROWSERS_PATH=0),
// so nothing is placed outside the project folder.
if (!process.env.PLAYWRIGHT_BROWSERS_PATH) process.env.PLAYWRIGHT_BROWSERS_PATH = '0';
const { chromium } = require('playwright');

const src = fs.readFileSync(path.join(__dirname, '..', 'esp32-dashboard', 'src', 'main.cpp'), 'utf8');
const m = src.match(/R"HTML\(([\s\S]*?)\)HTML"/);
if (!m) { console.log('page literal not found'); process.exit(1); }
const html = m[1];

const fakeWs = `
window.__sent = [];
class FakeWS {
  constructor(){ this.readyState = 0; window.__ws = this;
    setTimeout(()=>{ this.readyState = 1; this.onopen && this.onopen(); }, 10); }
  send(t){ window.__sent.push(JSON.parse(t)); }
  close(){}
}
window.WebSocket = FakeWS;
window.fetch = async () => ({ ok: true, text: async () => 'tok123' });
window.confirm = () => true;
window.__deliver = (o) => window.__ws.onmessage({ data: JSON.stringify(o) });
window.__diag = (t) => window.__deliver({ type: 'diag', message: t });
`;

function fail(msg) { console.log('FAIL: ' + msg); process.exitCode = 1; }

(async () => {
  const browser = await chromium.launch();
  const page = await browser.newPage();
  const errors = [];
  page.on('pageerror', e => errors.push(String(e)));
  await page.addInitScript(fakeWs);
  await page.route('http://dash.test/', r => r.fulfill({ contentType: 'text/html', body: html }));
  await page.goto('http://dash.test/');
  await page.waitForFunction(() => window.__sent.some(x => x.op === 'auth'), null, { timeout: 5000 });
  const auth = await page.evaluate(() => window.__sent.find(x => x.op === 'auth'));
  if (auth.token !== 'tok123') fail('auth token not sent');
  await page.evaluate(() => window.__deliver({ type: 'auth', ok: true }));

  // ECU scan
  await page.evaluate(() => { window.__sent.length = 0; scanEcu('x29'); });
  const scan = await page.evaluate(() => window.__sent[0]);
  if (!scan || scan.op !== 'ecuscan' || scan.format !== 'E' || scan.first !== '00' || scan.last !== 'FF' || scan.req !== '3E00')
    fail('29-bit scan command wrong: ' + JSON.stringify(scan));
  await page.evaluate(() => { __diag('ECU,FOUND,7E0,7E8,064100BE3FA81300'); __diag('ECU,DONE,1'); });
  const rows = await page.textContent('#ecuRows');
  if (!rows.includes('7E8')) fail('found ECU not listed');
  const opt = await page.evaluate(() => document.querySelectorAll('#rstEcu option').length);
  if (opt !== 1) fail('reset ECU list not filled');

  // answer the queued UDS requests one by one
  async function answer(expectReq, lines) {
    const req = await page.evaluate(() => { const u = window.__sent.filter(x => x.op === 'uds'); return u[u.length - 1]; });
    if (!req || req.req !== expectReq) { fail('expected request ' + expectReq + ', got ' + JSON.stringify(req)); return false; }
    await page.evaluate((ls) => ls.forEach(l => __diag(l)), lines);
    await page.waitForTimeout(60);
    return true;
  }

  // DTCs
  await page.evaluate(() => { window.__sent.length = 0; readDtcs(); });
  await answer('03', ['UDS,DATA,7E8,0,4302013304200000', 'UDS,POS,7E8,8']);
  await answer('07', ['UDS,NEG,7E8,07,12']);
  await answer('0A', ['UDS,ERR,TIMEOUT']);
  await answer('1902FF', ['UDS,DATA,7E8,0,5902FF01330009C0730108', 'UDS,POS,7E8,11']);
  const dtc = await page.textContent('#dtcRows');
  for (const want of ['P0133', 'P0420', 'P0133-00', 'confirmed', 'U0073-01'])
    if (!dtc.includes(want)) fail('DTC table missing ' + want + ': ' + dtc);
  const confs = await page.evaluate(() => window.__sent.filter(x => x.op === 'uds').map(x => x.confirm));
  if (confs.some(c => c)) fail('read requests must not be confirmed');

  // clear DTCs is confirmed
  await page.evaluate(() => { window.__sent.length = 0; clearDtcs(); });
  const clr = await page.evaluate(() => window.__sent.filter(x => x.op === 'uds')[0]);
  if (!clr || clr.req !== '04' || clr.confirm !== true) fail('clear DTC request wrong: ' + JSON.stringify(clr));
  await answer('04', ['UDS,DATA,7E8,0,44', 'UDS,POS,7E8,1']);
  await answer('14FFFFFF', ['UDS,NEG,7E8,14,11']);

  // stats
  await page.evaluate(() => window.__deliver({ type: 'stats', message: '100,0,0,1,2,0,5,ACTIVE' }));
  if ((await page.textContent('#bhTecRec')) !== '0 / 5') fail('TEC/REC not shown');
  if ((await page.textContent('#bhState')) !== 'ACTIVE') fail('CAN state not shown');

  // live PIDs
  await page.evaluate(() => { window.__sent.length = 0; togglePids(); });
  await answer('0100', ['UDS,DATA,7E8,0,410018000000', 'UDS,POS,7E8,6']);
  await answer('0104', ['UDS,DATA,7E8,0,410480', 'UDS,POS,7E8,3']);
  await answer('0105', ['UDS,DATA,7E8,0,41057B', 'UDS,POS,7E8,3']);
  const pid = await page.textContent('#pidRows');
  if (!pid.includes('50.2 %') || !pid.includes('83 °C')) fail('PID values wrong: ' + pid);
  await page.evaluate(() => togglePids());

  // value read / write by identifier
  await page.evaluate(() => { window.__sent.length = 0; $('wrDid').value = 'F190'; didOp('read'); });
  const rd = await page.evaluate(() => window.__sent.filter(x => x.op === 'uds')[0]);
  if (!rd || rd.req !== '22F190' || rd.confirm) fail('DID read wrong: ' + JSON.stringify(rd));
  await answer('22F190', ['UDS,DATA,7E8,0,62F1904A484D', 'UDS,POS,7E8,6']);
  if (!(await page.textContent('#wrOut')).includes('"JHM"')) fail('DID read not decoded');
  await page.evaluate(() => { window.__sent.length = 0; $('wrDid').value = '0101'; $('wrData').value = '01 02'; didOp('write'); });
  const wr = await page.evaluate(() => window.__sent.filter(x => x.op === 'uds')[0]);
  if (!wr || wr.req !== '2E01010102' || wr.confirm !== true) fail('DID write wrong: ' + JSON.stringify(wr));
  await answer('2E01010102', ['UDS,NEG,7E8,2E,33']);
  if (!(await page.textContent('#wrOut')).includes('NRC 0x33')) fail('NRC not shown');

  // markup in a reply must not become HTML
  await page.evaluate(() => __diag('ECU,FOUND,7E1,7E9,<img src=x onerror=window.__xss=1>'));
  await page.waitForTimeout(50);
  if (await page.evaluate(() => window.__xss === 1)) fail('ECU reply rendered as HTML');

  if (errors.length) fail('page errors: ' + errors.join(' | '));
  await browser.close();
  if (!process.exitCode) console.log('dashboard harness: all checks passed');
})().catch(e => { console.log('FAIL: ' + e.message); process.exit(1); });
