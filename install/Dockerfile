FROM ubuntu:22.04

RUN apt update
RUN apt install -y lsb-release wget software-properties-common gnupg
# 使用脚本安装
RUN wget https://apt.llvm.org/llvm.sh
RUN chmod +x llvm.sh
RUN ./llvm.sh 18 all
RUN rm -f https://apt.llvm.org/llvm.sh
# 入门基本就用到这个几个
RUN ln -s /usr/bin/clang++-18 /usr/bin/clang++
RUN ln -s /usr/bin/clang-18 /usr/bin/clang
RUN ln -s /usr/bin/llvm-config-18 /usr/bin/llvm-config
RUN ln -s /usr/bin/lli-18 /usr/bin/lli
RUN ln -s /usr/bin/opt-18 /usr/bin/opt

WORKDIR /app

CMD ["/bin/bash"]