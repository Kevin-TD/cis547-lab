#include "DivZeroAnalysis.h"
#include "Utils.h"

typedef llvm::ValueMap<llvm::Instruction *, dataflow::Memory *> MapType;

namespace dataflow {

std::string getInstName(Instruction* I) {
  if (I->getName().size() == 0) return "";
  std::string instName = "%" + I->getName().str();
  while (instName.size() != 8) {
    instName += " ";
  }
  return instName; 
}


/**
 * @brief Get the Predecessors of a given instruction in the control-flow graph.
 * NOTE: after code testing, this only gives one predecessor. I find it kinda weird that it's name "getPredecessors", implying that you will get **several** preds. But as soon as the code finds a single one, it returns a vector of size 1. Strange! 
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
 * NOTE: after code testing, this only gives one successor. 
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
      for (auto Succ = succ_begin(Block), BS = succ_end(Block);
           Succ != BS; ++Succ) {
        Ret.push_back(&(*((*Succ)->begin())));
      }
      return Ret;
    }
  }
  return Ret;
}

Memory* getPrevMemOfInst(Instruction* I, MapType& Map) {
  std::string instName = getInstName(I);
  std::vector<llvm::Instruction*> predsInst = getPredecessors(I);
  while (predsInst.size() != 0) {
    for (auto pred : predsInst) {
      if (getInstName(pred) != instName) continue;
      return Map[pred];
    }
    predsInst = getPredecessors(predsInst[0]);
  }
  return new Memory(); 
}

void constructMap(Instruction* I, MapType& Map) {
  auto predsInst = getPredecessors(I);
  while (predsInst.size() != 0) {
    for (auto pred : predsInst) {
      if (pred->getName().str().size() == 0) continue; 
      std::string predName = getInstName(pred);
      Domain* D;
      if (!Map[pred]->count(predName)) {
        D = new Domain(Domain::Uninit);
      } else {
        D = Map[pred]->at(predName);
      }
      Map[I]->emplace(std::make_pair(predName, D));
    }
    predsInst = getPredecessors(predsInst[0]);
  }
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

  // iterates over all instructions in Mem1 and add them to the result
  for (auto Pair : *Mem1) {
    std::string I = Pair.first;
    Domain* D1 = Pair.second;

    if (!Mem2->count(I)) {
      Result->emplace(std::make_pair(I, D1));
    } else {
      Domain *D2 = Mem2->at(I);
      Result->at(I) = Domain::join(D1, D2);
    }
  }

  // Iterate over all instructions in Mem2 that are not already in the result and add them to the result
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

  // builds InMem (crucial in making the analysis work, specifically with the transfer function analysis)
  std::vector<llvm::Instruction*> predsInst = getPredecessors(Inst);
  while(predsInst.size() != 0) {
    for (auto pred : predsInst) {

      if (getInstName(pred).size() == 0) continue; 
      Memory* outMem = OutMap[pred];
      std::string predName = getInstName(pred);

      Memory* joinedMemory = join(outMem, InMem); 
      InMem->emplace(std::make_pair(predName, joinedMemory->at(predName))); // ⭐️

    }
    predsInst = getPredecessors(predsInst[0]);
  }


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

  if (getInstName(Inst).size() == 0) {
    constructMap(Inst, OutMap);
    return;
  }


  std::string InstName = getInstName(Inst);
  Domain *PreDomain; 
  if (!Pre->count(InstName)) {
    PreDomain = new Domain(Domain::Uninit);
  }
  else {
    PreDomain = Pre->at(InstName);
  }

  Domain *PostDomain = Post->at(InstName);
  Domain *MergedDomain = Domain::join(PreDomain, PostDomain);
  
  
  if (!Domain::equal(*PreDomain, *MergedDomain)) {
    OutMap[Inst]->at(InstName) = MergedDomain;
  }

  constructMap(Inst, OutMap);

}


void DivZeroAnalysis::doAnalysis(Function &F) {
  SetVector<Instruction *> WorkSet;
  /**
   * TODO: Write your code to implement the chaotic iteration algorithm
   * for the analysis.
   *
   * Initialize the WorkSet with all the instructions in the function.
   *
   * While the WorkSet is not empty:
   * - Pop an instruction from the WorkSet.
   * - Construct it's Incoming Memory using flowIn.
   * - Evaluate the instruction using transfer and create the OutMemory.
   * - Use flowOut along with the previous Out memory and the current Out
   *   memory, to check if there is a difference between the two to update the
   *   OutMap and add all successors to WorkSet.
   */

  for(inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    WorkSet.insert(&(*I));
  }
  while(!WorkSet.empty()) {
    Instruction* top = WorkSet.front(); 
    Memory* topMemory = InMap[top];

    flowIn(top, topMemory); // construct top memory 

    transfer(top, topMemory, *OutMap[top]); // use top memory to evaluate instruction's domain. put result into outmap 

    Memory* outMemory = OutMap[top];
    Memory* prevOutMemory = getPrevMemOfInst(top, OutMap); 

    // compare pre and post out memory for differences. if there are differneces, adjust outmap's result
    flowOut(top, prevOutMemory, outMemory, WorkSet);

    // doesnt seem like i need to add successor as code still works without a call to getSuccessors

    WorkSet.remove(WorkSet.front());
  }
  

}

} // namespace dataflow