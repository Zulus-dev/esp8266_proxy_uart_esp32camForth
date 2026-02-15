(function () {
  const targets = Array.from(document.querySelectorAll('[data-system-status]'));
  if (!targets.length) return;

  const fmtBytes = (value) => {
    const n = Number(value) || 0;
    if (n < 1024) return `${n} B`;
    if (n < 1024 * 1024) return `${(n / 1024).toFixed(1)} KB`;
    return `${(n / (1024 * 1024)).toFixed(2)} MB`;
  };

  function draw(data) {
    const text = [
      `Uptime: ${data.uptime ?? '-'} s`,
      `Heap: ${fmtBytes(data.heap?.free)} | frag ${data.heap?.frag ?? '-'}% | max ${fmtBytes(data.heap?.maxBlock)}`,
      `FS: ${fmtBytes(data.fs?.used)} / ${fmtBytes(data.fs?.total)} (free ${fmtBytes(data.fs?.free)})`,
      `WiFi: ${data.wifi?.mode ?? '-'} | ${data.wifi?.status ?? '-'} | AP clients ${data.wifi?.ap_clients ?? 0} | RSSI ${data.wifi?.rssi ?? '-'}`,
      `AP: ${data.wifi?.ap_ip ?? '-'} | STA: ${data.wifi?.sta_ip ?? '-'}`,
      `CPU: ${data.chip?.cpu_mhz ?? '-'} MHz | Chip: ${data.chip?.id ?? '-'} | Reset: ${data.chip?.reset_reason ?? '-'}`,
      `Flash: sketch ${fmtBytes(data.flash?.sketch_size)} | free ${fmtBytes(data.flash?.free_space)} | real ${fmtBytes(data.flash?.real_size)}`,
    ].join('\n');

    for (const el of targets) el.textContent = text;
  }

  function drawError() {
    for (const el of targets) {
      el.textContent = 'System status unavailable';
    }
  }

  async function refresh() {
    try {
      const res = await fetch('/api/system/status', { cache: 'no-store' });
      if (!res.ok) throw new Error(`HTTP ${res.status}`);
      const data = await res.json();
      draw(data);
    } catch (err) {
      drawError();
    }
  }

  refresh();
  setInterval(refresh, 2000);
})();
