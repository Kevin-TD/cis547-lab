#include "DivZeroAnalysis.h"
#include "Utils.h"

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
      for (auto Succ = succ_begin(Block), BS = succ_end(Block);
           Succ != BS; ++Succ) {
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

  // Iterate over all instructions in Mem1 and add them to the result.
  for (auto Pair : *Mem1) {
    errs() << "1-join\n";
    std::string I = Pair.first;
    errs() << "2-join\n";
    errs() << "string = " << I << "\n";
    Domain* D1 = Pair.second;
    errs() << "3-join\n";

    if (!Mem2->count(I)) {
      Result->emplace(std::make_pair(I, D1));
    } else {
      Domain *D2 = Mem2->at(I);
      Result->at(I) = Domain::join(D1, D2);
    }
    errs() << "4-join\n";
  }

  // Iterate over all instructions in Mem2 that are not already in the result and add them to the result.
  for (auto Pair : *Mem2) {
    errs() << "5-join\n";
    std::string I = Pair.first;
    Domain *D2 = Pair.second;
    errs() << "6-join\n";
    if (!Result->count(I)) {
      errs() << "7-join\n";
      Result->emplace(std::make_pair(I, D2));
    }
    errs() << "8-join\n";
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
  errs() << "1-flowin\n";
  std::vector<llvm::Instruction*> predsInst = getPredecessors(Inst); // this acutally only gives one successor before 
  errs() << "preds size = " << predsInst.size() << "\n";

  errs() << "2-flowin\n";
  while(predsInst.size() != 0) {
    for (auto pred : predsInst) {

      if (pred->getName().str().size() == 0) continue; 

      errs() << "3-flowin\n";
      errs() << "flow inst = " << *pred << "\n";

      Memory* outMem = OutMap[pred];
      errs() << "4-flowin\n";
      std::string predName = getInstName(pred);

      errs() << "flowin predname = " << predName << "\n";

      Memory* joinedMemory = join(outMem, InMem); 
      printMemory(joinedMemory);
      InMem->emplace(std::make_pair(predName, joinedMemory->at(predName))); // ⭐️

      errs() << "5-flowin\n";
    }
    predsInst = getPredecessors(predsInst[0]);
  }
  printMemory(InMem);


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
  if (getInstName(Inst).size() == 0) {
    errs() << "empty string!\n";

    auto predsInst = getPredecessors(Inst);
    while (predsInst.size() != 0) {
      for (auto pred : predsInst) {
        if (pred->getName().str().size() == 0) continue; 
        std::string predName = getInstName(pred);
        Domain* D;
        if (!OutMap[pred]->count(predName)) {
          D = new Domain(Domain::Uninit);
        } else {
          D = OutMap[pred]->at(predName);
        }

        OutMap[Inst]->emplace(std::make_pair(predName, D));

        errs() << "theory: flow inst = " << pred->getName().str() << "\n";
      }
      predsInst = getPredecessors(predsInst[0]);
    }

    return;
  }


  errs() << "1\n";
  std::string InstName = getInstName(Inst);

  errs() << "2\n";

  Domain *PreDomain; 
  if (!Pre->count(InstName)) {
    PreDomain = new Domain(Domain::Uninit);
  }
  else {
    PreDomain = Pre->at(InstName);
  }

   errs() << "3\n";

  Domain *PostDomain = Post->at(InstName);
   errs() << "4 \n";

  Domain *MergedDomain = Domain::join(PreDomain, PostDomain);
   errs() << "5 domains = \n";
  
  PreDomain->print(errs());
  PostDomain->print(errs());
  MergedDomain->print(errs());


  // Merge the previous Out Memory of Inst with the current Out Memory for each instruction to update the OutMap and WorkSet as needed.
  
  if (!Domain::equal(*PreDomain, *MergedDomain)) {
     errs() << "6\n";
    OutMap[Inst]->at(InstName) = MergedDomain;
     errs() << "7\n";
    auto successors = getSuccessors(Inst);

    for (auto succ : successors) {
      WorkSet.insert(succ);
    }
     errs() << "8\n";
  }

  auto predsInst = getPredecessors(Inst);
  while (predsInst.size() != 0) {
    for (auto pred : predsInst) {
      if (pred->getName().str().size() == 0) continue; 

      std::string predName = getInstName(pred);
      Domain* D;
      if (!OutMap[pred]->count(predName)) {
        D = new Domain(Domain::Uninit);
      } else {
        D = OutMap[pred]->at(predName);
      }

      OutMap[Inst]->emplace(std::make_pair(predName, D));

      errs() << "theory: flow inst = " << pred->getName().str() << "\n";
    }
    predsInst = getPredecessors(predsInst[0]);
  }

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
    errs() << "inst = " << *top << "\n";
    errs() << "inst name = " << top->getName() << "\n";
    
    Memory* topMemory = InMap[top];
        
    errs() << "b4 flowin\n";
    flowIn(top, topMemory);

    errs() << "b5 flowin\n";

    errs() << "b4 transfer\n";

    // really unsure about the params of transfer
    // prev attempts have just made it so in/out maps are not populated with anything, just like one instruction 
   
   transfer(top, topMemory, *OutMap[top]);
   
   errs() << "top mem size = " << topMemory->size() << "\n";
    
    errs() << "b5 transfer\n";


    Memory* outMemory = OutMap[top];
    
    // prev out memory should be same instruction name as current instruction. if there is no match, then prev out memory is simply domain Uninit 
    Memory* prevOutMemory; 
    std::string instName = getInstName(top);
    std::vector<llvm::Instruction*> predsInst = getPredecessors(top);
    bool stop = false; 
    while (predsInst.size() != 0 && !stop) {
      for (auto pred : predsInst) {
        if (getInstName(pred) != instName) continue;
        prevOutMemory = OutMap[pred];
        stop = true; 
        break; 
      }
      predsInst = getPredecessors(predsInst[0]);
    }


    if (stop) {      
      errs() << "found: ";
      printMemory(prevOutMemory);
    }
    
    errs() << "b4 flowout \n";

    // Merge the previous Out Memory of Inst with the current Out Memory for each instruction to update the OutMap and WorkSet as needed.
    flowOut(top, prevOutMemory, outMemory, WorkSet);
    errs() << "b5 flowout \n";

    WorkSet.remove(WorkSet.front());

  }
  

}

} // namespace dataflow