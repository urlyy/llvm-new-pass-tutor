#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/TypeFinder.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;

namespace {
    // 旧版pass，继承FunctionPass
    struct SkeletonPass : public FunctionPass {
        static char ID;
        SkeletonPass() : FunctionPass(ID) {}
        // 编译器每遇到函数体就会执行这个
        bool runOnFunction(Function& F) override  {
            // 获取函数上下文
            LLVMContext& Ctx = F.getContext();
            // 定义插桩函数原型, 参数类型+返回类型+插桩函数
            Type* retType = Type::getVoidTy(Ctx);
            std::vector<Type*> param_types = { Type::getHalfPtrTy(Ctx), Type::getInt32Ty(Ctx), Type::getInt32Ty(Ctx) };
            FunctionType* func_type = FunctionType::get(retType, param_types, false);
            // 被调用的函数对象
            FunctionCallee rt_posix_memalign = F.getParent()->getOrInsertFunction(
                "rt_posix_memalign", func_type
            );
            // 迭代基本块
            for (auto& B : F) {
                // 迭代指令
                for (auto& I : B) {
                    // 判断指令是否是函数调用指令 CallInst
                    if (auto* call = dyn_cast<CallInst>(&I)) {
                        // 获取指令对应的被调用的函数
                        Function* fun = call->getCalledFunction();
                        if (!fun) {
                            continue;
                        }
                        // 只对posix_memalign函数调用的前后进行自定义函数插入
                        if (0 == fun->getName().compare(StringRef("posix_memalign"))) {
                            // 创建 IRBuilder, 用于 IR 指令构建
                            IRBuilder<> builder(call);
                            // 设置插桩点, 在当前指令前插桩
                            builder.SetInsertPoint(&B, builder.GetInsertPoint());
                            // 通过++，在指令后再插一个桩
                            auto* call2 = dyn_cast<CallInst>(&I);
                            IRBuilder<> builder2(call2);
                            builder2.SetInsertPoint(&B, ++builder2.GetInsertPoint());
                            // 将当前调用 posix_memalign 时的传参推到 args 中
                            std::vector<Value*> args;
                            for (auto arg = call->arg_begin(); arg != call->arg_end(); ++arg) {
                                args.push_back(*arg);
                            }
                            // 使用builder创建指令，将两个插桩函数插入上下文中, 分别在函数调用前后插桩
                            builder.CreateCall(rt_posix_memalign, args);
                            builder2.CreateCall(rt_posix_memalign, args);
                        }
                    }
                }
            }
            // 这里返回false表示没有改动
            return false;
        }
    };
}  // namespace

char SkeletonPass::ID = 0;

// 下方是注册之类的操作
static RegisterPass<SkeletonPass> X("rhpass", "Hello World Pass");

// 似乎需要下面这块才能用clang执行
static void registerSkeletonPass(const PassManagerBuilder&,legacy::PassManagerBase& PM) {
  PM.add(new SkeletonPass());
}
static RegisterStandardPasses RegisterMyPass(
    PassManagerBuilder::EP_EarlyAsPossible,
    registerSkeletonPass
);