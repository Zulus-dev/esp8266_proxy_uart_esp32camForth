const panes = {
  left: { dir: '/', selected: null, listEl: document.getElementById('list-left'), pathEl: document.getElementById('path-left') },
  right: { dir: '/', selected: null, listEl: document.getElementById('list-right'), pathEl: document.getElementById('path-right') },
};

let activePane = 'left';
const statusEl = document.getElementById('fm-status');

function setStatus(text) {
  statusEl.textContent = text;
}

function parentDir(path) {
  if (!path || path === '/') return '/';
  const parts = path.split('/').filter(Boolean);
  parts.pop();
  return '/' + parts.join('/');
}

function joinPath(dir, name) {
  const base = dir === '/' ? '' : dir;
  return `${base}/${name}`.replace(/\/+/g, '/');
}

function normalizeEntryPath(currentDir, apiName) {
  if (!apiName) return currentDir;
  if (apiName.startsWith('/')) return apiName;
  return joinPath(currentDir, apiName);
}

function basename(path) {
  const parts = String(path || '').split('/').filter(Boolean);
  return parts.length ? parts[parts.length - 1] : '/';
}

async function apiPost(url, data) {
  const body = new URLSearchParams(data);
  const res = await fetch(url, { method: 'POST', body });
  if (!res.ok) throw new Error(await res.text());
  return res.text();
}

function renderPane(name, items) {
  const pane = panes[name];
  pane.pathEl.textContent = pane.dir;
  pane.listEl.textContent = '';

  if (!Array.isArray(items)) {
    setStatus('Ошибка списка: неверный формат ответа API');
    items = [];
  }

  const up = document.createElement('li');
  up.className = 'fm-item fm-up';
  up.textContent = '..';
  up.dataset.type = 'dir';
  up.dataset.path = parentDir(pane.dir);
  pane.listEl.appendChild(up);

  items.sort((a, b) => {
    if (a.isDir !== b.isDir) return a.isDir ? -1 : 1;
    return String(a.name).localeCompare(String(b.name));
  });

  items.forEach((item) => {
    const li = document.createElement('li');
    li.className = 'fm-item';
    const fullPath = normalizeEntryPath(pane.dir, item.name);
    li.dataset.type = item.isDir ? 'dir' : 'file';
    li.dataset.path = fullPath;
    const shortName = basename(fullPath);
    li.textContent = `${item.isDir ? '📁' : '📄'} ${shortName}${item.isDir ? '' : ` (${item.size}B)`}`;
    pane.listEl.appendChild(li);
  });
}

async function loadPane(name) {
  const pane = panes[name];
  const res = await fetch(`/api/fs/list?dir=${encodeURIComponent(pane.dir)}`);
  const data = await res.json();
  if (!res.ok) throw new Error(data?.error || `HTTP ${res.status}`);
  renderPane(name, data);
}

function selectItem(name, li) {
  activePane = name;
  panes[name].selected = {
    path: li.dataset.path,
    type: li.dataset.type,
  };

  document.querySelectorAll('.fm-item.active').forEach((el) => el.classList.remove('active'));
  li.classList.add('active');
}

async function openSelection() {
  const pane = panes[activePane];
  if (!pane.selected) return;

  if (pane.selected.type === 'dir') {
    pane.dir = pane.selected.path;
    pane.selected = null;
    await loadPane(activePane);
    return;
  }

  location.href = `/www/ide_editor.html?file=${encodeURIComponent(pane.selected.path)}`;
}

function bindPane(name) {
  panes[name].listEl.addEventListener('click', (event) => {
    const li = event.target.closest('.fm-item');
    if (!li) return;
    selectItem(name, li);
  });

  panes[name].listEl.addEventListener('dblclick', async (event) => {
    const li = event.target.closest('.fm-item');
    if (!li) return;
    selectItem(name, li);
    await openSelection();
  });
}

async function renameSelection() {
  const pane = panes[activePane];
  if (!pane.selected) return;
  const src = pane.selected.path;
  const currentName = basename(src);
  const nextName = prompt('Новое имя', currentName);
  if (!nextName) return;
  const dst = joinPath(parentDir(src), nextName);
  await apiPost('/api/fs/move', { src, dst });
  await loadPane(activePane);
}

async function deleteSelection() {
  const pane = panes[activePane];
  if (!pane.selected) return;
  if (!confirm(`Удалить ${pane.selected.path}?`)) return;

  await apiPost('/api/fs/delete', {
    file: pane.selected.path,
    dir: pane.selected.type === 'dir' ? '1' : '0',
  });

  pane.selected = null;
  await loadPane(activePane);
}

async function copyToOtherPane() {
  const srcPane = panes[activePane];
  const dstPane = panes[activePane === 'left' ? 'right' : 'left'];
  if (!srcPane.selected) return;
  if (srcPane.selected.type === 'dir') {
    setStatus('Копирование папок не поддержано (только файлы)');
    return;
  }

  const fileName = basename(srcPane.selected.path);
  const dst = joinPath(dstPane.dir, fileName);
  await apiPost('/api/fs/copy', { src: srcPane.selected.path, dst });
  await loadPane(activePane);
  await loadPane(activePane === 'left' ? 'right' : 'left');
}

async function createDir() {
  const pane = panes[activePane];
  const name = prompt('Имя новой папки');
  if (!name) return;
  const dir = joinPath(pane.dir, name);
  await apiPost('/api/fs/mkdir', { dir });
  await loadPane(activePane);
}

async function refreshBoth() {
  await loadPane('left');
  await loadPane('right');
}

bindPane('left');
bindPane('right');

document.getElementById('btn-open').addEventListener('click', () => openSelection().catch((e) => setStatus(e.message)));
document.getElementById('btn-rename').addEventListener('click', () => renameSelection().catch((e) => setStatus(e.message)));
document.getElementById('btn-copy').addEventListener('click', () => copyToOtherPane().catch((e) => setStatus(e.message)));
document.getElementById('btn-delete').addEventListener('click', () => deleteSelection().catch((e) => setStatus(e.message)));
document.getElementById('btn-mkdir').addEventListener('click', () => createDir().catch((e) => setStatus(e.message)));
document.getElementById('btn-refresh').addEventListener('click', () => refreshBoth().catch((e) => setStatus(e.message)));

refreshBoth().catch((e) => setStatus(e.message));
