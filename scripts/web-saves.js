Module.preRun ??= [];
Module.preRun.push(() => {
  // DesktopStore writes under $HOME/.modus. Restore it before main reads saves.
  const directory = "/home/web_user/.modus";
  const dependency = "creative-engine-save-storage";
  FS.mkdirTree(directory);
  addRunDependency(dependency);
  let mounted = false;

  function finish(error) {
    if (error) {
      if (mounted) FS.unmount(directory);
      console.warn("Creative Engine save storage unavailable. Saves will last until reload.", error);
    }
    removeRunDependency(dependency);
  }

  try {
    FS.mount(IDBFS, { autoPersist: true }, directory);
    mounted = true;
    FS.syncfs(true, finish);
  } catch (error) {
    finish(error);
  }
});
