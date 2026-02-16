const terminal = document.getElementById('terminal');
const statusLabel = document.getElementById('status');
const form = document.getElementById('send-form');
const commandInput = document.getElementById('command');
const clearButton = document.getElementById('clear');
const macroForm = document.getElementById('macro-form');
const macroName = document.getElementById('macro-name');
const macroCommand = document.getElementById('macro-command');
const macroButtons = document.getElementById('macro-buttons');

if (!terminal || !statusLabel || !form || !commandInput || !clearButton || !macroForm || !macroName || !macroCommand || !macroButtons) {
  // Script can be included only on terminal page.
} else {
  let ws;
  let reconnectTimer;
  let macros = [];

  function appendTerminal(text) {
    terminal.textContent += text;
    terminal.scrollTop = terminal.scrollHeight;
  }

  function appendSystemMessage(text) {
    appendTerminal(`\n[${text}]\n`);
  }

  function setStatus(connected) {
    statusLabel.textContent = connected ? 'WS: подключено' : 'WS: отключено';
    statusLabel.classList.toggle('status-online', connected);
    statusLabel.classList.toggle('status-offline', !connected);
  }

  function sendCommand(cmd) {
    if (!cmd || !cmd.trim()) return;

    if (!ws || ws.readyState !== WebSocket.OPEN) {
      appendSystemMessage('нет соединения с ESP8266');
      return;
    }

    ws.send(cmd);
  }

  async function saveMacros() {
    const body = new URLSearchParams({
      file: '/www/terminal_macros.json',
      content: JSON.stringify(macros, null, 2)
    });

    const response = await fetch('/api/fs/writeText', {
      method: 'POST',
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
      body
    });

    if (!response.ok) {
      throw new Error(`Не удалось сохранить макросы: ${response.status}`);
    }
  }

  function renderMacros() {
    macroButtons.innerHTML = '';

    if (!macros.length) {
      const empty = document.createElement('p');
      empty.className = 'hint';
      empty.textContent = 'Пока нет кнопок. Добавьте первую команду.';
      macroButtons.appendChild(empty);
      return;
    }

    macros.forEach((item, index) => {
      const row = document.createElement('div');
      row.className = 'macro-item';

      const runButton = document.createElement('button');
      runButton.type = 'button';
      runButton.textContent = item.name;
      runButton.title = item.command;
      runButton.addEventListener('click', () => sendCommand(item.command));

      const removeButton = document.createElement('button');
      removeButton.type = 'button';
      removeButton.className = 'ghost danger';
      removeButton.textContent = '✕';
      removeButton.title = 'Удалить кнопку';
      removeButton.addEventListener('click', async () => {
        macros.splice(index, 1);
        renderMacros();
        try {
          await saveMacros();
        } catch (error) {
          appendSystemMessage(error.message);
        }
      });

      row.appendChild(runButton);
      row.appendChild(removeButton);
      macroButtons.appendChild(row);
    });
  }

  async function loadMacros() {
    try {
      const response = await fetch('/api/fs/read?file=/www/terminal_macros.json');
      if (response.status === 404) {
        macros = [];
        renderMacros();
        return;
      }

      if (!response.ok) {
        throw new Error(`Ошибка чтения макросов: ${response.status}`);
      }

      const loaded = await response.json();
      macros = Array.isArray(loaded)
        ? loaded.filter((item) => item && typeof item.name === 'string' && typeof item.command === 'string')
        : [];

      renderMacros();
    } catch (error) {
      macros = [];
      renderMacros();
      appendSystemMessage(`не удалось загрузить макросы (${error.message})`);
    }
  }

  function connectWS() {
    const protocol = location.protocol === 'https:' ? 'wss' : 'ws';
    const socketUrl = `${protocol}://${location.hostname}:81/`;

    ws = new WebSocket(socketUrl);

    ws.addEventListener('open', () => {
      setStatus(true);
      appendSystemMessage('подключено');
    });

    ws.addEventListener('message', (event) => {
      appendTerminal(String(event.data));
    });

    ws.addEventListener('close', () => {
      setStatus(false);
      appendSystemMessage('соединение потеряно, переподключение через 2с');
      clearTimeout(reconnectTimer);
      reconnectTimer = setTimeout(connectWS, 2000);
    });

    ws.addEventListener('error', () => {
      ws.close();
    });
  }

  form.addEventListener('submit', (event) => {
    event.preventDefault();

    const cmd = commandInput.value;
    if (!cmd.trim()) return;

    sendCommand(cmd);
    commandInput.value = '';
    commandInput.focus();
  });

  macroForm.addEventListener('submit', async (event) => {
    event.preventDefault();

    const name = macroName.value.trim();
    const command = macroCommand.value.trim();

    if (!name || !command) return;

    macros.push({ name, command });
    renderMacros();

    try {
      await saveMacros();
      macroForm.reset();
      macroName.focus();
    } catch (error) {
      appendSystemMessage(error.message);
    }
  });

  commandInput.addEventListener('keydown', (event) => {
    if (event.key === 'Enter' && !event.shiftKey) {
      event.preventDefault();
      form.requestSubmit();
    }
  });

  clearButton.addEventListener('click', () => {
    terminal.textContent = '';
  });

  setStatus(false);
  renderMacros();
  loadMacros();
  connectWS();
}
