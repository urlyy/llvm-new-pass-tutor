```shell
clang -O0 -emit-llvm -S main.c -o main.ll
```

clang -O0 -emit-llvm -Xclang -disable-O0-optnone -S main.c -o main.ll
opt -passes=indvars main.ll -S -o new_main.ll
opt -passes=dce main.ll -S -o new_main.ll

```
opt loop-deletion.ll -loop-deletion -S -o new_loop-deletion.ll

opt dce.ll -loop-deletion -S -o new_dce.ll
```

生成 cfg 和 cg

```
参考 https://stackoverflow.com/questions/67393329/llvm-doesnt-generate-cfg
opt main.ll -dot-cfg main.ll -disable-output -enable-new-pm=0 -S -o new_main.ll

opt -dot-callgraph main.ll -disable-output -enable-new-pm=0
```
