#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include <map>

using namespace llvm;

using std::map;

map<StringRef,Constant*>  FuncNameMap,CallCounterMap;

// 创建一个全局变量，对应一个函数的计数器
Constant *CreateGlobalCounter(Module &M, StringRef GlobalVarName) {
  auto &CTX = M.getContext();
  // This will insert a declaration into M
  Constant *NewGlobalVar =
      M.getOrInsertGlobal(GlobalVarName, IntegerType::getInt32Ty(CTX));
  // This will change the declaration into definition (and initialise to 0)
  GlobalVariable *NewGV = M.getNamedGlobal(GlobalVarName);
  NewGV->setLinkage(GlobalValue::CommonLinkage);
  NewGV->setAlignment(MaybeAlign(4));
  NewGV->setInitializer(llvm::ConstantInt::get(CTX, APInt(32, 0)));
  return NewGlobalVar;
}

namespace {
  class DynamicCallCounter : public PassInfoMixin<DynamicCallCounter>{
    public:
      PreservedAnalyses run(Module &M,ModuleAnalysisManager &MAM){
        // 用来标记是否对IR进行了修改
        bool Instrumented = false;
        auto &CTX = M.getContext();
        // STEP 1: 为每个函数新建访问次数计数器，并在函数体入口处放置一个计数器加一指令
        for (auto &F : M) {
            if (F.isDeclaration())continue;
            // 准备在函数开头创建指令
            IRBuilder<> Builder(&*F.getEntryBlock().getFirstInsertionPt());
            // Create a global variable to count the calls to this function
            std::string CounterName = "CounterFor_" + std::string(F.getName());
            Constant *Var = CreateGlobalCounter(M, CounterName);
            CallCounterMap[F.getName()] = Var;
            // Create a global variable to hold the name of this function
            auto FuncName = Builder.CreateGlobalStringPtr(F.getName());
            FuncNameMap[F.getName()] = FuncName;
            // 加载刚创建的那个计数器变量，然后插入一个自增操作
            LoadInst *Load2 = Builder.CreateLoad(IntegerType::getInt32Ty(CTX), Var);
            Value *Inc2 = Builder.CreateAdd(Builder.getInt32(1), Load2);
            Builder.CreateStore(Inc2, Var);
            Instrumented = true;
        }
        // 如果没有修改IR，直接退出
        if (false == Instrumented)return PreservedAnalyses::all();
        // STEP 2: Inject the declaration of printf
        // ----------------------------------------
        // Create (or _get_ in cases where it's already available) the following
        // declaration in the IR module:
        //    declare i32 @printf(i8*, ...)
        // It corresponds to the following C declaration:
        //    int printf(char *, ...)
        PointerType *PrintfArgTy = PointerType::getUnqual(Type::getInt8Ty(CTX));
        FunctionType *PrintfTy = FunctionType::get(IntegerType::getInt32Ty(CTX), PrintfArgTy,
                              /*IsVarArgs=*/true);

        FunctionCallee Printf = M.getOrInsertFunction("printf", PrintfTy);
        // Set attributes as per inferLibFuncAttributes in BuildLibCalls.cpp
        Function *PrintfF = dyn_cast<Function>(Printf.getCallee());
        PrintfF->setDoesNotThrow();
        PrintfF->addParamAttr(0, Attribute::NoCapture);
        PrintfF->addParamAttr(0, Attribute::ReadOnly);

        // STEP 3: 创建一个printf的format对象和美观打印
        // ------------------------------------------------------------------------
        llvm::Constant *ResultFormatStr =
            llvm::ConstantDataArray::getString(CTX, "%-20s %-10lu\n");

        Constant *ResultFormatStrVar = M.getOrInsertGlobal("ResultFormatStrIR", ResultFormatStr->getType());
        dyn_cast<GlobalVariable>(ResultFormatStrVar)->setInitializer(ResultFormatStr);

        std::string out = "";
        out += "=================================================\n";
        out += "LLVM-TUTOR: dynamic analysis results\n";
        out += "=================================================\n";
        out += "NAME                 #N DIRECT CALLS\n";
        out += "-------------------------------------------------\n";

        llvm::Constant *ResultHeaderStr =
            llvm::ConstantDataArray::getString(CTX, out.c_str());

        Constant *ResultHeaderStrVar =
            M.getOrInsertGlobal("ResultHeaderStrIR", ResultHeaderStr->getType());
        dyn_cast<GlobalVariable>(ResultHeaderStrVar)->setInitializer(ResultHeaderStr);

        // STEP 4: Define a printf wrapper that will print the results
        // -----------------------------------------------------------
        // Define `printf_wrapper` that will print the results stored in FuncNameMap
        // and CallCounterMap.  It is equivalent to the following C++ function:
        // ```
        //    void printf_wrapper() {
        //      for (auto &item : Functions)
        //        printf("llvm-tutor): Function %s was called %d times. \n",
        //        item.name, item.count);
        //    }
        // ```
        // (item.name comes from FuncNameMap, item.count comes from
        // CallCounterMap)
        FunctionType *PrintfWrapperTy = FunctionType::get(llvm::Type::getVoidTy(CTX), {},
                              /*IsVarArgs=*/false);
        Function *PrintfWrapperF = dyn_cast<Function>(M.getOrInsertFunction("printf_wrapper", PrintfWrapperTy).getCallee());
        // Create the entry basic block for printf_wrapper ...
        llvm::BasicBlock *RetBlock =
            llvm::BasicBlock::Create(CTX, "enter", PrintfWrapperF);
        IRBuilder<> Builder(RetBlock);
        // ... and start inserting calls to printf
        // (printf requires i8*, so cast the input strings accordingly)
        llvm::Value *ResultHeaderStrPtr =
            Builder.CreatePointerCast(ResultHeaderStrVar, PrintfArgTy);
        llvm::Value *ResultFormatStrPtr =
            Builder.CreatePointerCast(ResultFormatStrVar, PrintfArgTy);

        Builder.CreateCall(Printf, {ResultHeaderStrPtr});

        LoadInst *LoadCounter;
        for (auto &item : CallCounterMap) {
          LoadCounter = Builder.CreateLoad(IntegerType::getInt32Ty(CTX), item.second);
          // LoadCounter = Builder.CreateLoad(item.second);
          Builder.CreateCall(
              Printf, {ResultFormatStrPtr, FuncNameMap[item.first], LoadCounter});
        }

        // Finally, insert return instruction
        Builder.CreateRetVoid();

        // STEP 5: Call `printf_wrapper` at the very end of this module
        // ------------------------------------------------------------
        appendToGlobalDtors(M, PrintfWrapperF, /*Priority=*/0);
        // 修改了IR，返回none()
        return PreservedAnalyses::none();
      }
  };
}

llvm::PassPluginLibraryInfo getDynamicCallCounterPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "dynamic-cc", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            // 用这个以支持clang使用
            PB.registerPipelineStartEPCallback(
                [](ModulePassManager &MPM, OptimizationLevel Level) {
                  MPM.addPass(DynamicCallCounter());
                }
            );
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "dynamic-cc") {
                    MPM.addPass(DynamicCallCounter());
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getDynamicCallCounterPluginInfo();
}