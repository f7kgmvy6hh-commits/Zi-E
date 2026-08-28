window.ZIE_PLUGINS = Object.freeze([
  { id: 'vision', title: 'Vision', text: 'No camera feed attached.', permission: 'read', events: ['vision.frame'] },
  { id: 'computer', title: 'Computer', text: 'Computer Use status unavailable.', permission: 'read', events: ['computer.status'] },
  { id: 'terminal', title: 'Terminal', text: 'Read-only activity surface.', permission: 'read', events: ['tool.start', 'tool.end'] },
  { id: 'browser', title: 'Browser', text: 'Hermes-mediated browser activity.', permission: 'read', events: ['browser.status'] },
  { id: 'files', title: 'Files', text: 'No unrestricted filesystem endpoint.', permission: 'read', events: ['file.activity'] },
  { id: 'memory', title: 'Memory', text: 'Hermes-owned memory surface.', permission: 'read', events: ['memory.activity'] },
  { id: 'settings-extra', title: 'Settings', text: 'Server-controlled settings surface.', permission: 'read', events: ['settings.changed'] }
]);
