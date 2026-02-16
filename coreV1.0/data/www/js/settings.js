const form = document.getElementById('settings-form');
const ssidInput = document.getElementById('wifi-ssid');
const passInput = document.getElementById('wifi-pass');
const macInput = document.getElementById('wifi-mac');
const restartButton = document.getElementById('restart-device');
const status = document.getElementById('settings-status');

function setStatus(text, ok = false) {
  status.textContent = text;
  status.classList.toggle('status-online', ok);
  status.classList.toggle('status-offline', !ok);
}

async function loadSettings() {
  try {
    const response = await fetch('/api/settings');
    if (!response.ok) throw new Error(`HTTP ${response.status}`);

    const data = await response.json();
    ssidInput.value = data.ssid || '';
    passInput.value = data.pass || '';
    macInput.value = data.ap_mac || '';
    setStatus('Загружено', true);
  } catch (error) {
    setStatus(`Ошибка: ${error.message}`);
  }
}

async function saveSettings() {
  const body = new URLSearchParams({
    ssid: ssidInput.value.trim(),
    pass: passInput.value,
    mac: macInput.value.trim()
  });

  const response = await fetch('/api/settings', {
    method: 'POST',
    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
    body
  });

  if (!response.ok) {
    const message = await response.text();
    throw new Error(message || `HTTP ${response.status}`);
  }

  const data = await response.json();
  macInput.value = data.ap_mac || macInput.value;
  setStatus('Сохранено', true);
}

form.addEventListener('submit', async (event) => {
  event.preventDefault();
  try {
    await saveSettings();
  } catch (error) {
    setStatus(`Ошибка: ${error.message}`);
  }
});

restartButton.addEventListener('click', async () => {
  try {
    await saveSettings();
    const restartResponse = await fetch('/api/system/restart', { method: 'POST' });
    if (!restartResponse.ok) throw new Error(`Перезагрузка не запущена (${restartResponse.status})`);
    setStatus('Перезагрузка...', true);
  } catch (error) {
    setStatus(`Ошибка: ${error.message}`);
  }
});

loadSettings();
