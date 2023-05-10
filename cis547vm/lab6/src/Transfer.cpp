#include "DivZeroAnalysis.h"
#include "Utils.h"

// (in build) step 4 
// make clean ; cmake -DUSE_REFERENCE=ON .. ; make ; cd ../ ; cd test ; clang -emit-llvm -S -fno-discard-value-names -Xclang -disable-O0-optnone -c -o test03.ll test03.c ; opt -mem2reg -S test03.ll -o test03.opt.ll ; opt -load ../build/DivZeroPass.so -DivZero -disable-output test03.opt.ll > test03.out 2> test03.err ; cd ../ ; cd build

// (in build) step 5 
//  make clean ; cmake -DUSE_REFERENCE=ON .. ; make ; cd ../ ; cd test ; clang -emit-llvm -S -fno-discard-value-names -Xclang -disable-O0-optnone -c -o test03.ll test03.c ; opt -mem2reg -S test03.ll -o test03.opt.ll ; opt -load ../build/DivZeroPass.so -DivZero -disable-output test03.opt.ll > test03.out 2> test03.err ; opt -load ../build/DivZeroPass.so -DivZero -disable-output test03.opt.ll ; cd ../ ; cd build

// rm CMakeCache.txt
//  make clean ; cmake .. ; make ; cd ../ ; cd test ; clang -emit-llvm -S -fno-discard-value-names -Xclang -disable-O0-optnone -c -o test03.ll test03.c ; opt -mem2reg -S test03.ll -o test03.opt.ll ; opt -load ../build/DivZeroPass.so -DivZero -disable-output test03.opt.ll > test03.out 2> test03.err ; opt -load ../build/DivZeroPass.so -DivZero -disable-output test03.opt.ll ; cd ../ ; cd build

// test01
// make clean ; cmake .. ; make ; cd ../ ; cd test ; clang -emit-llvm -S -fno-discard-value-names -Xclang -disable-O0-optnone -c -o test01.ll test01.c ; opt -mem2reg -S test01.ll -o test01.opt.ll ; opt -load ../build/DivZeroPass.so -DivZero -disable-output test01.opt.ll > test01.out 2> test01.err ; opt -load ../build/DivZeroPass.so -DivZero -disable-output test01.opt.ll ; cd ../ ; cd build


#define DEBUG false
#if DEBUG
#define logout(x) errs() << x << "\n";
#else
#define logout(x) 
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

  logout("opname = " << OpName << " opcode = " << BinOp->getOpcode())


  // %sub = sub nsw i32 %sum.0, 55
  //  %add = add nsw i32 %sum.0, %call
  // %div = sdiv i32 %call, %sub

  // hope is that after calling:
  //  %add = add nsw i32 %sum.0, %call
  // merge domains differ, %sum.0 is added back to work list after adding all successros 


    
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
  
  // Get the source value being cast
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
  auto val1 = getOrExtract(InMem, LHS); 
  auto val2 = getOrExtract(InMem, RHS); 

  // assume it refines LHS 

  
  if (op == CmpInst::ICMP_EQ) {
    // 0 and 0 case (1)
    if (Domain::equal(*val1, Domain::Element::Zero) && Domain::equal(*val2, Domain::Element::Zero)) {
      auto* d = new Domain(Domain::Element::NonZero);
      return d;
    }
    
    // S n and S m case 
    if (Domain::equal(*val1, Domain::Element::NonZero) && Domain::equal(*val2, Domain::Element::NonZero)) {
       auto* d = new Domain(Domain::Element::MaybeZero);
       return d;
    }
    
    // 0 and S n case or S n and 0 case 
    auto* d = new Domain(Domain::Element::Zero);
    return d;
  }
  else if (op == CmpInst::ICMP_NE) {
     // 0 and 0 case (0)
    if (Domain::equal(*val1, Domain::Element::Zero) && Domain::equal(*val2, Domain::Element::Zero)) {
      auto* d = new Domain(Domain::Element::Zero);
      return d;
    }
    
    // S n and S m case 
    if (Domain::equal(*val1, Domain::Element::NonZero) && Domain::equal(*val2, Domain::Element::NonZero)) {
       auto* d = new Domain(Domain::Element::MaybeZero);
       return d;
    }
    
    // 0 and S n case or S n and 0 case 
    auto* d = new Domain(Domain::Element::NonZero);
    return d;
  }
  else if (op == CmpInst::ICMP_SGT) {
     // 0 > 0 case: false -> zero
    if (Domain::equal(*val1, Domain::Element::Zero) && Domain::equal(*val2, Domain::Element::Zero)) {
      auto* d = new Domain(Domain::Element::Zero);
      return d;
    }
    
    // S n > S m case -> non zero 
    if (Domain::equal(*val1, Domain::Element::NonZero) && Domain::equal(*val2, Domain::Element::NonZero)) {
       auto* d = new Domain(Domain::Element::NonZero);
       return d;
    }
    
    // S n > 0: true -> nonzero
     if (Domain::equal(*val1, Domain::Element::NonZero) && Domain::equal(*val2, Domain::Element::Zero)) {
       auto* d = new Domain(Domain::Element::NonZero);
       return d;
    }

    // 0 > S n: false -> zero
    auto* d = new Domain(Domain::Element::Zero);
    return d;
  }
  // IMCP_SLT (signed less than)
  // SLE, SGE
  auto* d = new Domain(Domain::Element::MaybeZero);
  return d;
  
}

void DivZeroAnalysis::transfer(Instruction *Inst, const Memory *In,
                               Memory &NOut) {
  logout("reached transfer where inst = " << *Inst << " " << Inst)
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
    // Used for the next lab, do nothing here.
  } else if (auto Store = dyn_cast<StoreInst>(Inst)) {
    // Used for the next lab, do nothing here.
  } else if (auto Load = dyn_cast<LoadInst>(Inst)) {
    // Used for the next lab, do nothing here.
  } else if (auto Branch = dyn_cast<BranchInst>(Inst)) {
    // Analysis is flow-insensitive, so do nothing here.
  } else if (auto Call = dyn_cast<CallInst>(Inst)) {
    // Analysis is intra-procedural, so do nothing here.
  } else if (auto Return = dyn_cast<ReturnInst>(Inst)) {
    // Analysis is intra-procedural, so do nothing here.
  } else {
    errs() << "Unhandled instruction (set to uninit): " << *Inst << "\n";

    // code likely not necessary   but just in case 
    std::string instName = "%" + Inst->getName().str();
    while (instName.size() != 8) {
      instName += " ";
    }
    NOut[instName] = new Domain(Domain::Uninit);
    logout("successfully set to nothing")
  }
}

} // namespace dataflow