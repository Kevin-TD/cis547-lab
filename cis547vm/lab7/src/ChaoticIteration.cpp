#include "DivZeroAnalysis.h"
#include "Utils.h"

// make clean ; cmake .. ; make ; cd ../test ;  clang -emit-llvm -S -fno-discard-value-names -Xclang -disable-O0-optnone -c test03.c -o test03.ll ; opt -load ../build/DivZeroPass.so -DivZero test03.ll ; cd ../build

typedef llvm::ValueMap<llvm::Instruction *, dataflow::Memory *> MapType;

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

std::string getInstName(Instruction* I) {
  std::string instName;

  if (I->getName().size() != 0) {
    instName = "%" + I->getName().str();
  }
  else if (auto Store = dyn_cast<StoreInst>(I)) {
    Value* ptrValue = Store->getOperand(1);
    instName = "%" + ptrValue->getName().str(); 
  }
  else {
    return "";
  }

  while (instName.size() != 8) {
    instName += " ";
  }

  return instName; 
}

Memory* copyMemory(Memory* mem) {
  Memory* copy = new Memory();
  for (auto Pair : *mem) {
    copy->emplace(std::make_pair(Pair.first, Pair.second));
  }
  return copy; 
}

/**
 * @brief Get the Predecessors of a given instruction in the control-flow graph.
 *
 * @param Inst The instruction to get the predecessors of.
 * @return Vector of all predecessors of Inst.
 */
std::vector<Instruction *> getPredecessors(Instruction *Inst) {
  std::vector<Instruction *> Ret;
  auto Block = Inst->getParent();
  for (auto Iter = Block->rbegin(), End = Block->rend(); Iter != End; ++Iter) {
    if (&(*Iter) == Inst) {
      ++Iter;
      if (Iter != End) {
        Ret.push_back(&(*Iter));
        return Ret;
      }
      for (auto Pre = pred_begin(Block), BE = pred_end(Block); Pre != BE;
           ++Pre) {
        Ret.push_back(&(*((*Pre)->rbegin())));
      }
      return Ret;
    }
  }
  return Ret;
}

/**
 * @brief Get the successors of a given instruction in the control-flow graph.
 *
 * @param Inst The instruction to get the successors of.
 * @return Vector of all successors of Inst.
 */
std::vector<Instruction *> getSuccessors(Instruction *Inst) {
  std::vector<Instruction *> Ret;
  auto Block = Inst->getParent();
  for (auto Iter = Block->begin(), End = Block->end(); Iter != End; ++Iter) {
    if (&(*Iter) == Inst) {
      ++Iter;
      if (Iter != End) {
        Ret.push_back(&(*Iter));
        return Ret;
      }
      for (auto Succ = succ_begin(Block), BS = succ_end(Block); Succ != BS;
           ++Succ) {
        Ret.push_back(&(*((*Succ)->begin())));
      }
      return Ret;
    }
  }
  return Ret;
}

/**
 * @brief Joins two Memory objects (Mem1 and Mem2), accounting for Domain
 * values.
 *
 * @param Mem1 First memory.
 * @param Mem2 Second memory.
 * @return The joined memory.
 */
Memory *join(Memory *Mem1, Memory *Mem2) {
  /**
   * TODO: Write your code that joins two memories.
   *
   * If some instruction with domain D is either in Mem1 or Mem2, but not in
   *   both, add it with domain D to the Result.
   * If some instruction is present in Mem1 with domain D1 and in Mem2 with
   *   domain D2, then Domain::join D1 and D2 to find the new domain D,
   *   and add instruction I with domain D to the Result.
   */

  Memory* Result = new Memory();

  // iterate over all instructions in Mem1 and add them to the result
  for (auto Pair : *Mem1) {
    std::string I = Pair.first;
    Domain* D1 = Pair.second;
    if (!Mem2->count(I)) {
      Result->emplace(std::make_pair(I, D1));
    } else {
      Domain *D2 = Mem2->at(I);
      Result->emplace(std::make_pair(I, Domain::join(D1, D2)));
    }
  }

  // iterate over all instructions in Mem2 that are not already in the result and add them to the result
  for (auto Pair : *Mem2) {
    std::string I = Pair.first;
    Domain *D2 = Pair.second;

    if (!Result->count(I)) {
      Result->emplace(std::make_pair(I, D2));
    }
  }

  return Result;
}

void DivZeroAnalysis::flowIn(Instruction *Inst, Memory *InMem) {
  /**
   * TODO: Write your code to implement flowIn.
   *
   * For each predecessor Pred of instruction Inst, do the following:
   *   + Get the Out Memory of Pred using OutMap.
   *   + Join the Out Memory with InMem.
   */

  std::vector<llvm::Instruction*> predsInst = getPredecessors(Inst);
  while (predsInst.size() != 0) {
    for (llvm::Instruction* pred : predsInst) {
      Memory* outMemory = OutMap[pred];
      Memory* joinedMemory = join(outMemory, InMem); 

      for (auto Pair : *joinedMemory) {
          InMem->emplace(std::make_pair(Pair.first, joinedMemory->at(Pair.first))); // ⭐️
      }

    }
    predsInst = getPredecessors(predsInst[0]);
  }
}

/**
 * @brief This function returns true if the two memories Mem1 and Mem2 are
 * equal.
 *
 * @param Mem1 First memory
 * @param Mem2 Second memory
 * @return true if the two memories are equal, false otherwise.
 */
bool equal(Memory *Mem1, Memory *Mem2) {
  /**
   * TODO: Write your code to implement check for equality of two memories.
   *
   * If any instruction I is present in one of Mem1 or Mem2,
   *   but not in both and the Domain of I is not UnInit, the memories are
   *   unequal.
   * If any instruction I is present in Mem1 with domain D1 and in Mem2
   *   with domain D2, if D1 and D2 are unequal, then the memories are unequal.
   */
  return false;
}

void DivZeroAnalysis::flowOut(Instruction *Inst, Memory *Pre, Memory *Post,
                              SetVector<Instruction *> &WorkSet) {
  /**
   * TODO: Write your code to implement flowOut.
   *
   * For each given instruction, merge abstract domain from pre-transfer memory
   * and post-transfer memory, and update the OutMap.
   * If the OutMap changed then also update the WorkSet.
   */

  std::string InstName = getInstName(Inst);
  if (Post->count(InstName) == 0) return; 

  Domain *PreDomain; 
  if (Pre->count(InstName) == 0) {
    PreDomain = new Domain(Domain::Uninit);
  }
  else {
    PreDomain = Pre->at(InstName);
  }

  Domain *PostDomain = Post->at(InstName);
  logout("read post domain")

  Domain *MergedDomain = Domain::join(PreDomain, PostDomain);
  
  if (!Domain::equal(*PreDomain, *MergedDomain)) {
    std::vector<llvm::Instruction*> succsInst = getSuccessors(Inst);
    for (auto succ : succsInst) {
      WorkSet.insert(succ);
    }
  }

  OutMap[Inst]->at(InstName) = MergedDomain;
  logout("read outmap[inst]")

}

void DivZeroAnalysis::doAnalysis(Function &F, PointerAnalysis *PA) {
  SetVector<Instruction *> WorkSet;
  SetVector<Value *> PointerSet;
  /**
   * TODO: Write your code to implement the chaotic iteration algorithm
   * for the analysis.
   *
   * First, find the arguments of function call and instantiate abstract domain values
   * for each argument.
   * Initialize the WorkSet and PointerSet with all the instructions in the function.
   * The rest of the implementation is almost similar to the previous lab.
   *
   * While the WorkSet is not empty:
   * - Pop an instruction from the WorkSet.
   * - Construct it's Incoming Memory using flowIn.
   * - Evaluate the instruction using transfer and create the OutMemory.
   *   Note that the transfer function takes two additional arguments compared to previous lab:
   *   the PointerAnalysis object and the populated PointerSet.
   * - Use flowOut along with the previous Out memory and the current Out
   *   memory, to check if there is a difference between the two to update the
   *   OutMap and add all successors to WorkSet.
   */

  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    WorkSet.insert(&(*I));
    PointerSet.insert(&(*I));
    logout("init push = " << &(*I) << " " << *I)
  }

  while(!WorkSet.empty()) {
    Instruction* top = WorkSet.front(); 
    Memory* topMemory = InMap[top];

    logout("inst = " << *top << " name = " << getInstName(top))
    
    flowIn(top, topMemory);
    logout("done flow in")
    logout("top mem read = ")
    logOutMemory(topMemory)

    Memory* prevOutMemory = copyMemory(OutMap[top]);
    logout("done copy memory, read = ")
    logOutMemory(prevOutMemory)

    transfer(top, topMemory, *OutMap[top], PA, PointerSet); 
    logout("done transfer")

    Memory* currOutMemory = OutMap[top];
    logout("read curr out")
    logOutMemory(currOutMemory)

    flowOut(top, prevOutMemory, currOutMemory, WorkSet);
    logout("done flowout")
   
    WorkSet.remove(WorkSet.front());
  }


}

} // namespace dataflow