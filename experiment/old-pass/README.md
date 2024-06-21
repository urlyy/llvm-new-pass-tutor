# 介绍

这个例子展示了在未使用 CMakeLists.txt 和 Makefile 的情况下，使用 pass 向 IR 中插入函数。

> 注意这两个例子中，使用的是旧版的 `PassManagerBuilder.h`，其在 llvm-14 中还存在，但在 llvm-18 中已移除。所以本例子仅供参考，不要求一定要跑。主要还是用新版 pass

# 使用 clang 加载并使用插件

## llvm-14 clang 案例

参考自：[LLVM-Pass-静态插桩方式](https://blogs.maikebuke.com/2023/02/18/03-LLVM-Pass-%E9%9D%99%E6%80%81%E6%8F%92%E6%A1%A9%E6%96%B9%E5%BC%8F/)

```shell
# 构建 LLVM Pass。注意`llvm-config --cxxflags`是在执行这段命令，链接llvm并展开一些标志。
# -shared生成shared object
# -fPIC用于生成位置无关代码。即生成的机器代码不依赖于它在内存中的具体地址，这对于创建共享库非常重要
clang++ `llvm-config --cxxflags` -fPIC -shared pass-14-clang.cpp -o Pass14.so `llvm-config --ldflags`
# 使用 Pass 处理 main.c 对应的IR
# -Xclang将 `-load ./LLVMHello.so`命令传递给clang，先编译为目标文件
clang++ -flegacy-pass-manager -Xclang -load -Xclang ./Pass14.so -c main.c -o main.o
# 由于引入了一个自定义函数，这个也需要编译为目标文件
clang -c custom.c -o custom.o
# 链接，生成最终的可执行文件
clang main.o custom.o -o main.exe
# 运行
./main.exe
```

打印了下方内容，可以发现 main.c 的两个 printf 之间有两个我们自定义函数的输出。而且显示出了一个 NULL 变量在运行时被分配内存后的地址。

```
LLVM Test posix_memalign
0x7fffb320ff68 -- (nil) -- 16 -- 1024
0x7fffb320ff68 -- 0x5875e697b6b0 -- 16 -- 1024
====== Finish ======
```

## llvm-14 opt 案例

参考自：https://releases.llvm.org/3.2/docs/WritingAnLLVMPass.html

```shell
clang++ `llvm-config --cxxflags` -fPIC -shared pass-14-opt.cpp -o Pass14.so `llvm-config --ldflags`
# 这里查看自己写的pass是否已经被注册了
# 注意这里一定要带./，不能直接就是文件名！！！！！
opt -load ./Pass14.so -help | grep hello
clang -emit-llvm main.c -c -o main.bc
# 根据这个讨论里的第二个发言 https://groups.google.com/g/llvm-dev/c/kQYV9dCAfSg，添加-enable-new-pm=0
# warning不用管
opt -enable-new-pm=0 -load ./Pass14.so --hello main.bc
```

输出

```
Hello: sum ,arg_size: 4
Hello: main ,arg_size: 2
```
