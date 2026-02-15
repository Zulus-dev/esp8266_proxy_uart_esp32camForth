const terminal = document.getElementById('terminal');
const statusLabel = document.getElementById('status');
const form = document.getElementById('send-form');
const commandInput = document.getElementById('command');
const clearButton = document.getElementById('clear');

if (!terminal || !statusLabel || !form || !commandInput || !clearButton) {
  // Script can be included only on terminal page.
} else {
  let ws;
  let reconnectTimer;

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

    if (!ws || ws.readyState !== WebSocket.OPEN) {
      appendSystemMessage('нет соединения с ESP8266');
      return;
    }

    ws.send(cmd);
    commandInput.value = '';
    commandInput.focus();
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
  connectWS();
}
