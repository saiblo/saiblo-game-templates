import argparse
import os
import pathlib

GAME_URL = 'https://game.cdn.saiblo.net/'
GAME_URL_DEV = 'https://player.dev.saiblo.net/'

# Setup parser

parser = argparse.ArgumentParser(description='Convert files generated by PlayCanvas to make it compatible with Saiblo.')
parser.add_argument('--root', metavar='root-dir', dest='root', type=str,
                    required=True, help='source root of generated files')
parser.add_argument('--name', metavar='game-name', dest='name', type=str,
                    required=True, help='name of the game')
parser.add_argument('--dev', dest='dev', action='store_true', default=False,
                    help='whether to be deployed onto dev server')

args = parser.parse_args()

root_dir = pathlib.Path(args.root)

base_url = f'{GAME_URL_DEV if args.dev else GAME_URL}{args.name}/'

# Convert __settings__.js

with open(root_dir / '__settings__.js') as f:
    settings_js = f.read()

settings_js = settings_js \
    .replace('ASSET_PREFIX = "', f'ASSET_PREFIX = "{base_url}') \
    .replace('SCRIPT_PREFIX = "', f'SCRIPT_PREFIX = "{base_url}') \
    .replace('SCENE_PATH = "', f'SCENE_PATH = "{base_url}') \
    .replace('CONFIG_FILENAME = "', f'CONFIG_FILENAME = "{base_url}')

with open(root_dir / '__settings__.js', 'w') as f:
    f.write(settings_js)

# Convert index.html

with open(root_dir / 'index.html') as f:
    index_html = f.read()

index_html = index_html \
    .replace('__loading__.js', f'{base_url}__loading__.js') \
    .replace('__settings__.js', f'{base_url}__settings__.js') \
    .replace('__start__.js', f'{base_url}__start__.js') \
    .replace('__modules__.js', f'{base_url}__modules__.js') \
    .replace('playcanvas-stable.min.js', f'{base_url}playcanvas-stable.min.js') \
    .replace('styles.css', f'{base_url}styles.css') \
    .replace('<link rel="manifest" href="manifest.json">', '')

with open(root_dir / 'player.html', 'w') as f:
    f.write(index_html)

os.remove(root_dir / 'index.html')

os.remove(root_dir / 'logo.png')

if not os.path.exists(root_dir / 'cover.png'):
    print('WARNING: cover.png does not exist; please manually add a cover image')
