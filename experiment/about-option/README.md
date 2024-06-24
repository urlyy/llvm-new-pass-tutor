# 介绍

两个例子：

- 使用 llvm 内置的 pass
- 通过命令行选项动态向 pass 内传值

# 运行

## 导出 cg 和 cfg

参考自 https://stackoverflow.com/questions/67393329/llvm-doesnt-generate-cfg

```shell
clang -O0 -emit-llvm -Xclang -disable-O0-optnone -S main.c -o main.ll
opt main.ll -passes=dot-cfg
opt main.ll -passes=dot-callgraph
```

## 向 pass 中传入参数

通过环境变量传值有很多麻烦，官网也讲了使用这个命令行传值与解析的众多优点

> 参考自
>
> - [LLVM 核心库之 CommandLine 2.0 Library](https://blog.csdn.net/qq_42909352/article/details/128331571?ops_request_misc=%257B%2522request%255Fid%2522%253A%2522171888142516800185843802%2522%252C%2522scm%2522%253A%252220140713.130102334.pc%255Fall.%2522%257D&request_id=171888142516800185843802&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~all~first_rank_ecpm_v1~rank_v31_ecpm-5-128331571-null-null.142^v100^pc_search_result_base3&utm_term=cl%3A%3Aopt%3Cstd%3A%3Astring%3E&spm=1018.2226.3001.4187)
> - 官网的文档：[https://www.llvm.org/docs/CommandLine.html](https://www.llvm.org/docs/CommandLine.html)
> - [Running the plugin](https://clang.llvm.org/docs/ClangPlugins.html#running-the-plugin)
> - [How to pass addition arguments to llvm pass registered by new pass manager](https://github.com/llvm/llvm-project/issues/56137#issuecomment-1549565099)
> - [How to define and read CLI arguments for an LLVM Pass with the new Pass Manager?](https://stackoverflow.com/a/71751364/23686563)

```shell
clang++ `llvm-config --cxxflags` -fPIC -shared pass.cpp -o pass.so `llvm-config --ldflags`
clang -emit-llvm -S main.c -o main.ll
# 注意这里添加了-fplugin，虽然我也不知道为什么
clang++ -S -disable-output -fplugin=./pass.so -fpass-plugin=./pass.so -mllvm -wave-goodbye -mllvm -urlyy=urlyy.c main.ll


# 使用opt时传参
opt -load ./pass.so -load-pass-plugin=./pass.so -passes="about-option" -wave-goodbye -urlyy=urlyy.c ./main.ll
```

输出如下

```
loop_print
urlyy.c
have waved
main
urlyy.c
have waved
```

## 打印此次使用到的 pass

```
opt -load ./pass.so -load-pass-plugin=./pass.so -passes="about-option" -wave-goodbye -urlyy=urlyy.c ./main.ll --print-pipeline-passes
```

## 疑惑，这个 pass 没有生效

本人已经在 stackoverflow 上发布了提问：[How to use built-in passes in LLVM-18 with opt?](https://stackoverflow.com/questions/78644693/how-to-use-built-in-passes-in-llvm-18-with-opt)，期望大佬解答

```shell
# 编译为ll
clang -O0 -emit-llvm -Xclang -disable-O0-optnone -S main.c -o main.ll
# 使用pass修改ll
opt -passes=indvars main.ll -S -o new_main.ll
# 直接获得o2编译成的
clang -O2 -emit-llvm -Xclang -disable-O0-optnone -S main.c -o o2-main.ll
```
