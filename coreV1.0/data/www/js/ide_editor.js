const pathInput = document.getElementById('editor-path');
const contentEl = document.getElementById('editor-content');
const statusEl = document.getElementById('editor-status');
const form = document.getElementById('editor-form');

function setStatus(text) {
  statusEl.textContent = text;
}

async function loadFile() {
  const file = pathInput.value.trim();
  if (!file) return;
  const res = await fetch(`/api/fs/read?file=${encodeURIComponent(file)}`);
  if (!res.ok) throw new Error(await res.text());
  contentEl.value = await res.text();
  setStatus(`Открыт: ${file}`);
}

async function saveFile() {
  const file = pathInput.value.trim();
  if (!file) throw new Error('Укажите путь файла');
  const body = new URLSearchParams({ file, content: contentEl.value });
  const res = await fetch('/api/fs/writeText', { method: 'POST', body });
  if (!res.ok) throw new Error(await res.text());
  setStatus(`Сохранено: ${file}`);
}

document.getElementById('btn-load').addEventListener('click', () => loadFile().catch((e) => setStatus(e.message)));

form.addEventListener('submit', (event) => {
  event.preventDefault();
  saveFile().catch((e) => setStatus(e.message));
});

const initial = new URLSearchParams(location.search).get('file');
if (initial) {
  pathInput.value = initial;
  loadFile().catch((e) => setStatus(e.message));
}
