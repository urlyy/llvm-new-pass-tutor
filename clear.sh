# 运行后，删除项目内的所有这些后缀的文件

find $(pwd) -type f \( -name "*.so" -o -name "*.s" -o -name "*.o" -o -name "*.dot" -o -name "*.out" -o -name "*.ll" -o -name "*.bc" -o -name "*.exe" \) -exec rm -f {} \;

echo "All .so,.s,.o,.dot,.out,.ll,.bc,.exe files have been deleted from subdirectories of $(pwd)"