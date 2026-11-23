// Runs the dashboard page served by the STM32 (firmware/web/index.html)
// in headless Chromium with a fake WebSocket, and checks the diagnostics UI:
// ECU scan list, DTC read/decode, CAN stats, live PIDs. Exit 0 = pass.
// The last output line is the reason on failure (read by testresult.py).
const fs = require('fs');
const path = require('path');
// Chromium is installed inside firmware/node_modules (PLAYWRIGHT_BROWSERS_PATH=0),
// so nothing is placed outside the project folder.
if (!process.env.PLAYWRIGHT_BROWSERS_PATH) process.env.PLAYWRIGHT_BROWSERS_PATH = '0';
const { chromium } = require('playwright');

// The STM32 serves web/index.html (single-board build; the ESP32 is retired).
const html = fs.readFileSync(path.join(__dirname, '..', 'web', 'index.html'), 'utf8');

const fakeWs = `
window.__sent = [];
window.__authOk = false;
class FakeWS {
  constructor(){ this.readyState = 0; window.__ws = this;
    setTimeout(()=>{ this.readyState = 1; this.onopen && this.onopen(); window.__authOk = true; }, 10); }
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
  await page.waitForFunction(() => window.__authOk === true, null, { timeout: 5000 });

  // ECU scan
  await page.evaluate(() => { window.__sent.length = 0; scanEcu('x29'); });
  const scan = await page.evaluate(() => window.__sent[0]);
  if (!scan || scan.op !== 'ecuscan' || scan.format !== 'E' || scan.first !== '00' || scan.last !== 'FF' || scan.req !== '3E00')
    fail('29-bit scan command wrong: ' + JSON.stringify(scan));
  await page.evaluate(() => { __diag('ECU,F,FOUND,7E0,7E8,064100BE3FA81300'); __diag('ECU,F,DONE,1'); });
  const rows = await page.textContent('#ecuRows');
  if (!rows.includes('7E8')) fail('found ECU not listed');
  const opt = await page.evaluate(() => document.querySelectorAll('#rstEcu option').length);
  if (opt !== 1) fail('reset ECU list not filled');

  // Bus-tagged ARM/DISARM acknowledgements must update the visible state.
  await page.evaluate(() => { __diag('STATUS,F,ARMED,12'); });
  if (!/1[12] s left/.test(await page.textContent('#armState'))) fail('bus-tagged ARM acknowledgement not shown');
  await page.evaluate(() => { __diag('STATUS,F,DISARMED,KEY_OFF'); });
  if (!(await page.textContent('#armState')).includes('Disarmed')) fail('bus-tagged DISARM acknowledgement not shown');

  // answer the queued UDS requests one by one
  // Wait (condition, not a fixed sleep) until the page has sent the expected
  // request: the UI queue paces requests with timers, so a loaded machine is
  // slower but must not change the outcome.
  async function answer(expectReq, lines, delayMs = 60) {
    const latest = () => page.evaluate(() => { const u = window.__sent.filter(x => x.op === 'uds'); return u[u.length - 1]; });
    try {
      await page.waitForFunction((r) => { const u = window.__sent.filter(x => x.op === 'uds'); return u.length && u[u.length - 1].req === r; },
                                 expectReq, { timeout: 10000, polling: 20 });
    } catch (e) { /* reported below with the request actually seen */ }
    const req = await latest();
    if (!req || req.req !== expectReq) { fail('expected request ' + expectReq + ', got ' + JSON.stringify(req)); return false; }
    await page.evaluate((ls) => ls.forEach(l => __diag(l)), lines);
    await page.waitForTimeout(delayMs);
    return true;
  }

  // DTCs
  await page.evaluate(() => { window.__sent.length = 0; readDtcs(); });
  await answer('03', ['UDS,F,DATA,7E8,0,4302013304200000', 'UDS,F,POS,7E8,8']);
  await answer('07', ['UDS,F,NEG,7E8,07,12']);
  await answer('0A', ['UDS,F,ERR,TIMEOUT']);
  await answer('1902FF', ['UDS,F,DATA,7E8,0,5902FF01330009C0730108', 'UDS,F,POS,7E8,11']);
  const dtc = await page.textContent('#dtcRows');
  for (const want of ['P0133', 'P0420', 'P0133-00', 'confirmed', 'U0073-01'])
    if (!dtc.includes(want)) fail('DTC table missing ' + want + ': ' + dtc);
  const confs = await page.evaluate(() => window.__sent.filter(x => x.op === 'uds').map(x => x.confirm));
  if (confs.some(c => c)) fail('read requests must not be confirmed');

  // clear DTCs is confirmed
  await page.evaluate(() => { window.__sent.length = 0; clearDtcs(); });
  const clr = await page.evaluate(() => window.__sent.filter(x => x.op === 'uds')[0]);
  if (!clr || clr.req !== '04' || clr.confirm !== true) fail('clear DTC request wrong: ' + JSON.stringify(clr));
  await answer('04', ['UDS,F,DATA,7E8,0,44', 'UDS,F,POS,7E8,1']);
  await answer('14FFFFFF', ['UDS,F,NEG,7E8,14,11']);

  // Persistent DTC event journal: render current, acknowledge, then history.
  await page.evaluate(() => __deliver({ type: 'events_reset' }));
  await page.evaluate(() => __deliver({ type: 'event', id: 7, code: 'P0133', bus: 'F', ecu: '7E8', source: 'OBD stored', status: 'reported by ECU', timestamp: '2026-11-14T12:00:00Z', seen: false, repeats: 1, snapshotReady: true, rpm: 820, speed: 0, pids: '05=88.0;0C=820.0;' }));
  if (!(await page.textContent('#eventCurrent')).includes('P0133')) fail('current DTC event not rendered');
  if (!(await page.locator('#eventCurrent').innerHTML()).includes('/event.csv?id=7')) fail('event snapshot download link missing');
  if (!(await page.textContent('#eventSummary')).includes('1 current')) fail('current event count wrong');
  await page.evaluate(() => markEventSeen(7));
  const ack = await page.evaluate(() => window.__sent[window.__sent.length - 1]);
  if (!ack || ack.op !== 'event_seen' || ack.id !== 7) fail('event acknowledgement command wrong: ' + JSON.stringify(ack));
  await page.evaluate(() => __deliver({ type: 'event', id: 7, code: 'P0133', bus: 'F', ecu: '7E8', source: 'OBD stored', status: 'reported by ECU', timestamp: '2026-11-14T12:00:00Z', seen: true, repeats: 1, rpm: 820, speed: 0, pids: '05=88.0;0C=820.0;' }));
  if (!(await page.textContent('#eventHistory')).includes('P0133')) fail('seen event not moved to history');
  if (!(await page.textContent('#eventSummary')).includes('1 historical')) fail('historical event count wrong');

  // stats
  await page.evaluate(() => window.__deliver({ type: 'stats', message: '100,0,0,1,2,0,5,ACTIVE' }));
  if ((await page.textContent('#bhTecRec')) !== '0 / 5') fail('TEC/REC not shown');
  if ((await page.textContent('#bhState')) !== 'ACTIVE') fail('CAN state not shown');

  // Key II alone must not transmit; automatic polling starts only after explicit F-CAN ARM.
  await page.evaluate(() => { window.__sent.length = 0; __deliver({type:'veh',state:'ON',rpm:0,spd:0,acc:0,src:'NONE'}); });
  if (await page.evaluate(() => window.__sent.some(x => x.op === 'uds'))) fail('PID polling started before ARM');
  await page.evaluate(() => __diag('STATUS,F,ARMED,60'));
  // Honda Civic model-year 2012/2013 read-only gear DID: ECU 0E, 29-bit.
  // Pause repeat cadence only in this harness so the remaining OBD queue is deterministic.
  await page.evaluate(() => { hondaGearPollMs=60000; $('gearLedSrc').value='v2'; $('obdbOn').checked=false; });
  const gearPayload = await page.evaluate(() => {
    const b=[0x62,0x26,0x12,...Array(47).fill(0)]; b[47]=4; b[49]=3;
    return b.map(x=>x.toString(16).padStart(2,'0')).join('').toUpperCase();
  });
  const gearReq = await page.evaluate(() => window.__sent.filter(x => x.op === 'uds').slice(-1)[0]);
  if (!gearReq || gearReq.req !== '222612' || gearReq.format !== 'E' || gearReq.tx !== '18DA0EF1' || gearReq.rx !== '18DAF10E' || gearReq.confirm)
    fail('Honda gear DID request wrong: ' + JSON.stringify(gearReq));
  await page.evaluate((h) => {
    __diag('UDS,F,DATA,18DAF10E,0,'+h.slice(0,64));
    __diag('UDS,F,DATA,18DAF10E,32,'+h.slice(64,128));
    __diag('UDS,F,POS,18DAF10E,50');
  }, gearPayload);
  await page.waitForTimeout(250);
  if ((await page.textContent('#hondaGearV2')) !== '4' || (await page.textContent('#hondaGearV3')) !== '3')
    fail('Honda gear candidate fields did not decode: '+await page.textContent('#hondaGearState'));
  // One decoded value must NOT reach the STM32 LEDs yet (two consecutive identical replies required).
  if (await page.evaluate(() => window.__sent.some(x => x.op === 'gear'))) fail('gear forwarded to STM32 after a single reply');
  await answer('0100', ['UDS,F,DATA,7E8,0,410018000000', 'UDS,F,POS,7E8,6'], 250);
  // The R18Z1 5AT substitute candidate (ECU 1E, first of 2301/2322/3086) is read-only, 29-bit, after DID 2612.
  const altReq = await page.evaluate(() => window.__sent.filter(x => x.op === 'uds').slice(-1)[0]);
  if (!altReq || altReq.req !== '222301' || altReq.format !== 'E' || altReq.tx !== '18DA1EF1' || altReq.rx !== '18DAF11E' || altReq.confirm)
    fail('Honda DA1E substitute DID request wrong: ' + JSON.stringify(altReq));
  await answer('222301', ['UDS,F,DATA,18DAF11E,0,62230105A0', 'UDS,F,POS,18DAF11E,5'], 250);
  if ((await page.textContent('#hondaAlt2301')) !== '05 a0') fail('DA1E 2301 raw bytes not shown: ' + await page.textContent('#hondaAlt2301'));
  await page.evaluate(() => { hondaAlt.pollMs = 60000; hondaAlt.last = Date.now(); });
  await answer('0104', ['UDS,F,DATA,7E8,0,410480', 'UDS,F,POS,7E8,3'], 250);
  // Queue a second 2612 read behind the in-flight 0105: an identical reply is the
  // debounce confirmation and must forward the gear exactly once as a LED-only op.
  await page.evaluate(() => { clearTimeout(hondaGearTimer); hondaGearPending = false; pollHondaGear(); });
  await answer('0105', ['UDS,F,DATA,7E8,0,41057B', 'UDS,F,POS,7E8,3'], 250);
  const pid = await page.textContent('#pidRows');
  if (!pid.includes('50.2 %') || !pid.includes('83 °C')) fail('automatic PID values wrong: ' + pid);
  await answer('222612', [], 50);
  await page.evaluate((h) => {
    __diag('UDS,F,DATA,18DAF10E,0,'+h.slice(0,64));
    __diag('UDS,F,DATA,18DAF10E,32,'+h.slice(64,128));
    __diag('UDS,F,POS,18DAF10E,50');
  }, gearPayload);
  await page.waitForTimeout(250);
  const gearOps = await page.evaluate(() => window.__sent.filter(x => x.op === 'gear'));
  if (gearOps.length !== 1 || gearOps[0].gear !== 4) fail('debounced gear not forwarded exactly once: ' + JSON.stringify(gearOps));
  if ((await page.textContent('#gearLedLast')) !== '4 (v2)') fail('gear LED last-sent label wrong');
  await page.evaluate(() => { clearTimeout(hondaGearTimer); hondaGearPending = false; });

  // Global Write display mode changes the dashboard palette but never stops Read monitoring.
  if (await page.locator('#tabR, #tabW').count()) fail('duplicate mode controls remain; only one global toggle is allowed');
  if (await page.locator('#globalModeToggle').count() !== 1) fail('single global Write toggle is missing');
  await page.click('#globalModeToggle');
  const wrOn = await page.evaluate(() => window.__sent.filter(x => x.op === 'write').slice(-1)[0]);
  if (!wrOn || wrOn.on !== true) fail('Write toggle did not request WRITE,ON from the board');
  if ((await page.locator('body').getAttribute('class') || '').includes('write-mode')) fail('UI entered write mode before the board confirmed');
  await page.evaluate(() => __diag('STATUS,SYS,WRITE,ON'));
  if (!(await page.locator('body').getAttribute('class') || '').includes('write-mode')) fail('global Write palette not enabled');
  const anim = await page.locator('#globalModeToggle').evaluate(e => getComputedStyle(e).animationName);
  if (anim !== 'wpulse') fail('write mode is not pulsing: ' + anim);
  if ((await page.locator('#pnlR').evaluate(e => getComputedStyle(e).display)) === 'none') fail('Read panel hidden in Write mode');
  if ((await page.locator('#pnlW').evaluate(e => getComputedStyle(e).display)) === 'none') fail('Write panel not shown');
  const writesBeforeDraft = await page.evaluate(() => window.__sent.length);
  const liveField = page.locator('#pidRows b[data-field-id]').first();
  if (!(await liveField.count())) fail('decoded live PID value is not editable/stageable');
  await liveField.dblclick();
  if ((await page.locator('#fieldEditor').evaluate(e => getComputedStyle(e).display)) === 'none') fail('double-click editor did not open');
  await page.fill('#fieldEditInput', '81.5');
  await page.click('#fieldEditorSend');
  const staged = await page.textContent('#editDraftRows');
  if (!staged.includes('81.5') || !staged.includes('STAGED')) fail('proposed edit not staged: ' + staged);
  const writesAfterDraft = await page.evaluate(() => window.__sent.length);
  if (writesAfterDraft !== writesBeforeDraft) fail('local stage editor transmitted a command');
  await page.click('#globalModeToggle');
  const wrOff = await page.evaluate(() => window.__sent.filter(x => x.op === 'write').slice(-1)[0]);
  if (!wrOff || wrOff.on !== false) fail('Write toggle did not request WRITE,OFF');
  await page.evaluate(() => __diag('STATUS,SYS,WRITE,OFF'));
  if ((await page.locator('body').getAttribute('class') || '').includes('write-mode')) fail('global Read palette did not restore');
  if ((await page.locator('#pnlR').evaluate(e => getComputedStyle(e).display)) === 'none') fail('Read panel hidden after mode toggle');

  const dtcPoll = await page.evaluate(() => window.__sent.filter(x => x.op === 'uds').slice(-1)[0]);
  if (!dtcPoll || dtcPoll.req !== '03') fail('automatic DTC polling did not start: ' + JSON.stringify(dtcPoll));
  await answer('03', ['UDS,F,DATA,7E8,0,43000000', 'UDS,F,POS,7E8,4'], 250);
  await answer('07', ['UDS,F,DATA,7E8,0,47000000', 'UDS,F,POS,7E8,4'], 250);
  await answer('0A', ['UDS,F,DATA,7E8,0,4A000000', 'UDS,F,POS,7E8,4'], 250);
  await answer('1902FF', ['UDS,F,DATA,7E8,0,5902FF00000000', 'UDS,F,POS,7E8,7'], 250);
  await page.evaluate(() => __diag('STATUS,F,DISARMED,COMMAND'));
  if (!(await page.textContent('#autoPidState')).includes('paused')) fail('automatic PID polling did not pause on DISARM');
  if (await page.evaluate(() => hondaGearOn)) fail('Honda gear DID poll remained active after DISARM');
  if (!(await page.textContent('#hondaGearState')).includes('POLLING STOPPED')) fail('Honda gear status did not report polling stopped');
  const sentAfterDisarm = await page.evaluate(() => window.__sent.length);
  await page.waitForTimeout(300);
  if (await page.evaluate(n => window.__sent.slice(n).some(x => x.op === 'uds'), sentAfterDisarm))
    fail('a read-only UDS request was sent after DISARM');

  // value read / write by identifier
  await page.evaluate(() => { window.__sent.length = 0; $('wrDid').value = 'F190'; didOp('read'); });
  const rd = await page.evaluate(() => window.__sent.filter(x => x.op === 'uds')[0]);
  if (!rd || rd.req !== '22F190' || rd.confirm) fail('DID read wrong: ' + JSON.stringify(rd));
  await answer('22F190', ['UDS,F,DATA,7E8,0,62F1904A484D', 'UDS,F,POS,7E8,6']);
  if (!(await page.textContent('#wrOut')).includes('"JHM"')) fail('DID read not decoded');
  await page.evaluate(() => { window.__sent.length = 0; $('wrDid').value = '0101'; $('wrData').value = '01 02'; didOp('write'); });
  const wr = await page.evaluate(() => window.__sent.filter(x => x.op === 'uds')[0]);
  if (!wr || wr.req !== '2E01010102' || wr.confirm !== true) fail('DID write wrong: ' + JSON.stringify(wr));

  await answer('2E01010102', ['UDS,F,NEG,7E8,2E,33']);
  if (!(await page.textContent('#wrOut')).includes('NRC 0x33')) fail('NRC not shown');

  // markup in a reply must not become HTML
  await page.evaluate(() => __diag('ECU,F,FOUND,7E1,7E9,<img src=x onerror=window.__xss=1>'));
  await page.waitForTimeout(50);
  if (await page.evaluate(() => window.__xss === 1)) fail('ECU reply rendered as HTML');

  // gear-LED source: broadcast 0x1A3 by default, re-applied on connect
  const gb = await page.evaluate(() => { $('gearLedSrc').value = 'bcast'; gearLedToggle(); return window.__sent.filter(x => x.op === 'gear_bcast').pop(); });
  if (await page.evaluate(() => document.querySelector('#gearLedSrc option[selected]').value) !== 'bcast') fail('page default gear source is not the broadcast');
  if (!gb || gb.on !== true) fail('broadcast gear source not enabled by default: ' + JSON.stringify(gb));
  await page.evaluate(() => { $('gearLedSrc').value = 'v2'; gearLedToggle(); });
  if (await page.evaluate(() => window.__sent.filter(x => x.op === 'gear_bcast').pop().on !== false)) fail('selecting DID 2612 did not disable broadcast gear');

  // decoded broadcast panel
  await page.evaluate(() => { $('gearLedSrc').value = 'bcast';
    __deliver({ type: 'dec', l: [['1A3', 'GEARBOX_AUTO', 'GEAR_SHIFTER', 4, '', 'D'], ['17C', 'POWERTRAIN_DATA', 'ENGINE_RPM', 750, 'rpm', '']] }); });
  await page.waitForTimeout(300);
  await page.evaluate(() => drawDec());
  const decTxt = await page.textContent('#decBody');
  if (!decTxt.includes('GEAR_SHIFTER') || !decTxt.includes('D (4)') || !decTxt.includes('750 rpm')) fail('decoded panel wrong: ' + decTxt);
  if (!(await page.textContent('#decState')).includes('LIVE')) fail('decoded panel not LIVE');
  await page.evaluate(() => __deliver({ type: 'dec', l: [['1A3', 'GEARBOX_AUTO', 'TRANS_TARGET_GEAR', 3, '', '3rd']] }));
  if (!(await page.textContent('#gearLedLast')).includes('3rd')) fail('broadcast gear not shown as last gear');

  // state capture: lever P vs D -> the bits of 0x1A3 byte 4 that differ
  const cap = await page.evaluate(() => {
    const fr = (hex) => __deliver({ type: 'frames', fps: [0, 0], bps: [0, 0], total: 0, locked: true, bitrate: 500000,
      l: [['1A3', 0, 8, hex, 1, 50], ['17C', 0, 8, '0000000000000000', 1, 100]] });
    $('capLabel').value = 'P'; capToggle(); for (let i = 0; i < 5; i++) fr('0000000001000000'); capToggle();
    $('capLabel').value = 'D'; capToggle(); for (let i = 0; i < 5; i++) fr('0000000004000000'); capToggle();
    capAnalyze();
    return { info: $('capInfo').textContent, body: $('capBody').textContent, r: capFind(cap.samples) };
  });
  if (!cap.info.includes('P: 10') || !cap.info.includes('D: 10')) fail('capture counts wrong: ' + cap.info);
  const bits = cap.r.bits.map(b => b.id + ':' + b.byte + ':' + b.bit + '=' + b.vals.P + b.vals.D).join(' ');
  if (bits !== '1A3:4:2=01 1A3:4:0=10') fail('bit finder wrong: ' + bits);

  // capture integrity banner
  const ci = await page.evaluate(() => {
    __deliver({ type: 'frames', fps: [0, 0], bps: [0, 0], total: 50, locked: true, bitrate: 500000, drop: [0, 0], over: 0, l: [] });
    const ok = $('capIntegrity').textContent;
    __deliver({ type: 'frames', fps: [0, 0], bps: [0, 0], total: 60, locked: true, bitrate: 500000, drop: [3, 0], over: 2, l: [] });
    return { ok, bad: $('capIntegrity').textContent, color: $('capIntegrity').style.color };
  });
  if (!ci.ok.includes('no frame dropped')) fail('integrity OK text wrong: ' + ci.ok);
  if (!ci.bad.includes('CAPTURE LOSS') || !ci.bad.includes('F-CAN 3') || !ci.bad.includes('2 frames')) fail('capture loss not shown: ' + ci.bad);

  // Honda DID layouts (OBDb) + system blocks
  const sys = await page.evaluate(() => {
    const vtec = OBDB.find(c => c.h === 'DA10' && c.did === '2612');
    const sh = OBDB.find(c => c.h === 'DA1E' && c.did === '2301');
    const b = new Array(60).fill(0); b[0] = 0x62; b[1] = 0x26; b[2] = 0x12;
    const ps = vtec.sigs.find(s => s.n === 'VTEC pressure switch');
    const bit = ps.b; b[3 + (bit >> 3)] |= 0x80 >> (bit & 7);
    const r1 = obdbVal(ps, b);
    const sig = sh.sigs.find(s => s.n === 'Input shaft speed');
    const b2 = new Array(60).fill(0); b2[0] = 0x62; b2[1] = 0x23; b2[2] = 0x01;
    b2[3 + (sig.b >> 3)] = 0x50;                              // first byte of the field
    const r2 = obdbVal(sig, b2);
    const nNamed = OBDB.filter(c => c.sigs.length).length, nRaw = OBDB.filter(c => !c.sigs.length).length;
    obdbState.res.x1 = { v: 1, t: 'ON', n: 'VTEC pressure switch', p: 'Engine', u: 'offon', h: 'DA10', did: '2612', ts: Date.now() };
    obdbState.res.x2 = { v: 1200, t: '', n: 'Input shaft speed', p: 'Transmission', u: 'rpm', h: 'DA1E', did: '2301', ts: Date.now() };
    obdbState.res.x3 = { v: 30, t: '', n: 'Tire pressure FL', p: 'Tires', u: 'psi', h: 'DA26', did: '1111', ts: Date.now() };
    sysDrawAt = 0; drawSystems();
    return { r1, r2, sigLen: sig.l, nNamed, nRaw, eng: $('sysEngine').textContent, tr: $('sysTrans').textContent, oth: $('sysOther').textContent, src: $('obdbSrc').textContent };
  });
  if (!sys.r1 || sys.r1.t !== 'ON') fail('VTEC bit decode wrong: ' + JSON.stringify(sys.r1));
  if (!sys.r2 || !(sys.r2.v > 0)) fail('input shaft speed decode wrong: ' + JSON.stringify(sys.r2));
  if (sys.nNamed < 20 || sys.nRaw < 100) fail('OBDb table not embedded: ' + sys.nNamed + '/' + sys.nRaw);
  if (!sys.eng.includes('VTEC pressure switch') || !sys.eng.includes('ON')) fail('engine block missing VTEC: ' + sys.eng.slice(0, 200));
  if (!sys.eng.includes('ENGINE_RPM')) fail('engine block missing broadcast RPM');
  if (!sys.tr.includes('Input shaft speed') || !sys.tr.includes('GEAR_SHIFTER')) fail('transmission block wrong: ' + sys.tr.slice(0, 200));
  if (!sys.oth.includes('Tire pressure FL')) fail('other block wrong');
  if (!sys.src.includes('OBDb/Honda-Civic')) fail('OBDb source not shown');
  // poller: read-only 22 requests on 29-bit Honda addressing, only while ARMed polling is on
  const pol = await page.evaluate(async () => {
    window.__sent.length = 0; idq.length = 0; idCur = null; $('obdbOn').checked = true;
    pidOn = false; hondaGearOn = false; obdbTick();
    const none = window.__sent.filter(x => x.op === 'uds').length;
    pidOn = true; hondaGearOn = true; obdbState.slot = 0; obdbTick();
    await new Promise(r => setTimeout(r, 50));
    const u = window.__sent.filter(x => x.op === 'uds');
    idDone(null, 'NO_ANSWER');                                 // that unit does not answer
    const first = u[0];
    const skipped = obdbState.noEcu[first.tx.slice(2, 6)] > 0;
    pidOn = false; hondaGearOn = false; clearTimeout(obdbState.timer); obdbState.timer = null;
    return { none, first, skipped };
  });
  if (pol.none !== 0) fail('Honda DID poll ran while not ARMed');
  if (!pol.first || pol.first.format !== 'E' || !/^18DA..F1$/.test(pol.first.tx) || !/^18DAF1..$/.test(pol.first.rx) || !/^22[0-9A-F]{4}$/.test(pol.first.req) || pol.first.confirm)
    fail('Honda DID request wrong: ' + JSON.stringify(pol.first));
  if (!pol.skipped) fail('silent control unit not skipped');

  if (errors.length) fail('page errors: ' + errors.join(' | '));
  // Engine start: one click, no confirmation dialog, no ARM requirement.
  await page.evaluate(() => { window.__sent.length = 0; window.confirm = () => false; doStart(); window.confirm = () => true; });
  const st = await page.evaluate(() => window.__sent.map(x => x.op));
  if (!st.includes('start')) fail('single-click engine start did not send start: ' + JSON.stringify(st));
  await page.evaluate(() => { window.__sent.length = 0; doStartCfg(); });
  const sc = await page.evaluate(() => window.__sent.filter(x => x.op === 'startcfg')[0]);
  if (sc && sc.armed !== 0) fail('engine start must not require ARM: ' + JSON.stringify(sc));

  // bxCAN abilities panel sends whitelisted ability lines.
  await page.evaluate(() => { window.__sent.length = 0; $('abBus').value = 'B'; abOpt('NART', true);
    $('abBank').value = '20'; $('abFf').value = '1';
    document.querySelector('#canAbilities button[onclick*="FILTER"]').click(); });
  const ab = await page.evaluate(() => window.__sent.filter(x => x.op === 'can').map(x => x.line));
  if (!ab.includes('OPT,B,NART,on')) fail('ability option not sent: ' + JSON.stringify(ab));
  if (!ab.some(l => l.startsWith('FILTER,B,20,M,32,1,'))) fail('filter program not sent: ' + JSON.stringify(ab));
  await page.evaluate(() => __diag('STATUS,B,OPTS,ABOM=1,AWUM=0,NART=1,RFLM=0,TXFP=0,TTCM=1,DBF=1,SJW=3'));
  if (!(await page.locator('#abNART').isChecked()) || (await page.inputValue('#abSJW')) !== '3') fail('OPTS status not reflected in panel');
  // Connected device: detected banner -> select; NEW -> add; EXISTS -> observe prompt; settings list.
  await page.evaluate(() => { window.__sent.length = 0; __diag('DEVICE,DETECTED,1,Honda Civic 2013 9th gen R18Z1,950'); });
  if (!(await page.textContent('#deviceBanner')).includes('Honda Civic 2013')) fail('detected device name not shown');
  await page.evaluate(() => document.querySelector('#deviceBanner button').click());
  let dv = await page.evaluate(() => window.__sent.filter(x => x.op === 'can').map(x => x.line));
  if (!dv.includes('DEVICE,SELECT,1')) fail('select did not send DEVICE,SELECT,1: ' + JSON.stringify(dv));
  await page.evaluate(() => __diag('DEVICE,TARGET,1,Honda Civic 2013 9th gen R18Z1'));
  if (await page.locator('#deviceBanner').isVisible()) fail('banner still shown after target selected');
  await page.evaluate(() => { window.__sent.length = 0; __diag('DEVICE,NEW,Unknown network F250k/B0k 5 IDs,0'); });
  await page.evaluate(() => { document.getElementById('devNewName').value = 'Bench rig'; });
  await page.evaluate(() => document.querySelector('#deviceBanner button').click());
  dv = await page.evaluate(() => window.__sent.filter(x => x.op === 'can').map(x => x.line));
  if (!dv.includes('DEVICE,ADD,Bench rig')) fail('add did not send DEVICE,ADD: ' + JSON.stringify(dv));
  await page.evaluate(() => { window.__sent.length = 0; window.confirm = () => true; __diag('DEVICE,EXISTS,2,Bench rig'); });
  dv = await page.evaluate(() => window.__sent.filter(x => x.op === 'can').map(x => x.line));
  if (!dv.includes('DEVICE,SELECT,2')) fail('already-added prompt did not offer observe/select');
  await page.evaluate(() => { devList(); __diag('DEVICE,ITEM,1,Honda Civic 2013 9th gen R18Z1,500000,0,36,VERIFIED'); __diag('DEVICE,ITEM,2,Bench rig,250000,0,5,VERIFIED'); __diag('DEVICE,LIST,END'); });
  const devRowCount = await page.locator('#devRows tr').count();
  if (devRowCount !== 2) fail('settings device list has ' + devRowCount + ' rows');
  if (await page.locator('#sideNav a[href="#settings"]').count() !== 1) fail('Settings menu missing from side bar');
  // Log email settings go to the board; the password field is cleared after save.
  await page.evaluate(() => { window.__sent.length = 0; $('mailPass').value = 'abcd efgh ijkl mnop'; mailSave(); });
  const mc = await page.evaluate(() => window.__sent.filter(x => x.op === 'mailcfg')[0]);
  if (!mc || mc.to !== 'stringswebvortex@gmail.com' || mc.pass !== 'abcd efgh ijkl mnop') fail('mail settings not sent: ' + JSON.stringify(mc));
  if ((await page.inputValue('#mailPass')) !== '') fail('app password left in the page');
  await page.evaluate(() => __diag('MAIL,PENDING'));
  if (!(await page.textContent('#mailState')).includes('PENDING')) fail('mail state not shown');
  // Boot view: detection screen hides the dashboard; a selected target boots into its own view.
  await page.evaluate(() => __diag('DEVICE,DETECTING'));
  if (!(await page.evaluate(() => document.body.classList.contains('detecting')))) fail('detection mode not shown');
  if (await page.locator('#canAnalyzer').isVisible()) fail('dashboard visible during detection');
  if (!(await page.locator('#settings').isVisible())) fail('settings must stay reachable during detection');
  await page.evaluate(() => __diag('DEVICE,TARGET,1,Honda Civic 2013 9th gen R18Z1'));
  if (await page.evaluate(() => document.body.classList.contains('detecting'))) fail('target selected but still detecting');
  if (!(await page.textContent('#pageTitle')).includes('Honda Civic 2013')) fail('device view title missing');
  // Signal learning table: full list, then a live change.
  await page.evaluate(() => { __deliver({ type: 'sigstates', reset: true, list: [['158','ENGINE_DATA','CHECKED',40,0,0,8],['17C','POWERTRAIN_DATA','SEED',0,0,0,0]] });
                              __deliver({ type: 'sigstate', id: '158', name: 'ENGINE_DATA', state: 'VERIFIED', frames: 60, match: 10, miss: 0, dlc: 8 }); });
  const sl = await page.textContent('#slRows');
  if (!sl.includes('ENGINE_DATA') || !sl.includes('VERIFIED') || !sl.includes('POWERTRAIN_DATA')) fail('signal learning table: ' + sl);
  if (!(await page.textContent('#slSum')).startsWith('1 verified')) fail('signal learning summary');
  if (await page.locator('#sideNav a[href="#sigLearn"]').count() !== 1) fail('Signal Learning missing from side bar');
  // Local CAN AI baseline learning and anomaly scoring are read-only.
  if (await page.locator('#fwCard, #esp32File, #stm32File').count()) fail('iPhone dashboard still exposes the ESP32-dependent firmware uploader');
  await page.evaluate(() => { localStorage.removeItem(CAN_AI_KEY); canAIClearModel(); canAIStartTraining(); window.__ctInitial = JSON.stringify(ctWq); });
  if (await page.evaluate(() => canAIFinishTraining()) !== false) fail('AI accepted an empty training baseline');
  const validAiFrame = ['17C',0,8,'0000000000000000',10,2.0];
  await page.evaluate(r => canAIProcessFrames(Array(600).fill(r)), validAiFrame);
  const learned = await page.evaluate(() => ({mode:canAI.mode,n:canAI.model['F:17C'].n,samples:canAI.samples,encoderChanged:JSON.stringify(ctWq)!==window.__ctInitial}));
  if (learned.mode!=='training' || learned.n!==600 || learned.samples!==600 || !learned.encoderChanged) fail('AI baseline training count wrong: '+JSON.stringify(learned));
  const invalidAi = await page.evaluate(() => canAIFeatures(['bad',2,9,'xx',0,NaN])===null);
  if (!invalidAi) fail('AI feature extractor accepted malformed CAN data');
  await page.evaluate(() => canAIFinishTraining());
  if (await page.textContent('#aiState')!=='TRANSFORMER ACTIVE') fail('Transformer did not enter prediction mode');
  await page.evaluate(r => canAIProcessFrames([r]), validAiFrame);
  if (await page.evaluate(() => canAI.total)!==0) fail('AI model flagged an unchanged baseline as anomalous');
  const sentBeforeAi = await page.evaluate(() => window.__sent.length);
  await page.evaluate(r => canAIProcessFrames([r]), ['17C',0,8,'0000000000000000',10,100.0]);
  const aiResult = await page.evaluate(() => ({total:canAI.total,score:canAI.anomalies[0]&&canAI.anomalies[0].score,feature:canAI.anomalies[0]&&canAI.anomalies[0].feature}));
  if (aiResult.total!==1 || !(aiResult.score>1) || aiResult.feature!=='sequence forecast') fail('AI anomaly score incorrect: '+JSON.stringify(aiResult));
  if (await page.evaluate(n => window.__sent.length!==n, sentBeforeAi)) fail('AI monitor transmitted a CAN command');
  await page.evaluate(() => { canAISaveModel(); canAI.model=Object.create(null); canAI.mode='idle'; canAI.samples=0; canAILoadModel(); canAIRender(); });
  if (await page.evaluate(() => canAI.mode)!=='monitor') fail('saved AI baseline did not reload');
  const ctShape = await page.evaluate(() => ({inputs:CT_INPUTS,width:CT_WIDTH,context:CT_CONTEXT,encoded:ctEncode([Array(CT_INPUTS).fill(0),Array(CT_INPUTS).fill(.1)]).length,predicted:ctPredict([Array(CT_INPUTS).fill(0),Array(CT_INPUTS).fill(.1)]).y.length}));
  if (ctShape.inputs!==12 || ctShape.width!==8 || ctShape.context!==8 || ctShape.encoded!==8 || ctShape.predicted!==12)
    fail('Transformer tensor dimensions wrong: '+JSON.stringify(ctShape));
  const ctReload = await page.evaluate(() => { const q=JSON.stringify(ctWq), ok=canTransformerSave(); canTransformer.head=null; ctWq=ctMatrix(CT_INPUTS,CT_WIDTH,99,0.5); canTransformer.mode='idle'; canAI.mode='idle'; return ok && canTransformerLoad() && canTransformer.mode==='monitor' && JSON.stringify(ctWq)===q; });
  if (!ctReload) fail('saved Transformer weights did not reload');
  await page.evaluate(() => canAIClearModel());
  if (await page.evaluate(() => Object.keys(canAI.model).length)!==0) fail('AI clear did not remove model');

  await browser.close();
  if (!process.exitCode) console.log('dashboard harness: all checks passed');
})().catch(e => { console.log('FAIL: ' + e.message); process.exit(1); });
