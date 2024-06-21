> 参考自 [DynamicCallCounter.cpp](https://github.com/banach-space/llvm-tutor/blob/main/lib/DynamicCallCounter.cpp)

# 介绍

这个例子中完全将统计函数调用次数的行为交给运行时，通过在 IR 中为每个函数设置计数器变量，并在函数体入口处添加 `对应函数计数器+1` 的指令，在执行程序的过程中动态获取到函数真实的被调用次数。

# 运行

构建 pass

```shell
clang++ `llvm-config --cxxflags` -fPIC -shared pass.cpp -o pass.so `llvm-config --ldflags`
```

编译、修改并运行 IR

```shell
clang -emit-llvm -S main.c -o main.ll
opt -S -load-pass-plugin=./pass.so -passes="dynamic-cc" main.ll -o main.ll
cat main.ll
lli ./main.ll
```

clang 方式

```shell
clang -Xclang -fpass-plugin=./pass.so main.c -o main.exe
./main.exe
```

# 输出结果

可以看到 for 循环确实增加了 foo 函数的调用次数

```
=================================================
LLVM-TUTOR: dynamic analysis results
=================================================
NAME                 #N DIRECT CALLS
-------------------------------------------------
bar                  2
fez                  1
foo                  13
main                 1
```
