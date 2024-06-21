#include "llvm/Support/CommandLine.h"
#include <iostream>
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"

using namespace llvm;

static cl::opt<std::string> Urlyy(
    "urlyy",
    cl::desc("a urlyy"),
    cl::value_desc("urlyy value")
);

static cl::opt<bool> Wave("wave-goodbye", cl::init(false),
                          cl::desc("wave good bye"));

namespace{
  class HelloWorldPass : public PassInfoMixin<HelloWorldPass> {
  public:
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM){
        std::cout << F.getName().str() << "\n";
        std::cout<<Urlyy<<'\n';
        if(Wave){
            std::cout<<"have waved" << "\n";
        }
        return PreservedAnalyses::all();
    }
    // 注意需要加这个，否则加-Xclang -disable-O0-optnone
    static bool isRequired() { return true; }
  };
} 


llvm::PassPluginLibraryInfo getPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION, "MyPass", LLVM_VERSION_STRING,
        [](llvm::PassBuilder &PB) {
            // 用这个以支持clang使用
            PB.registerPipelineStartEPCallback(
                [](ModulePassManager &MPM, OptimizationLevel Level) {
                    FunctionPassManager FPM;                
                    FPM.addPass(HelloWorldPass());
                    MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
                }
            );
            PB.registerPipelineParsingCallback(
                [](
                    llvm::StringRef Name, llvm::ModulePassManager &MPM,
                    llvm::ArrayRef <llvm::PassBuilder::PipelineElement>
                ) {
                    if (Name == "about-option") {
                        MPM.addPass(createModuleToFunctionPassAdaptor(HelloWorldPass()));
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
  return getPluginInfo();
}

