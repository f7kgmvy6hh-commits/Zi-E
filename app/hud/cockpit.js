(() => {
  'use strict';
  const q = selector => document.querySelector(selector);
  const safe = value => String(value ?? 'UNAVAILABLE').replace(/[&<>"']/g, c => ({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
  const chip = value => `<span class="truth-chip ${/UNAVAILABLE|BLOCKED|FAULT/.test(value)?'bad':/UNVERIFIED|WAITING|MODEL/.test(value)?'warn':'ok'}">${safe(value)}</span>`;

  async function enhance() {
    if (!q('#token')?.value) return;
    try {
      const response = await fetch('/api/control-center', {headers:{Authorization:'Bearer '+q('#token').value}});
      if (!response.ok) throw new Error(response.status === 401 ? 'AUTH_REQUIRED' : 'UNAVAILABLE');
      const c = await response.json();
      q('#version').textContent = c.version;
      document.title = `ZI-E Control Center ${c.version}`;
      q('#top-esp32').textContent = c.controllers[0].state;
      q('#top-stm32').textContent = c.controllers[1].state;
      q('#top-safety').textContent = c.safety.motion_authority;
      q('#system-graph').innerHTML = [
        ['HostRuntime','Brain / coordinator',c.target.runtime.readiness],
        ['ESP32 Presence','Media / network / semantic bridge',c.presence.state],
        ['STM32 Safety','Physical rejection authority',c.controllers[1].state],
      ].map(([name,role,state])=>`<article class="system-node"><h3>${safe(name)}</h3><p>${safe(role)}</p>${chip(state)}</article>`).join('');
      const alerts = [
        ['CRITICAL','FIRST POWER NOT AUTHORIZED'],['BLOCKED','HW-002 NOT VERIFIED'],
        ['BLOCKED','STM32 TARGET MISSING'],['BLOCKED','PHASE2B2 WAITING_FOR_VERIFIED_INPUTS'],
        ['INFO','REAL TARGET UNAVAILABLE'],
      ];
      q('#global-alerts').innerHTML = alerts.map(([level,text])=>`<div class="alert-item ${level==='CRITICAL'?'critical':''}"><b>${level}</b>${safe(text)}</div>`).join('');
      q('#face-mode').innerHTML = `Mode ${chip(c.face_engine.mode)} · Host link ${chip(c.presence.host_link)}<br><small>${safe(c.face_engine.pack)} · ${safe(c.face_engine.variant)} · MODEL ONLY</small>`;
      q('#face-library').innerHTML = [['Pack','Zi-E Core Procedural'],['Version','1.0.0'],['Author','Zi-E Project'],['License','ZI-E OWNED'],['Variants','5'],['Storage','8 KiB estimate'],['Validation','HOST VALIDATED'],['Device','NOT INSTALLED']].map(([a,b])=>`<div class="metric"><span>${a}</span><strong>${b}</strong></div>`).join('');
      const flow = `<div id="provider-flow" class="provider-flow"><span>MIC ${chip(c.media.audio.capture)}</span><i>→</i><span>ESP32</span><i>→</i><span>STT</span><i>→</i><span>HERMES / LLM</span><i>→</i><span>TTS</span><i>→</i><span>ESP32 SPEAKER</span></div>`;
      q('#provider-flow')?.remove();
      q('#providers').insertAdjacentHTML('afterbegin', flow);
    } catch (error) {
      q('#global-alerts').innerHTML = `<div class="alert-item critical"><b>${safe(error.message)}</b>Provide the local token and verify the App connection.</div>`;
    }
  }

  function updateFace() {
    const face = q('#face-canvas'); if (!face) return;
    const gazeX = Number(q('#face-gaze-x').value) / 12;
    const gazeY = Number(q('#face-gaze-y').value) / 12;
    const speech = Number(q('#face-speech').value);
    const expression = (window.expression || 'neutral').toLowerCase();
    face.style.setProperty('--gx', `${gazeX}px`);
    face.style.setProperty('--gy', `${gazeY}px`);
    face.className = `face-canvas ${speech > 20 ? 'speaking' : ''} ${expression}`;
  }
  ['#face-intensity','#face-gaze-x','#face-gaze-y','#face-speech'].forEach(id => q(id)?.addEventListener('input', updateFace));
  q('#token')?.addEventListener('change', enhance);
  const originalRefresh = window.refresh;
  if (typeof originalRefresh === 'function') window.refresh = async (...args) => { const result = await originalRefresh(...args); await enhance(); return result; };
  setInterval(enhance, 10000);
  updateFace();
})();
