import argparse
import os
import pathlib
import re

GAME_URL = 'https://cdn.jsdelivr.net/gh/saiblo/saiblo-game-cdn@latest/'
GAME_URL_DEV = 'https://player.dev.saiblo.net/'

INJECTED_CODE = """
<script>
  function debounce(f, msecs) {
    let timer = null;
    return function(...args) {
      if (timer === null) {
        timer = setTimeout(() => { f(...args); timer = null; }, msecs);
      }
    }
  }
  window.addEventListener('message', (ev) => {
    const msg = ev.data;
    if (msg.message === 'init_replay_player') {
      function origResize(h) {
        ev.source.postMessage({
          message: "resized",
          height: h - 30,
        }, ev.origin);
      }
      if (!window.saiblo) window.saiblo = {};
      window.saiblo.replay = msg.replay_data;
      window.saiblo.confirmReplay = function (numberOfFrames) {
        const initMsg = { message: 'init_successfully', number_of_frames: numberOfFrames };
        ev.source.postMessage(initMsg, ev.origin);
      };
      window.saiblo.resize = debounce(origResize, 100);
      window.saiblo.callback = {};
    } else if (msg.message === 'load_frame') {
      if (window.saiblo.callback.loadFrame) {
        window.saiblo.callback.loadFrame(msg["index"]);
      }
    } else if (msg.message === 'load_next_frame' || msg.message === 'next_frame') {
      if (window.saiblo.callback.nextFrame) {
        window.saiblo.callback.nextFrame();
      }
    } else if (msg.message === "load_players") {
      if (!window.saiblo) window.saiblo = {};
      window.saiblo.players = msg.players;
      window.saiblo.callback.loadPlayers(msg.players);
    } else if (msg.message === "init_player_player") {
      if (!window.saiblo) window.saiblo = {};
      window.saiblo.token = msg.token;
      window.saiblo.resize = debounce(origResize, 100);
      window.saiblo.callback = {};
    }
  }, false)
  window.onload = () => {
    const div = window.document.getElementById("GameDiv");
    let timeout = null;
    window.onresize = () => {
      if (timeout === null) {
        timeout = setTimeout(() => {
          div.style.setProperty("height", (window.innerHeight - 20) + "px");
          div.style.setProperty("width", (window.innerWidth - 20) + "px");
          window.dispatchEvent(new Event("resize"));
          timeout = null;
        }, 100);
      }
    }
  }
  console.log('Saiblo compatibility code injected.')
</script>
"""

# Setup parser

parser = argparse.ArgumentParser(description='Convert files generated by Cocos to make it compatible with Saiblo.')
parser.add_argument('--root', metavar='root-dir', dest='root', type=str,
                    required=True, help='source root of generated files')
parser.add_argument('--name', metavar='game-name', dest='name', type=str,
                    required=True, help='name of the game')
parser.add_argument('--dev', dest='dev', action='store_true', default=False,
                    help='whether to be deployed onto dev server')

args = parser.parse_args()

root_dir = pathlib.Path(args.root)

base_url = f'{GAME_URL_DEV if args.dev else GAME_URL}{args.name}/'

# Convert index.js

with open(root_dir / 'index.js') as f:
    index_js = f.read()

index_js = index_js \
    .replace('./application.js', f'{base_url}application.js')

with open(root_dir / 'index.js', 'w') as f:
    f.write(index_js)

# Convert application.js

with open(root_dir / 'application.js') as f:
    application_js = f.read()

application_js = application_js \
    .replace('loadBundle(name,', f'loadBundle("{base_url}assets/" + name,') \
    .replace('src/settings.json', f'{base_url}src/settings.json')

with open(root_dir / 'application.js', 'w') as f:
    f.write(application_js)

# Convert index.html

with open(root_dir / 'index.html') as f:
    index_html = f.read()

index_html = index_html \
    .replace('style.css', f'{base_url}style.css') \
    .replace('favicon.ico', f'{base_url}favicon.ico') \
    .replace('src/polyfills.bundle.js', f'{base_url}src/polyfills.bundle.js') \
    .replace('src/system.bundle.js', f'{base_url}src/system.bundle.js') \
    .replace('src/import-map.json', f'{base_url}src/import-map.json') \
    .replace('./index.js', f'{base_url}index.js') \
    .replace('<body>', f'<body>{INJECTED_CODE}')

index_html = re.sub(r'<h1 class="header">(?:.|\n)*</h1>', "", index_html)

index_html = re.sub(r'<p class="footer">(?:.|\n)*</p>', "", index_html)

with open(root_dir / 'player.html', 'w') as f:
    f.write(index_html)

os.remove(root_dir / 'index.html')

if not os.path.exists(root_dir / 'cover.png'):
    print('WARNING: cover.png does not exist; please manually add a cover image')
