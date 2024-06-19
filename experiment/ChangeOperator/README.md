> 参考自[instrument.cpp](https://github.com/LuoRongLuoRong/llvm_clang_instrument_example/blob/main/instrumentation/instrument.cpp)

# 介绍

这个例子特别涉及到了：如果实现的是 `PreservedAnalyses run(Function &F ...`时怎么注册使 clang 使用 这一情况

# 运行

```shell
clang++ `llvm-config --cxxflags` -fPIC -shared pass.cpp -o pass.so `llvm-config --ldflags`
clang++ -emit-llvm -S main.cpp -o main.ll
# 加载pass修改IR，并导出新的IR
opt -S -load-pass-plugin ./pass.so -passes=change-operator main.ll -o new_main.ll
# 运行IR
lli new_main.ll
# 可以看看没被pass处理的结果
lli main.ll
```

clang 方式

```shell
clang++ -Xclang -fpass-plugin=./pass.so main.cpp -o main.exe
./main.exe
```

# 输出结果

输出`50`而不是`15`
