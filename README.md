# 基于 LLVM-18，使用 New Pass Manager，编写和使用 Pass

# 相关视频链接

[基于 LLVM-18，使用 New Pass Manager，编写和使用 Pass](https://www.bilibili.com/video/BV1Bf3neVEnN/)

# 做这个视频的原因

- 最近在看 AFLGo，涉及到了使用 LLVM 插桩
- 想自己实现插桩，又不想专门找书和课程，总是碰壁，把国内外的文档博客 issue 翻了个遍，也还是有些问题找不到解决方案。所以一个原因就是让我这段时间的搜索工作有点意义，减少其他同学花费在搜索上的时间
- 为还在观望的同学做个介绍，展示用法，避掉几个坑，抛砖引玉

# 预先说明

- 首先本人不是专门搞编译器的，只是看到了这个东西想试一试，很多东西都是一知半解
- 所以本教程里的很多内容都来自网友的回答，仅作参考，这个项目也主要是面向和我一样的小白。当然也欢迎大家的批评指正和建议

# llvm 与 clang

LLVM 原本全称: low level virtual machine，因为作者原本只想写个虚拟机。但后面即使 LLVM 没成为虚拟机，但名称已经传开了。现在官方有如下描述：`The name "LLVM" itself is not an acronym; it is the full name of the project`，即 `LLVM`这个名称本身不是首字母缩略词，而是项目的全名(与全称切割!🤣)。

LLVM 的结构展示如下。经常有人问到 clang 与 gcc 的区别，可以参考这篇文章: [clang 到底是什么？gcc 和 clang 到底有什么区别？](https://blog.csdn.net/qq_33919450/article/details/130911617)<br>
下图也可以看到本内容涉及到的核心：Pass，类似一个个`算子`，可以将传入的 IR 进行分析、转换，并输出新 IR。
![alt text](picture/LLVM架构.png)
![alt text](picture/LLVM的结构.png)

LLVM 的特点：

1. 不同的前端后端使用统一的中间代码 LLVM Intermediate Representation (LLVM IR)。如果需要支持一种新的编程语言，只需实现一个新的前端。如果需要支持一种新的硬件设备，只需实现一个新的后端。扩展性强。
2. 代码模块化设计，易于理解，扩展性强。
3. 优化阶段针对的是统一的 LLVM IR，不论是支持新的编程语言，还是支持新的硬件设备，都不需要对优化阶段做修改。
4. [LLVM](https://github.com/llvm/llvm-project?tab=License-1-ov-file) 使用 Apache 许可证，[GCC](https://gcc.gnu.org/onlinedocs/libstdc++/manual/license.html) 使用 GPL 许可证。相比之下 LLVM 更宽松。

   ![alt text](picture/License.png)

# 为什么使用 Pass

我们希望在不修改源码的情况下，在编译时通过遍历语法树或插入、修改代码等方式，收集程序数据，帮助了解程序的行为，发现潜在的错误或问题，最终增强程序的安全性。又或者进行代码混淆、改变逻辑、优化代码等操作。

# 可以使用 pass 的例子

- 代码分析：获取代码执行路径与覆盖率，
- 代码混淆：`a - b == (a + ~b) + 1`、`a + b == (((a ^ b) + 2 * (a & b)) * 39 + 23) * 151 + 111`，对于这两个等价的例子，将简单操作变为同等逻辑的复杂操作。可参考 [a+b 混淆例子](experiment/code-obfuscation/main.cpp)
- 逻辑修改：将两个浮点数的直接比较，修改为`两者之差在误差范围内即判断为相等`

# 安装

- 仓库地址：[https://github.com/llvm/llvm-project](https://github.com/llvm/llvm-project)
- 使用源码编译：[https://llvm.org/docs/GettingStarted.html#getting-the-source-code-and-building-llvm](https://llvm.org/docs/GettingStarted.html#getting-the-source-code-and-building-llvm)。个人认为只有需要 DEBUG 功能时才需要编译源码
- 使用官方提供的脚本：[https://apt.llvm.org](https://apt.llvm.org/)。注意这种安装方式，似乎所有的可执行文件都会带`-18` 后缀，可以自行添加软链
- 我在编写此教程中用到的是Ubuntu 22.04
- 由于在机器上已经装过了 llvm-14，我在视频中就使用装了 llvm-18 的 docker 镜像进行演示。具体的安装流程参考[Dockerfile](install/Dockerfile)。打包并运行镜像的流程如下。

  ```
  # 打包镜像，时间可能较长
  cd install && sudo docker build -t ubuntu22.04:llvm18 .
  # 启动并进入容器,挂载当前目录到容器内的/app目录，并在退出时删除容器
  cd .. && sudo docker run -it --rm -v ./:/app ubuntu22.04:llvm18
  ```

  - 通过挂载目录的方式使用容器内的 llvm 编译宿主机上的代码。但该种方式不能让 宿主机上的 IDE 识别到 llvm 相关库文件，不方便调试，因此最好还是在主机上安装。

- 测试：可以进入`/usr/bin`下查看是否有 llvm 的一系列工具，也可以输入命令 `llvm-config-18 --version`看是否正确输出版本。

# 官方文档

- [User Guides](https://llvm.org/docs/UserGuides.html)
- [编写旧版 pass](https://llvm.org/docs/WritingAnLLVMPass.html)
- [编写新版 pass](https://llvm.org/docs/WritingAnLLVMNewPMPass.html)
- [编写新版 PassManager](https://llvm.org/docs/NewPassManager.html)
- [API 文档](https://llvm.org/doxygen/index.html)

# clang 初使用

参考 [clang-example 文件夹](experiment/clang-example/README.md)

# 旧版 pass(可选，主要还是用新版)

参考 [old-pass 文件夹](experiment/old-pass/README.md)

# 新版 pass

## 编译时分析代码

参考 [StaticCallCounter 文件夹](experiment/StaticCallCounter/README.md)

## 编译时修改逻辑

参考 [ChangeOperator 文件夹](experiment/ChangeOperator/README.md)

## 插入运行时收集数据的指令

参考 [DynamicCallCounter 文件夹](experiment/DynamicCallCounter/README.md)

## 使用自带的 pass

`opt -print-passes`打印当前可用的 pass。也可在官网查看未及时更新的 pass 列表：[LLVM’s Analysis and Transform Passes](https://llvm.org/docs/Passes.html)

参考 [about-option 文件夹](experiment/about-option/README.md)

## 使用 cmake

虽然网上的教程基本全是用 cmake 构建 pass。但本人不太会写 CMakeLists 的配置，而且也不喜欢一来就放一堆小白看不懂的配置，故本项目不涉及 cmake<br>
可以参考 [Building LLVM with CMake](https://llvm.org/docs/CMake.html) 和 [LLVM：从零开始实现 Function Pass](https://www.less-bug.com/posts/llvm-implement-function-pass-from-scratch/)

# 常见问题

## 主要的命令行里的 option

- -emit-llvm：告诉 clang 要生成 IR
- -shared：告诉 clang 生成 so 共享库文件
- -fpIC
- -load-pass-plugin：opt 加载 pass
- -fpass-plugin：clang 加载 pass
- -passes：clang 加载时指定 pass 名
- `llvm-config --cxxflags`: 尝试用 echo 输出一下，其实就是返回一个链接到 llvm 的字符串给 clang
- -load：似乎是老版本 passmanager 会用这个加载 pass
- -flegacy-pass-manager：clang 加载老版本 passmanager
- -enable-new-pm：好像也是老版本用的
- -print-passes：使用在 opt 上，打印内置的可用 pass
- -c: 生成\*.bc
- -S: 生成\*.ll
- -g：生成调试信息，有些 api 需要获取这些信息
- -mllvm：向 LLVM 后端传递选项。在本项目用于通过命令行向 pass 传参
- -Xclang：向 Clang 前端传递选项
- -disable-O0-optnone：参考下面的 `isRequired()`
- -disable-output: is used to prevent opt from printing the output bitcode file.

## IR 学习

网上资料不少，自行查找

## isRequired()

> [Overview of The Passes](https://github.com/banach-space/llvm-tutor/blob/main/README.md#overview-of-the-passes)

关于在 pass 中还要额外 overide 一个 isRequired() 方法：<br>
Note that clang adds the optnone function attribute if either

- no optimization level is specified, or -O0 is specified.
- If you want to compile at -O0, you need to specify -O0 -Xclang -disable-O0-optnone or define a static isRequired method in your pass. Alternatively, you can specify -O1 or higher. Otherwise the new pass manager will register the pass but your pass will not be executed.

## 关于 PassInfoMixin 和 AnalysisInfoMixin

> [Analysis vs Transformation Pass](https://github.com/banach-space/llvm-tutor/blob/main/README.md#analysis-vs-transformation-pass)

a transformation pass will normally inherit from PassInfoMixin,
an analysis pass will inherit from AnalysisInfoMixin.<br>
This is one of the key characteristics of the New Pass Managers - it makes the split into Analysis and Transformation passes very explicit.

## 返回值

对于 pass 的返回值，旧版是`return 修改了 IR? true:false`，新版是`return 修改了IR? PreservedAnalyses::none():PreservedAnalyses::all()`。<br>
可以参考：[LLVM 学习笔记 - Using the New Pass Manager](https://blog.csdn.net/qq_43688320/article/details/136121107)

# 进一步使用的难点

- 注册 pass 有很多种 API，令人疑惑
- 进行动态插入指令时需要了解 IR 指令和相关 API
- 官网的教程有时更新不及时，有些功能可能需要查看源码了

# 当前还存在的疑问

希望大佬解答

- 在使用内置的[Transform Passes](https://llvm.org/docs/Passes.html#transform-passes)时，IR 并没有经过转换。已发布提问：[How to use built-in passes in LLVM-18 with opt?](https://stackoverflow.com/questions/78644693/how-to-use-built-in-passes-in-llvm-18-with-opt)

# 推荐资料

[helpful LLVM resources ](https://github.com/banach-space/llvm-tutor/blob/main/README.md#references)

# 参考

这里有些是提出解决方案的博客，有些提供了代码参考，有些是更深入的源码剖析<br>
因参考太多，这里可能有疏漏

- [深入浅出让你理解什么是 LLVM](https://www.jianshu.com/p/1367dad95445)
- [使用 LLVM+Clang13 进行代码插桩的简单示例](https://www.bilibili.com/video/BV16V4y1n7Mz/?spm_id_from=333.337.search-card.all.click&vd_source=43b6af819307ca6bdad60477a02d8d1c)
- [clang 到底是什么？gcc 和 clang 到底有什么区别？](https://blog.csdn.net/qq_33919450/article/details/130911617)
- [从零开始的 LLVM18 教程（一）](https://zhuanlan.zhihu.com/p/670338052)
- [How to automatically register and load modern Pass in Clang?](https://stackoverflow.com/questions/54447985/how-to-automatically-register-and-load-modern-pass-in-clang/75999804#75999804)
- [Windows 下优雅使用 LLVMPass](http://www.qfrost.com/posts/llvm/llvmwindows%E4%B8%8B%E4%BC%98%E9%9B%85%E4%BD%BF%E7%94%A8llvmpass/)
- [LLVM PassManager 对 C++程序设计的思考](https://zhuanlan.zhihu.com/p/338837812)
- [How to load pass plugin in new pass manager?](https://stackoverflow.com/questions/76848689/how-to-load-pass-plugin-in-new-pass-manager)
- [LLVM——LLVMHello](https://xia0ji233.pro/2024/01/21/LLVM2/index.html)
- [llvm NewPassManager API 分析及适配方案](https://leadroyal.cn/p/2210/)
- [LLVM’s Analysis and Transform Passes](https://llvm.org/docs/Passes.html)
- [Adding Passes to a Pass Manager](https://llvm.org/docs/NewPassManager.html#adding-passes-to-a-pass-manager)
- [LLVM 中的 pass 及其管理机制](https://zhuanlan.zhihu.com/p/290946850)
- [第 9 章: 使用 PassManager 和 AnalysisManager](https://zhqli.com/post/1665878400)
- [LLVM 入门教程之基本介绍](https://blog.yuuoniy.cn/posts/llvm-1/)
- [LLVM doesn't generate CFG](https://stackoverflow.com/questions/67393329/llvm-doesnt-generate-cfg)
- [官方的`Bye`例子，同时支持新旧版的 pass manager](https://github.com/llvm/llvm-project/blob/main/llvm/examples/Bye/Bye.cpp)
