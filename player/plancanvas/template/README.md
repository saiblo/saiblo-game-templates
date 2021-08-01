# PlayCanvas 样板代码

该样板代码封装了游戏播放器与评测机交互的必要代码，并提供了一些实用功能。

**理想情况下，播放器开发者能够基于该样板代码，快速开发出可在 Saiblo 上部署的游戏，而无需关心与 Saiblo 交互的具体逻辑。**

## 脚本加载顺序

- `Saiblo/interface.js`定义了一些常量和变量，因此应置于（除原生代码接口外的）第一位加载该脚本。
- 若播放器需要原生代码接口，需将`Logic/logicWrapper.js`置于第一位加载。
- 该样板代码本身对其他脚本的加载顺序无严格要求。

## 用法

### `loading.js`

该脚本设置了自定义的开始界面。

它是必要的，因为它处理了在游戏加载完成前就收到网页消息的情形。

> 注意：该脚本不应被预加载。

### Saiblo

该目录下的脚本封装了与 Saiblo 交互的代码。

#### 可使用的常量、变量和函数

|        名称         |        类型        |                          功能                           |
| :-----------------: | :----------------: | :-----------------------------------------------------: |
|       `DEBUG`       |  `const boolean`   | 是否与本地 judger 进行交互；部署到Saiblo前须置为`false` |
| `OFFLINE`、`ONLINE` |   `const number`   |                 表示游戏模式的枚举类型                  |
|     `gameMode`      |      `number`      |                      表示游戏模式                       |
|      `sendMsg`      | `(object) => void` |                    向评测机发送消息                     |

#### 需实现的函数

|      名称       |                 类型                 |                            功能                            |
| :-------------: | :----------------------------------: | :--------------------------------------------------------: |
| `parseOffline`  | `(string, (number) => void) => void` | 处理从评测机收到的回放文件，将总帧数告知网页，并绘制第一帧 |
| `loadNextFrame` |             `() => void`             |       绘制下一帧，网页上点击下一帧按钮时会调用该函数       |
|   `loadFrame`   |          `(number) => void`          |  绘制指定帧，网页上点击上一帧或重新播放按钮时会调用该函数  |
|  `parseOnline`  |          `(string) => void`          |                 处理从评测机收到的在线消息                 |

### Logic（可选）

该目录下的脚本封装了逻辑的部分 C++ 代码，可以实现代码复用。

如果播放器不需要调用逻辑的 C++ 代码，该目录可被删去。

样板代码中的`logic.js`、`logic.wasm`和`logicWrapper.js`为日本麻将听牌接口的封装，可供参考。

如需自己动手封装逻辑代码，请先前往 [Emscripten](https://emscripten.org/) 下载安装`emcc`工具链，并按如下步骤进行操作。

#### 准备C++函数

将需要复用的逻辑函数整理进`logic.cpp`文件中。一个简单的样例如下所示：

```c++
#include <string>

extern "C" {
    const char *reverse_string(const char *src) {
        std::string s{src};
        reverse(s.begin(), s.end());
        auto ret = new char[s.length() + 10];
        strcpy(ret, s.c_str());
        return ret;
    }
}
```

> Emscripten 要求使用C方式链接函数，因此如果需要以字符串的形式传参，不得使用`std::string`，必须使用`char *`。
>
> 如果返回值类型为字符串，必须返回堆空间上的字符数组。Emscripten 会自动进行垃圾回收，因此为了返回堆空间上的字符数组而使用`new`申请的空间不需要`delete`。

#### 编译`.js`和`.wasm`文件

使用`emcc`进行编译，一个示例编译指令为：

```shell
emcc logic.cpp -o logic.js -s EXPORTED_FUNCTIONS='["_reverse_string"]' -s EXPORTED_RUNTIME_METHODS='["cwrap"]'
```

更详细的`emcc`工具链使用方法参见 [Interacting with code](https://emscripten.org/docs/porting/connecting_cpp_and_javascript/Interacting-with-code.html) 。

#### 微调生成的`.js`文件

在生成的`logic.js`前后分别添加如下代码以使其符合 PlayCanvas 接口：

```javascript
var Logic = (function() {
  var _scriptDir = typeof document !== 'undefined' && document.currentScript ? document.currentScript.src : undefined;
  if (typeof __filename !== 'undefined') _scriptDir = _scriptDir || __filename;
  return (
function(Module) {
  Module = Module || {};

    // logic.js

  return Promise.resolve(Module);
}
);
})();
if (typeof exports === 'object' && typeof module === 'object')
      module.exports = Logic;
    else if (typeof define === 'function' && define['amd'])
      define([], function() { return Logic; });
    else if (typeof exports === 'object')
      exports["Logic"] = Logic;
```

#### 新增`logicWrapper.js`

按照 Emscripten 的描述，使用`cwrap`封装函数。

一个示例为：

```javascript
const reverse_string = Logic.cwrap("reverse_string", "string", ["string"]);
```

#### 上传到PlayCanvas工程

注意上传后要为`logic.wasm`设置`Name`为`Logic`，`Glue script`为`logic.js`，并将`native.js`的`Preload`勾选为`false`。

至此，你已可以在播放器代码中调用`reverse_string`函数了。

### Ammo（可选）

该目录存放了物理引擎脚本。

如果播放器不需要物理引擎，该目录可被删去。

注意，射线检测间接依赖物理引擎。

### EntityPicking（可选）

该目录下的脚本能够实现场景中 3D 物体的点击。

**它依赖射线检测，因此需要存在`Ammo`目录，并在游戏设置中配置`Ammo`脚本路径。**

如果播放器不需要处理 3D 物体的点击，该目录可被删去。

将`EntityPicking/pickerRaycast.js`挂载到场景的摄像机上，并为需要处理点击的物体添加碰撞盒。此后，每当一个场景中添加了碰撞盒的3D物体被点击时，该物体就会收到`click`事件。

> 你也可以手动修改`EntityPicking/pickerRaycast.js`来改变点击事件的处理逻辑。

### TextMesh（可选）

PlayCanvas 缺少对3D文字的支持，幸而有社区大佬实现了相关组件。

如果播放器不需要显示 3D 文字，该目录可被删去。

将`TextMesh/text-mesh.js`挂载到场景中的空实体上，即可实现 3D 文字的显示。