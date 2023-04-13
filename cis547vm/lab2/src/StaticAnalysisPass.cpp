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
      std::string operator1_name = "%0", operator2_name = "%1";

      if (Inst.getOperand(0)->hasName()) {
        operator1_name = Inst.getOperand(0)->getName();
      }

      if (Inst.getOperand(1)->hasName()) {
        operator2_name = Inst.getOperand(1)->getName();
      }

      outs() << Opname << " on Line " << Line << ", Column " << Col << " with first operand " << operator1_name << " and second operand " << operator2_name << "\n";
    }
    

  }
  return false;
}

char Instrument::ID = 1;
static RegisterPass<Instrument> X(PASS_NAME, PASS_NAME, false, false);

} // namespace instrument
