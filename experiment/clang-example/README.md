> 参考自：[Example with clang](https://llvm.org/docs/GettingStarted.html#example-with-clang)

c 语言使用 clang，cpp 使用 clang++，我这里全部都用 clang++了

# 三种编译方式

## 使用 clang++ 直接编译 cpp 为可执行文件

```shell
clang++ clang-test.cpp -o hello
./hello
```

## 先编译为二进制，再运行 bc

```shell
clang++ -O3 -emit-llvm clang-test.cpp -c -o hello.bc
lli hello.bc
```

## 先编译为 ll 再运行

```shell
clang++ -O3 -emit-llvm clang-test.cpp -S -o hello.ll
# 可以自行查看内容
cat hello.ll
lli hello.ll
```

## 其他转换

```
.ll -> .bc: llvm-as  a.ll -o a.bc
.bc -> .ll: llvm-dis a.bc -o a.ll
.bc ->  .s: llc a.bc -o a.s
```

# O1 O2 O3 的使用

这三种是用来指定编译器的优化级别，级别越高越激进，越会调整程序的行为，太高了可能会影响后续对 IR 的分析和转换。

```shell
clang -O0 -emit-llvm o-test.c -S -o o0.ll
clang -O1 -emit-llvm o-test.c -S -o o1.ll
clang -O2 -emit-llvm o-test.c -S -o o2.ll
clang -O3 -emit-llvm o-test.c -S -o o3.ll
```

可以看到 O1 删除了调用死代码的部分，O2 进行了循环转换(参考[indvars: Canonicalize Induction Variables](https://llvm.org/docs/Passes.html#indvars-canonicalize-induction-variables))，O3 基本与 O2 一致

# 总结

- 使用 `-emit-llvm` 选项告诉 clang 生成 llvm 的 IR
- `bc` 和 `ll` 都是 IR，只是 bc 是二进制格式，而 ll 是方便阅读、类似汇编的文本格式
- 使用 `-S` 选项生成 ll，使用 `-c` 选项生成 bc
- 使用 -Ox 指定编译的优化级别
