#include "Instrument.h"
#include "Utils.h"

using namespace llvm;

namespace instrument {

const auto PASS_NAME = "StaticAnalysisPass";
const auto PASS_DESC = "Static Analysis Pass";

bool Instrument::runOnFunction(Function &F) {
  auto FunctionName = F.getName().str();
  outs() << "Running " << PASS_DESC << " on function " << FunctionName << "\n";

  outs() << "Locating Instructions\n";
  for (inst_iterator Iter = inst_begin(F), E = inst_end(F); Iter != E; ++Iter) {
    Instruction &Inst = (*Iter);
    llvm::DebugLoc DebugLoc = Inst.getDebugLoc();
    if (!DebugLoc) {
      // Skip Instruction if it doesn't have debug information.
      continue;
    }

    int Line = DebugLoc.getLine();
    int Col = DebugLoc.getCol();
    outs() << Line << ", " << Col << "\n";

    /**
     * TODO: Add code to check if the instruction is a BinaryOperator and if so,
     * print the information about its location and operands as specified in the
     * Lab document.
     */
    // idk how it wants me to get "first operand %0 and second operand %1"; the operators just have no name so i put %0 and %1 as default 

    if (Inst.isBinaryOp()) {  
      BinaryOperator::BinaryOps Opcode = (llvm::BinaryOperator::BinaryOps)Inst.getOpcode();
      std::string Opname = getBinOpName(getBinOpSymbol(Opcode));

      LoadInst *LI1 = dyn_cast<LoadInst>(Inst.getOperand(0));
      Value* L1 = LI1 -> getPointerOperand();  
      StringRef operator1_name = L1->getName();

      LoadInst *LI2 = dyn_cast<LoadInst>(Inst.getOperand(1));
      Value* L2 = LI2 -> getPointerOperand();  
      StringRef operator2_name = L2->getName();

      outs() << Opname << " on Line " << Line << ", Column " << Col << " with first operand " << operator1_name << " and second operand " << operator2_name << "\n";
    }
    

  }
  return false;
}

char Instrument::ID = 1;
static RegisterPass<Instrument> X(PASS_NAME, PASS_NAME, false, false);

} // namespace instrument
  