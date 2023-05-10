#include "DivZeroAnalysis.h"
#include "Utils.h"

#define DEBUG true
#if DEBUG
#define logout(x) errs() << x << "\n";
#define logDomain(x) x->print(errs()); 
#define logOutMemory(x) printMemory(x);
#else
#define logout(x) 
#define logDomain(x) 
#define logOutMemory(x) 
#endif 

namespace dataflow {

/**
 * @brief Is the given instruction a user input?
 *
 * @param Inst The instruction to check.
 * @return true If it is a user input, false otherwise.
 */
bool isInput(Instruction *Inst) {
  if (auto Call = dyn_cast<CallInst>(Inst)) {
    if (auto Fun = Call->getCalledFunction()) {
      return (Fun->getName().equals("getchar") ||
              Fun->getName().equals("fgetc"));
    }
  }
  return false;
}

/**
 * Evaluate a PHINode to get its Domain.
 *
 * @param Phi PHINode to evaluate
 * @param InMem InMemory of Phi
 * @return Domain of Phi
 */
Domain *eval(PHINode *Phi, const Memory *InMem) {
  if (auto ConstantVal = Phi->hasConstantValue()) {
    return new Domain(extractFromValue(ConstantVal));
  }

  Domain *Joined = new Domain(Domain::Uninit);

  for (unsigned int i = 0; i < Phi->getNumIncomingValues(); i++) {
    auto Dom = getOrExtract(InMem, Phi->getIncomingValue(i));
    Joined = Domain::join(Joined, Dom);
  }
  return Joined;
}

/**
 * @brief Evaluate the +, -, * and / BinaryOperator instructions
 * using the Domain of its operands and return the Domain of the result.
 *
 * @param BinOp BinaryOperator to evaluate
 * @param InMem InMemory of BinOp
 * @return Domain of BinOp
 */
Domain *eval(BinaryOperator *BinOp, const Memory *InMem) {
  /**
   * TODO: Write your code here that evaluates +, -, * and /
   * based on the Domains of the operands.
   */

  const char *OpName = BinOp->getOpcodeName(); 
  Value *LHS = BinOp->getOperand(0); 
  Value *RHS = BinOp->getOperand(1); 
  auto val1 = getOrExtract(InMem, LHS); 
  auto val2 = getOrExtract(InMem, RHS); 

  if (BinOp->getOpcode() == Instruction::Add) {
    return Domain::add(val1, val2);
  }
  else if (BinOp->getOpcode() == Instruction::Sub) {
    return Domain::sub(val1, val2);
  }
  else if (BinOp->getOpcode() == Instruction::Mul) {
    return Domain::mul(val1, val2);
  }
  else if (BinOp->getOpcode() == Instruction::SDiv || BinOp->getOpcode() == Instruction::UDiv) {
    return Domain::div(val1, val2);
  }

}

/**
 * @brief Evaluate Cast instructions.
 *
 * @param Cast Cast instruction to evaluate
 * @param InMem InMemory of Instruction
 * @return Domain of Cast
 */
Domain *eval(CastInst *Cast, const Memory *InMem) {
  /**
   * TODO: Write your code here to evaluate Cast instruction.
   */
  unsigned Opcode = Cast->getOpcode();
  Value *SrcValue = Cast->getOperand(0);
  return getOrExtract(InMem, SrcValue);
}

/**
 * @brief Evaluate the ==, !=, <, <=, >=, and > Comparision operators using
 * the Domain of its operands to compute the Domain of the result.
 *
 * @param Cmp Comparision instruction to evaluate
 * @param InMem InMemory of Cmp
 * @return Domain of Cmp
 */
Domain *eval(CmpInst *Cmp, const Memory *InMem) {
  /**
   * TODO: Write your code here that evaluates:
   * ==, !=, <, <=, >=, and > based on the Domains of the operands.
   *
   * NOTE: There is a lot of scope for refining this, but you can just return
   * MaybeZero for comparisons other than equality.
   */
  auto op = Cmp->getPredicate(); 
  Value *LHS = Cmp->getOperand(0); 
  Value *RHS = Cmp->getOperand(1); 
  Domain* val1 = getOrExtract(InMem, LHS); 
  Domain* val2 = getOrExtract(InMem, RHS); 
  
  
  if (!Domain::equal(*val1, Domain::Element::MaybeZero)) return val1; 
  if (Domain::equal(*val2, Domain::Element::MaybeZero)) return new Domain(Domain::Element::MaybeZero); 
  if (Domain::equal(*val2, Domain::Element::Uninit)) return new Domain(Domain::Element::Uninit); 

  // values are now only  LHS = MaybeZero, RHS = Zero or NonZero

   if (op == CmpInst::ICMP_EQ) { // ==
    return val2; 
  }
  else if (op == CmpInst::ICMP_NE || op == CmpInst::ICMP_SLT || op == CmpInst::ICMP_SGE) { // !=, <, > 
    if (Domain::equal(*val2, Domain::Element::Zero)) return new Domain(Domain::Element::NonZero);
    return new Domain(Domain::Element::Zero); 
  }
  // <=, >=
  return new Domain(Domain::Element::MaybeZero);


}

void DivZeroAnalysis::transfer(Instruction *Inst, const Memory *In,
                               Memory &NOut, PointerAnalysis *PA,
                               SetVector<Value *> PointerSet) {
  if (isInput(Inst)) {
    // The instruction is a user controlled input, it can have any value.
    NOut[variable(Inst)] = new Domain(Domain::MaybeZero);
  } else if (auto Phi = dyn_cast<PHINode>(Inst)) {
    // Evaluate PHI node
    NOut[variable(Phi)] = eval(Phi, In);
  } else if (auto BinOp = dyn_cast<BinaryOperator>(Inst)) {
    // Evaluate BinaryOperator
    NOut[variable(BinOp)] = eval(BinOp, In);
  } else if (auto Cast = dyn_cast<CastInst>(Inst)) {
    // Evaluate Cast instruction
    NOut[variable(Cast)] = eval(Cast, In);
  } else if (auto Cmp = dyn_cast<CmpInst>(Inst)) {
    // Evaluate Comparision instruction
    NOut[variable(Cmp)] = eval(Cmp, In);
  } else if (auto Alloca = dyn_cast<AllocaInst>(Inst)) {
    // Do nothing here.
  } else if (auto Store = dyn_cast<StoreInst>(Inst)) {
    /**
     * TODO: Store instruction can either add new variables or overwrite existing variables into memory maps.
     * To update the memory map, we rely on the points-to graph constructed in PointerAnalysis.
     *
     * To build the abstract memory map, you need to ensure all pointer references are in-sync, and
     * will converge upon a precise abstract value. To achieve this, implement the following workflow:
     *
     * Iterate through the provided PointerSet:
     *   - If there is a may-alias (i.e., `alias()` returns true) between two variables:
     *     + Get the abstract values of each variable.
     *     + Join the abstract values using Domain::join().
     *     + Update the memory map for the current assignment with the joined abstract value.
     *     + Update the memory map for all may-alias assignments with the joined abstract value.
     *
     * Hint: You may find getOperand(), getValueOperand(), and getPointerOperand() useful.
     */
    logout("doing store inst")

    //  store i32 0, i32* %retval

    Value* storingValue = Store->getOperand(0);       // 0 
    Value* storedToRef = Store->getOperand(1);   // %retval

    logout("stored = " << storingValue << " !!!! " << *storingValue)
    logout("stored = " << storedToRef << " !!!! " << *storedToRef)

    for (llvm::Value* ptr : PointerSet) {
      std::string storedName = variable(storedToRef);
      std::string ptrName = variable(ptr);
       logout("testing " << ptrName << " !!!! " << storedName)
      if (PA->alias(ptrName, storedName)) {
        logout("aliases " << ptrName << " " << storedName)
        Domain* storedDomain = getOrExtract(In, storingValue);
        Domain* ptrDomain = getOrExtract(In, ptr);
        Domain* joinedDomain = Domain::join(storedDomain, ptrDomain);
        NOut[storedName] = joinedDomain; 
        logout("stored to storedname")

        logDomain(storedDomain)
        logDomain(ptrDomain)
        logout("stored then ptr domain logged")
      }
    }


  } else if (auto Load = dyn_cast<LoadInst>(Inst)) {
    /**
     * TODO: Rely on the existing variables defined within the `In` memory to
     * know what abstract domain should be for the new variable
     * introduced by a load instruction.
     *
     * If the memory map already contains the variable, propagate the existing
     * abstract value to NOut.
     * Otherwise, initialize the memory map for it.
     *
     * Hint: You may use getPointerOperand().
     */

    // %0 = load i32, i32* %a, align 4

    Value* storingValue = Load->getPointerOperand();      
    logout("i got a (value)" << storingValue << " !! " << *storingValue << " !! " << variable(storingValue))
    if (In->count(variable(storingValue)) == 0) {
      logout("i have never seen this value in my life beofre. making it uninit")
      NOut[variable(Load)] = new Domain(Domain::Uninit);
    }
    else {
      logout("seen it bfore!! its the uhh " << *In->at(variable(storingValue)) )
      NOut[variable(Load)] = In->at(variable(storingValue));
    }

  } else if (auto Branch = dyn_cast<BranchInst>(Inst)) {
    // Analysis is flow-insensitive, so do nothing here.
  } else if (auto Call = dyn_cast<CallInst>(Inst)) {
    /**
     * TODO: Populate the NOut with an appropriate abstract domain.
     *
     * You only need to consider calls with int return type.
     */
  } else if (auto Return = dyn_cast<ReturnInst>(Inst)) {
    // Analysis is intra-procedural, so do nothing here.
  } else {
    errs() << "Unhandled instruction: " << *Inst << "\n";
  }
}

} // namespace dataflow