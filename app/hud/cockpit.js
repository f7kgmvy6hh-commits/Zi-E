(() => {
  'use strict';

  const q = selector => document.querySelector(selector);
  const safe = value => String(value ?? 'UNAVAILABLE').replace(
    /[&<>"']/g,
    character => ({'&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;'}[character]),
  );
  const chip = value => {
    const state = String(value ?? 'UNAVAILABLE');
    const className = /UNAVAILABLE|BLOCKED|FAULT|NOT_AUTHORIZED|NOT_ARRIVED|INCOMPLETE|QUARANTINED/.test(state)
      ? 'bad'
      : /UNVERIFIED|WAITING|MODEL|NOT_TESTED|NOT_COMMISSIONED|PENDING/.test(state) ? 'warn' : 'ok';
    return `<span class="truth-chip ${className}">${safe(state)}</span>`;
  };

  function enhance(cockpit) {
    if (!cockpit || typeof cockpit !== 'object') return;
    try {
      q('#version').textContent = cockpit.version;
      document.title = `ZI-E Control Center ${cockpit.version}`;
      q('#top-esp32').textContent = cockpit.controllers[0].state;
      q('#top-stm32').textContent = cockpit.controllers[1].state;
      q('#top-safety').textContent = cockpit.safety.motion_authority;
      q('#firmware-status').innerHTML = Object.entries(cockpit.firmware.esp32)
        .map(([name, state]) => `<div class="metric"><span>${safe(name.replaceAll('_', ' '))}</span><strong>${safe(state)}</strong></div>`)
        .join('');
      q('#development-readiness').innerHTML = Object.entries(cockpit.development_readiness)
        .map(([name, state]) => `<div class="metric"><span>${safe(name.replaceAll('_', ' '))}</span><strong>${safe(state)}</strong></div>`)
        .join('');
      q('#bench-evidence').innerHTML = cockpit.bench_evidence.length
        ? cockpit.bench_evidence.map(item => `<article class="evidence-row">
            <div><b>${safe(item.id)} · ${safe(item.label)}</b><small>${safe(item.evidence_state)} · ${safe(item.functional_state)}</small></div>
            ${chip(item.physical_state)}${chip(item.integration_state)}
          </article>`).join('')
        : '<p class="unavailable-state">EVIDENCE UNAVAILABLE · no physical readiness inferred</p>';
      q('#system-graph').innerHTML = [
        ['HostRuntime', 'Brain / coordinator', cockpit.target.runtime.readiness],
        ['ESP32 Presence', 'Media / network / semantic bridge', cockpit.presence.state],
        ['STM32 Safety', 'Physical rejection authority', cockpit.controllers[1].state],
      ].map(([name, role, state]) => `<article class="system-node"><h3>${safe(name)}</h3><p>${safe(role)}</p>${chip(state)}</article>`).join('');
      const readiness = cockpit.development_readiness;
      const alerts = [
        ['CRITICAL', `FIRST POWER ${readiness.first_integrated_power}`],
        ['BLOCKED', `HW-002 ${readiness.esp32_physical_target}`],
        ['BLOCKED', `STM32 DEVELOPMENT ${readiness.stm32_development}`],
        ['BLOCKED', `PHASE2B2 ${readiness.phase2b2}`],
        ['INFO', `ESP32 GENERIC CROSS BUILD ${readiness.esp32_generic_cross_build} · FLASH ${readiness.esp32_flash}`],
      ];
      q('#global-alerts').innerHTML = alerts.map(([level, value]) =>
        `<div class="alert-item ${level === 'CRITICAL' ? 'critical' : ''}"><b>${safe(level)}</b>${safe(value)}</div>`).join('');
      q('#face-mode').innerHTML = `Mode ${chip(cockpit.face_engine.mode)} · Host link ${chip(cockpit.presence.host_link)}<br><small>${safe(cockpit.face_engine.pack)} · ${safe(cockpit.face_engine.variant)} · MODEL ONLY</small>`;
      q('#face-library').innerHTML = [
        ['Pack', 'Zi-E Core Procedural'], ['Version', '1.0.0'], ['Author', 'Zi-E Project'],
        ['License', 'ZI-E OWNED'], ['Variants', '5'], ['Storage', '8 KiB estimate'],
        ['Validation', 'HOST VALIDATED'], ['Device', 'NOT INSTALLED'],
      ].map(([name, value]) => `<div class="metric"><span>${safe(name)}</span><strong>${safe(value)}</strong></div>`).join('');
      const flow = `<div id="provider-flow" class="provider-flow"><span>MIC ${chip(cockpit.media.audio.capture)}</span><i>→</i><span>ESP32</span><i>→</i><span>STT</span><i>→</i><span>HERMES / LLM</span><i>→</i><span>TTS</span><i>→</i><span>ESP32 SPEAKER</span></div>`;
      q('#provider-flow')?.remove();
      q('#providers').insertAdjacentHTML('afterbegin', flow);
    } catch (error) {
      q('#global-alerts').innerHTML = `<div class="alert-item critical"><b>RENDER UNAVAILABLE</b>${safe(error.message)}</div>`;
    }
  }

  function updateFace() {
    const face = q('#face-canvas');
    if (!face) return;
    const gazeX = Number(q('#face-gaze-x').value) / 12;
    const gazeY = Number(q('#face-gaze-y').value) / 12;
    const speech = Number(q('#face-speech').value);
    const expression = (window.expression || 'neutral').toLowerCase();
    face.style.setProperty('--gx', `${gazeX}px`);
    face.style.setProperty('--gy', `${gazeY}px`);
    face.className = `face-canvas ${speech > 20 ? 'speaking' : ''} ${expression}`;
  }

  ['#face-intensity', '#face-gaze-x', '#face-gaze-y', '#face-speech']
    .forEach(id => q(id)?.addEventListener('input', updateFace));
  const originalRender = window.render;
  if (typeof originalRender === 'function') {
    window.render = (...arguments_) => {
      originalRender(...arguments_);
      enhance(arguments_[1]);
    };
  }
  window.zieEnhance = enhance;
  if (window.latestCockpitStatus) enhance(window.latestCockpitStatus);
  updateFace();
})();
