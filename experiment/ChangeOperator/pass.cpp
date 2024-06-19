#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
using namespace llvm;


void changeFromAddToMul(BinaryOperator *op) {
   
    IRBuilder<> builder(op);
    // Make a multiply with the same operands as `op`.
    Value *lhs = op->getOperand(0);
    Value *rhs = op->getOperand(1);
    Value *mul = builder.CreateMul(lhs, rhs);
    // Everywhere the old instruction was used as an operand, use our
    // new multiply instruction instead.
    for (auto &U : op->uses())
    {
        User *user = U.getUser(); // A User is anything with operands.
        // here we change from plus to multiply
        user->setOperand(U.getOperandNo(), mul);
    }
}

namespace{
    class ChangeOperatorPass : public PassInfoMixin<ChangeOperatorPass>{
    public:
        static bool isRequired() { return true; }
        PreservedAnalyses run(Function &F,FunctionAnalysisManager &FAM){
            if (!F.isIntrinsic()){
                // Get the function to call from our runtime library.
                LLVMContext &Ctx = F.getContext();
                for (auto &B : F){
                    for (auto &I : B){
                        if (auto *op = dyn_cast<BinaryOperator>(&I)){
                            changeFromAddToMul(op);
                        }
                    }
                }
            }
            return PreservedAnalyses::all();
        }
    };
}

llvm::PassPluginLibraryInfo getPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION, "MyPass", "v0.1",
        [](llvm::PassBuilder &PB) {
            // 用这个以支持clang使用
            PB.registerPipelineStartEPCallback(
                [](ModulePassManager &MPM, OptimizationLevel Level) {
                    FunctionPassManager FPM;                
                    FPM.addPass(ChangeOperatorPass());
                    MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
                }
            );
            PB.registerPipelineParsingCallback(
                [](
                    llvm::StringRef Name, llvm::ModulePassManager &MPM,
                    llvm::ArrayRef <llvm::PassBuilder::PipelineElement>
                ) {
                    if (Name == "change-operator") {
                        // 注意因为pass实现的是Function的那种，所以要转成Module
                        MPM.addPass(createModuleToFunctionPassAdaptor(ChangeOperatorPass()));
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

