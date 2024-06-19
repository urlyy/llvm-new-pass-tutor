> 参考自 [StaticCallCounter.cpp](https://github.com/banach-space/llvm-tutor/blob/main/lib/StaticCallCounter.cpp)

# 介绍

这个例子展示了在不修改 IR 的情况下，以一种类似遍历语法树的方式，静态获取各个函数被调用次数。

# 运行

构建 pass，`.so` for Linux, `.dylib` for MacOS

```shell
clang++ `llvm-config --cxxflags` -fPIC -shared pass.cpp -o pass.so `llvm-config --ldflags`
```

分析 bc 形式的 IR

```shell

clang -emit-llvm -c main.c -o main.bc
opt -load-pass-plugin ./pass.so -passes=static-call-counter -disable-output main.bc
```

分析 ll 同理

```shell
clang -emit-llvm -S main.c -o main.ll
opt -load-pass-plugin=./pass.so -passes="static-call-counter" main.ll
```

除了 opt，还可以使用 clang 加载运行 pass。虽然产生了 main.exe 但是我们不需要使用。

```shell
clang -Xclang -fpass-plugin=./pass.so main.c -o main.exe
./main.exe
```

# 输出结果

可以看到 for 循环并没有增加`foo`函数的调用次数。函数在文本上出现了几次调用就是几次。

```
foo
bar
foo
bar
fez
printf
foo
=================================================
LLVM-TUTOR: static analysis results
=================================================
NAME                 #N DIRECT CALLS
-------------------------------------------------
bar                  2
fez                  1
foo                  3
printf               1
```
