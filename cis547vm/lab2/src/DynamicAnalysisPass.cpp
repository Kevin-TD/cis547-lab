// make ; cd ../ ; cd test ;clang -emit-llvm -S -fno-discard-value-names -c -o simple0.ll simple0.c -g ; opt -load ../build/StaticAnalysisPass.so -StaticAnalysisPass -S simple0.ll -o simple0.static.ll ; opt -load ../build/DynamicAnalysisPass.so -DynamicAnalysisPass -S simple0.ll -o simple0.dynamic.ll ; clang -o simple0 -L../build -lruntime simple0.dynamic.ll ; ./simple0 ; make clean ; cd ../ ; cd build

#include "Instrument.h"
#include "Utils.h"
#include <map> 

using namespace llvm;

namespace instrument {

const auto PASS_NAME = "DynamicAnalysisPass";
const auto PASS_DESC = "Dynamic Analysis Pass";
const auto COVERAGE_FUNCTION_NAME = "__coverage__";
const auto BINOP_OPERANDS_FUNCTION_NAME = "__binop_op__";
std::map<StringRef, int> varToValueMap; 

void instrumentCoverage(Module *M, Instruction &I, int Line, int Col);
void instrumentBinOpOperands(Module *M, BinaryOperator *BinOp, int Line,
                             int Col);

bool Instrument::runOnFunction(Function &F) {
  auto FunctionName = F.getName().str();
  outs() << "Running " << PASS_DESC << " on function " << FunctionName << "\n";

  outs() << "Instrument Instructions\n";
  LLVMContext &Context = F.getContext();
  Module *M = F.getParent();
  

  Type *VoidType = Type::getVoidTy(Context);
  Type *Int32Type = Type::getInt32Ty(Context);
  Type *Int8Type = Type::getInt8Ty(Context);

  M->getOrInsertFunction(COVERAGE_FUNCTION_NAME, VoidType, Int32Type,
                         Int32Type);

  M->getOrInsertFunction(BINOP_OPERANDS_FUNCTION_NAME, VoidType, Int8Type,
                         Int32Type, Int32Type, Int32Type, Int32Type);

  for (inst_iterator Iter = inst_begin(F), E = inst_end(F); Iter != E; ++Iter) {
    Instruction &Inst = (*Iter);
    llvm::DebugLoc DebugLoc = Inst.getDebugLoc();
    if (!DebugLoc) {
      // Skip Instruction if it doesn't have debug information.
      continue;
    }

    int Line = DebugLoc.getLine();
    int Col = DebugLoc.getCol();
    instrumentCoverage(M, Inst, Line, Col);

    /**
     * TODO: Add code to check if the instruction is a BinaryOperator and if so,
     * instrument the instruction as specified in the Lab document.
     */
    
    if (StoreInst *SI = dyn_cast<StoreInst>(&Inst)) {
      // Check if the stored value is an i32
      Value *storedValue = SI->getValueOperand();
      if (storedValue->getType() == Type::getInt32Ty(Inst.getContext())) {
        Value* Sval = SI -> getValueOperand();
        Value* Sptr = SI -> getPointerOperand();

        StringRef name; 
        int value; 

        if (AllocaInst *allocaInst = dyn_cast<AllocaInst>(Sptr)) {
          name = allocaInst->getName();
        }

        if (ConstantInt *constInt = dyn_cast<ConstantInt>(Sval)) {
          value = constInt->getSExtValue();
          varToValueMap[name] = value; 
        }
      }
    }

    
    if (Inst.isBinaryOp()) {  
       BinaryOperator *binaryOperator = dyn_cast<BinaryOperator>(&Inst);
       instrumentBinOpOperands(M, binaryOperator, Line, Col);

    }

  }

  return true;
}

void instrumentCoverage(Module *M, Instruction &I, int Line, int Col) {
  auto &Context = M->getContext();
  auto *Int32Type = Type::getInt32Ty(Context);

  auto LineVal = ConstantInt::get(Int32Type, Line);
  auto ColVal = ConstantInt::get(Int32Type, Col);

  std::vector<Value *> Args = {LineVal, ColVal};

  auto *CoverageFunction = M->getFunction(COVERAGE_FUNCTION_NAME);
  CallInst::Create(CoverageFunction, Args, "", &I);
}

void instrumentBinOpOperands(Module *M, BinaryOperator *BinOp, int Line,
                             int Col) {
  auto &Context = M->getContext();
  auto *Int32Type = Type::getInt32Ty(Context);
  auto *CharType = Type::getInt8Ty(Context);

  /**
   * TODO: Add code to instrument the BinaryOperator to print
   * its location, operation type and the runtime values of its
   * operands.
   */
  Type *VoidType = Type::getVoidTy(Context);
  Type *Int8Type = Type::getInt8Ty(Context);

  M->getOrInsertFunction(BINOP_OPERANDS_FUNCTION_NAME, VoidType, Int8Type,
                         Int32Type, Int32Type, Int32Type, Int32Type);
  
  Type* IntType =Type::getInt32Ty(Context);

  llvm::Value* val1 = BinOp->getOperand(0); 
  llvm::Value* val2 = BinOp->getOperand(1); 

  LoadInst *LI1 = dyn_cast<LoadInst>(BinOp->getOperand(0));
  Value* L1 = LI1 -> getPointerOperand();  

  LoadInst *LI2 = dyn_cast<LoadInst>(BinOp->getOperand(1));
  Value* L2 = LI2 -> getPointerOperand();

  int op1Value = varToValueMap[L1->getName()]; 
  int op2Value = varToValueMap[L2->getName()]; 
  
  outs() << "Division on Line "<< Line << ", Column " << Col << " with first operator=" << op1Value << " and second operator=" << op2Value << "\n";
  
}

char Instrument::ID = 1;
static RegisterPass<Instrument> X(PASS_NAME, PASS_NAME, false, false);

} // namespace instrument
