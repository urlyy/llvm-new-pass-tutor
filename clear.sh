# 运行后，删除项目内的所有这些后缀的文件

find $(pwd) -type f \( -name "*.so" -o -name "*.o" -o -name "*.out" -o -name "*.ll" -o -name "*.bc" -o -name "*.exe" \) -exec rm -f {} \;

echo "All .so,.o,.out,.ll,.bc,.exe files have been deleted from subdirectories of $(pwd)"