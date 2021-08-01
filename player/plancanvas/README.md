# Saiblo-PlayCanvas 适配器

PlayCanvas 生成的产物无法直接被 Saiblo 使用。播放器开发者可以使用本工具进行一定的适配。

## 用法

```
usage: saiblo-pc.py [-h] --root root-dir --name game-name [--dev]

Convert files generated by PlayCanvas to make it compatible with Saiblo.

optional arguments:
  -h, --help        show this help message and exit
  --root root-dir   source root of generated files
  --name game-name  name of the game
  --dev             whether to be deployed onto dev server
```

## 示例

> 该示例基于 Windows 系统，但其他系统下的操作方式大体类似。

设 PlayCanvas 生成的产物为`Mahjong.zip`，将其解压，设解压后的路径为`D:\Mahjong`。再设 Saiblo 平台上游戏的（英文）名称为`mahjong`，我们想将其部署到`dev.saiblo.net`上进行测试。此时，输入指令

```shell
python .\saiblo-pc.py --root D:\Mahjong --name mahjong --dev
```

即可在`D:\Mahjong`下得到适配后的产物。将其提交给Saiblo网站管理员即可进行部署。

## 附件

在`template`目录下，我们还提供了一份样板 PlayCanvas 代码。它封装了游戏播放器与评测机交互的必要代码，并提供了一些实用功能。更多信息参见`template/README.md`。

## 致未来的维护者

### 为什么需要适配

Saiblo 使用`iframe`的`srcdoc`从 CDN 加载播放器，最初设计时只支持单个`HTML`，不过通过`url`引用其他文件是允许的。然而，该`url`必须为绝对路径（否则 Saiblo 网站会把`url`指向奇怪的地方）。为此，需要使用工具对构建产物中的`url`进行改写。

此外，Saiblo 要求网页播放器文件名为`player.html`，因此要将 PlayCanvas 生成的`index.html`重命名为`player.html`。

以下以部署在`https://player.dev.saiblo.net/`为例。

- 修改`__settings__.js`中的配置常量
  - `ASSET_PREFIX`设置为根目录`"https://player.dev.saiblo.net/mahjong/"`
  - `SCRIPT_PREFIX`设置为根目录`"https://player.dev.saiblo.net/mahjong/"`
  - `SCENE_PATH`设置为`"https://player.dev.saiblo.net/mahjong/xxxxxxx.json"`
  - `CONFIG_FILENAME`设置为`"https://player.dev.saiblo.net/mahjong/config.json"`
- 将`index.html`中的路径引用调整为绝对路径
  - `__loading__.js`修改为`https://player.dev.saiblo.net/mahjong/__loading__.js`
  - `__settings__.js`修改为`https://player.dev.saiblo.net/mahjong/__settings__.js`
  - `__start__.js`修改为`https://player.dev.saiblo.net/mahjong/__start__.js`
  - `__modules__.js`修改为`https://player.dev.saiblo.net/mahjong/__modules__.js`
  - `playcanvas-stable.min.js`修改为`https://player.dev.saiblo.net/mahjong/playcanvas-stable.min.js`
  - `styles.css`修改为`https://player.dev.saiblo.net/mahjong/styles.css`
  - `manifest.json`相关可直接删除
- 将`index.html`重命名为`player.html`

### 维护说明

- `GAME_URL`和`GAME_URL_DEV`分别为Saiblo主站和测试站的游戏静态文件路径。若未来该路径有变，`saiblo-pc.py`中的相关常量值也需进行相应调整。
- 目前对`__settings__.js`和`index.html`中`url`的改写方式为朴素的字符串替换，因此若未来 PlayCanvas 生成产物的方式有变，`saiblo-pc.py`中的有关代码也需进行相应调整。
- Saiblo 约定游戏封面图为`cover.png`，因此该脚本会对`cover.png`不存在的情况进行提醒。