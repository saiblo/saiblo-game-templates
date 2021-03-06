# AI SDK 开发指导手册

AI SDK 的开发思路与游戏逻辑的有所不同。这一差异的根源在于，**游戏逻辑的通信协议和基本框架是由平台规定好的**（参见：[游戏开发者手册](https://docs.saiblo.net/developer/developer.html#_3)），平台方可以提供实现好的抽象基类，由游戏开发方补全派生类，实现自己的游戏逻辑；然而，**AI 与逻辑的通信协议因不同的游戏而异，需要由游戏开发方自行约定**，因此**游戏开发方需要自己实现一个易用的基类**，由选手实现其派生类，以实现自己的 AI 策略。

|            |          游戏逻辑          |             AI             |
| :--------: | :------------------------: | :------------------------: |
|  通信协议  |         由平台规定         |     游戏开发方自行约定     |
|    基类    |       平台方已实现好       |       游戏开发方实现       |
|  基类作用  | 封装通信协议，提供流程框架 | 封装通信协议，提供易用方法 |
|   派生类   |       游戏开发方实现       |          选手实现          |
| 派生类作用 |        实现游戏逻辑        |        实现 AI 策略        |

对于 AI 来说，这样一个由游戏开发方实现的基类，便称作 AI SDK。（相应地，由平台方实现好的逻辑基类也可以称作逻辑 SDK，只是我们不采用这一称呼。）

该模板以示例代码的方式给出 AI SDK 的参考范例，以“石头剪刀布”为例，提供 C++ 与 Python 两种语言的 AI SDK。游戏开发方实现自己的 AI SDK 时，可参考模仿该范例。

游戏开发者开发 AI SDK 时有着较高的自由度，**只有两个方法是固定的**：用于向逻辑发送消息的`void sendMsg(const std::string &msg)`(C++)、`__send_msg(self, msg: str)`(Python) 以及用于从逻辑接收消息的`std::string readMsg()`(C++)、`__read_msg() -> str`(Python)。

AI SDK 暴露给选手使用的接口应当尽可能易用。

## AI SDK 文档示例：石头剪刀布

正如平台方在逻辑的样例模板中提供了逻辑框架的使用文档，游戏开发方也应在下发给选手的 AI SDK 中给出 AI SDK 的说明文档。以下是一份基于 C++ 版 AI SDK 的文档示例，游戏开发方也可以参考这份文档对 AI SDK 样例进行理解。

### SDK 文件结构

在下发的 AI SDK 下，你可以看到这样的文件结构：

```
.
├── jsoncpp
│   ├── json
│   │   ├── json-forwards.h
│   │   └── json.h
│   └── jsoncpp.cpp
├── sdk
│   └── Client.hpp
├── CMakeLists.txt
├── MyAI.hpp
└── main.cpp
```

其中，在`sdk`文件夹中的文件是我们给你提供的一套 C++ SDK，包含了与 judger 的通信、按协议解析与发送对手与玩家的操作、记录对手与玩家的操作历史以及手势大小的比较函数等常用函数。

你需要完善`MyAI.hpp`以实现你的策略。`MyAI.hpp`中已经有一个非常简单的策略实现了，即“对手上一回合出什么，我这一回合就跟着出什么”。你需要修改这一实现，以提升你的 AI 的智能。~~（然而“石头剪刀布”能有什么智能hhh）~~

### 选手填写函数

如果选手想要实现自己的 AI，应该填写`MyAI.hpp`中的`strategy`函数。

#### 功能

已知上一回合对手的手势，作出决策，给出这一回合自己的手势。

#### 原型

```cpp
std::string strategy(const std::string &enemyChoice)
```

#### 参数

接收字符串表示的上回合对手的选择。如果是第一回合，接收到的参数将会是一个空字符串。

#### 返回值

返回字符串表示的本回合自己的选择。

#### 约定

石头、剪刀、布分别用 rock、scissors 和 paper 指代。

### 可用的成员变量

#### `std::vector<std::string> myHistory`

我方给出的手势历史。

#### `std::vector<std::string> enemyHistory`

对手给出的手势历史。可以用`enemyHistory[enemyHistory.size() - 1]`获取对手上一回合给出的手势。

### 可用的成员方法

#### `bool greater(const std::string &l, const std::string &r)`

若`l`表示的手势比`r`表示的手势大，则返回`true`，否则返回`false`。

### 如何调试

你可以使用输出调试法进行调试。在 C++ 版 SDK 中，你可以调用`DEBUG("message")`进行调试；在 Python 版 SDK 中，你可以调用`self._debug('message')`进行调试。