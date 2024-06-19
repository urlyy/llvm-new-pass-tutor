#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include <map>
#include <iostream>

using namespace llvm;
using std::cout;

void prettyPrint(std::map<std::string,int> &callCounter){
   cout << "================================================="
       << "\n";
  cout << "LLVM-TUTOR: static analysis results\n";
  cout << "=================================================\n";
  const char *str1 = "NAME";
  const char *str2 = "#N DIRECT CALLS";
  printf("%-20s %-10s\n", str1, str2);
  cout << "-------------------------------------------------" << "\n";
  for (auto &kv : callCounter) {
    printf("%-20s %-10d\n", kv.first.c_str(),kv.second);
  }
}

namespace {
  // module,CGSCC, function, loop 这 4 个不同粒度的 Pass。
  // 这让 pass 之间的关系更清楚。并且分别提供了 
  // module,CGSCC, function, loop 4个粒度上的 analysis manager。
  class StaticCallCounterPrinter : public PassInfoMixin<StaticCallCounterPrinter>{
    public:
      PreservedAnalyses run(Module &M,ModuleAnalysisManager &MAM){
        // 被 pass 管理的调用计数器
        std::map<std::string,int> callCounter;
        for (auto &F : M) {
          for (auto &BB : F) {
            for (auto &I : BB) {
              // If this is a call instruction then CB will be not null.
              auto *CB = dyn_cast<CallBase>(&I);
              if (nullptr == CB) {
                continue;
              }
              // If CB is a direct function call then DirectInvoc will be not null.
              auto DirectInvoc = CB->getCalledFunction();
              if (nullptr == DirectInvoc) {
                continue;
              }
              // We have a direct function call
              // update the count for the function being called.
              cout<<DirectInvoc->getName().str()<<"\n";
              callCounter[DirectInvoc->getName().str()]++;
            }
          }
        }
        prettyPrint(callCounter);
        // 没有修改代码，返回all()
        return PreservedAnalyses::all();
      }
  };
}

llvm::PassPluginLibraryInfo getStaticCallCounterPluginInfo() {
    return {
        // 前三个随便
        LLVM_PLUGIN_API_VERSION, "MyPass", "v0.1",
        // 主要是这个
        [](llvm::PassBuilder &PB) {
          // 用这个以支持clang使用
          PB.registerPipelineStartEPCallback(
              [](ModulePassManager &MPM, OptimizationLevel Level) {
                MPM.addPass(StaticCallCounterPrinter());
              }
          );
          PB.registerPipelineParsingCallback(
              [](
                  llvm::StringRef Name, llvm::ModulePassManager &MPM,
                  llvm::ArrayRef <llvm::PassBuilder::PipelineElement>
              ) {
                  if (Name == "static-call-counter") {
                      MPM.addPass(StaticCallCounterPrinter());
                      return true;
                  }
                  return false;
              }
          );
        }
    };      
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getStaticCallCounterPluginInfo();
}



