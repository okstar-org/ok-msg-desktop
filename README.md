<h1 align="center">OkMSG Desktop</h1>

<p align="center">
  <img src="https://img.shields.io/badge/build-passing-brightgreen.svg">
  <img src="https://img.shields.io/badge/platform-Windows%20|%20Linux%20|%20Web-brightgreen.svg">
  <img src="https://img.shields.io/badge/license-MulanPubL%202.0-blue.svg">
  <img src="https://img.shields.io/badge/std-C++20-blue.svg">
  <img src="https://img.shields.io/badge/language-Qt-blue.svg">
</p>

# 🎁 项目介绍

OkMSG是由OkStar(okstar.org)社区开发和维护的注重数据安全与保护的企业通讯协同工具，支持独立私有化部署的集即时消息、语音、视频通话、发送文件、会议等多种功能于一身的开源项目，同时让您的企业更加有效开启协作、有效沟通，控制成本，开拓新业务，并帮助您加速发展业务。

OkMSG的诞生主要解决企业信息化过程中面对的问题：
• 远程协同工具提高工作效率同时,如何保障企业数据安全和隐私、自主可控将成为企业最核心的问题。
• 市面上现有产品同质化严重，市场需要一款有本质化差异的产品。

# 🧭 功能介绍
- 聊天模块
  - 具备单聊、群聊；
  - 支持文字、音视频、文件传输等基本功能；
  - 消息加密（计划中）。
  
- 教室模块 
  - 互动白板、课堂直播、交流互动、在线群聊等功能；

# 🏢 软件架构


# 🖼️ 界面展示


# ⛽ 第三方库

- CMake ([New BSD License](https://github.com/Kitware/CMake/blob/master/Copyright.txt))
- WebRTC ([New BSD License](https://github.com/desktop-app/tg_owt/blob/master/LICENSE))
- OpenSSL 3.0.x ([OpenSSL License](https://www.openssl.org/source/license.html))
- qTox([GPL v3](https://github.com/qTox/qTox/LICENSE))
- gloox ([GPL v3](https://gitee.com/chuanshantech/ok-edu-gloox))
- OpenAL Soft ([LGPL](https://github.com/kcat/openal-soft/blob/master/COPYING))
- FFmpeg ([LGPL](https://www.ffmpeg.org/legal.html))
- Qt 5.15 ([LGPL](http://doc.qt.io/qt-5/lgpl.html))
- zlib ([zlib License](http://www.zlib.net/zlib_license.html))
- Sqlite3 ([Public Domain](https://sqlite.org/copyright.html))
- Sodium([ISC license.](https://github.com/jedisct1/libsodium))
- libexif([GPL v2](https://github.com/libexif/libexif/blob/master/COPYING))
- libqrencode([GPL v2+](https://github.com/fukuchi/libqrencode))

# 🖥️ 支持平台
> 🐧 Linux
- Ubuntu  已支持 文档：[Build On Ubuntu](./docs/BUILD-linux.md "Linux 构建")
- Fedora  计划中

> 🪟 Windows
- Windows10+ 已支持 文档：[Build On Windows](./docs/building/BUILD-win.md "Windows 构建")。

> 🍎 macOS
- 计划中   文档：[Build On macOS](./docs/building/BUILD-macos.md "MacOS 构建")。

# 🧰 编译环境

- ✅ 支持 **GCC On Linux**
- ✅ 支持 **Clang On Linux**
- ✅ 支持 **MSVC On Windows**
- ✅ 支持 **Clang On Windows**
- 📌 支持 **MinGW On Windows** 计划中
- 📌 支持 **Clang On macOS** 计划中

# ⚙️ 构建开发
- C++版本：C++20 
- Qt版本：Qt5.15.x

✔️ 支持**静态Qt编译** (Linux、Windows)。
✔️ 支持**动态Qt编译**（Linux、Windows）；

> 构建本项目需要分如下几步：
1. 准备工具链环境：[Linux](./docs/building/ToolChain-linux.md) | [Windows](./docs/building/ToolChain-win.md) | [macOS](./docs/building/ToolChain-macos.md) 计划中
2. 准备Qt环境：[Linux](./docs/building/Qt-linux.md) | [Windows](./docs/building/Qt-win.md) | [macOS](./docs/building/BUILD-macos.md) 计划中
3. 准备包依赖：[Linux](./docs/building/ThirdPart-linux.md)  | [Windows](./docs/building/ThirdPart-win.md) | [macOS](./docs/building/ThirdPart-macos.md) 计划中
4. 准备IDE开发： [VS Code](./docs/building/IDE-vscode.md) | [VS Studio](./docs/building/IDE-vsstudio.md) | [QtCreator](./docs/building/IDE-qtcreator.md) | [Clion](./docs/building/IDE-clion.md)

# ⚒️ 开发规范

- Git  [Git规范](./docs/spec/git.md)
- Format [代码格式化](./docs/spec/format.md)
- 开发者规约（进行中）

# 🗓️ 版本规则


# 🙏 感谢支持
- 感谢社区成员的鼎力支持等
- [感谢 JetBrains 对本项目的支持（Drive by JetBrains）](https://jb.gg/OpenSourceSupport) <img width="64" src="https://resources.jetbrains.com/storage/products/company/brand/logos/jb_beam.svg?_ga=2.83044246.1221182059.1672752920-1856866598.1665301971&_gl=1*3fzoi7*_ga*MTg1Njg2NjU5OC4xNjY1MzAxOTcx*_ga_9J976DJZ68*MTY3Mjc1MjkyMC40LjEuMTY3Mjc1NDM0Ni4wLjAuMA">

# ❤️ 捐赠方式

> 您的捐款是OkMSG开源项目持续前进的动力，希望该项目可以让任何人从中受益。
> 捐赠列表 https://kdocs.cn/l/cr7rVyXnbxuK

<div>
<img src="./docs/donate/wx.png" width="240"  alt=""/> 
<img src="./docs/donate/zfb.png" width="240"  alt=""/>
</div>

> 欢迎大家 Clone 本项目，捐赠收入将用于对贡献者的奖励。

# ⏰ 任务规划
- 2023年 
  1. 完成项目基本功能 
  2. 开启社区基本建设
  3. 开启社区项目内测
- 2024年
  1. 完成对各系统平台的支持
  2. 登录到各平台面向消费者
  3. 开启商业定制之路

> 【金山文档】 OkMSG任务列表 https://kdocs.cn/l/csib86aYwx0P


# 🏭 社区建设

> 为了OkMSG项目能长期有序有质量发展，诚邀有梦想情怀人士加入，一起建设社区，
> 目前规划需要如下方面人才：

- 开发、问题反馈、 Bug修复
- 文档、翻译、设计、品控
- 支持、推广、投资、项目管理

> 社区模式：敬请期待。。。
- 欢迎加入 [OkStar 社区](http://okstar.org)


# 🧑‍🤝‍🧑 贡献者
> 欢迎加入参与社区建设
- [更多](https://github.com/okstar-org/ok-msg-desktop/contributors?ref=master)

# 💰 赞助商

- 湖南船山信息科技有限公司
- Joy1024（个人）
- 社区支持者们

# 📦 欢迎体验
- 下载地址：https://www.chuanshaninfo.com/download/OkMSG/

# ☎️ 联系我们

> 技术交流群，添加微信

![OkStar公众号](./docs/assets/assistant-OkEDU.jpg "OkStar")

> 微信公众号，关注账号

![OkStar公众号](./docs/assets/gzh-OkEDU.jpg "OkStar")


# ©️ 版权信息

> 本软件已经申请软件著作权和商标。
> 本项目采用双重授权，请按合规正确使用本项目！

1.遵循软件分发协议
  - 国际：**[GPL-2.0](https://opensource.org/license/gpl-2-0/)**
  - 中国：**[木兰公共许可证, 第2版](https://opensource.org/license/mulanpsl-2-0/)**

2.付费得到商业授权（试行）
  - [OkMSG软件合作伙伴合同-个人版](https://www.kdocs.cn/l/cgdtyImooeol)
  - [企业版软件授权，请移步👉 chuanshaninfo.com](https://www.chuanshaninfo.com/)