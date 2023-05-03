#include "DivZeroAnalysis.h"
#include "Utils.h"

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

std::map<std::string, dataflow::Domain*> totalInstDomainMap; 

// instruction names, as stored in Memory, have a "%" and extra padding to them. I suspect this is so when the program begins loggin the in and out map, it's formatted nicely. Thus, the function adds the necessary extra padding. I do wish that had been mentioned more explicitly! 
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
 * NOTE: after code testing, this only gives one predecessor. I find it kinda weird that it's name "getPredecessors", implying that you will get **several** preds. But as soon as the code finds a single one, it returns a vector of size 1. Strange
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

Memory* copyMemory(Memory* mem) {
  Memory* copy = new Memory();
  for (auto Pair : *mem) {
    copy->emplace(std::make_pair(Pair.first, Pair.second));
  }
  return copy; 
}
// finds in outmap a memory of the same instruction name; if it is not found, an empty memory is returned
Memory* getPrevMemOfInst(Instruction* I, MapType& Map) {
  std::string instName = getInstName(I);
  std::vector<llvm::Instruction*> predsInst = getPredecessors(I);
  SetVector<Instruction *> instructionPathTaken;
  // instructionPathTaken.insert(I);
  
  for (auto Pair : Map) {
    logout("prev inst map = " << getInstName(Pair.first))
    logOutMemory(Pair.second)
  }
  
  while (predsInst.size() != 0) {
    for (auto pred : predsInst) {
      int sizeBefore = instructionPathTaken.size();
      instructionPathTaken.insert(pred);
      int sizeAfter = instructionPathTaken.size();

      if (sizeBefore == sizeAfter) {
      logout("(prevmem) we just may be loopin. found " << *predsInst[0] << " and " << instructionPathTaken.count(predsInst[0]))
      return new Memory(); 
    }

      logout("(prevmeminst) pred = " << getInstName(pred))
      if (getInstName(pred) != instName) continue;
      
      return copyMemory(Map[pred]);
    }
    predsInst = getPredecessors(predsInst[0]);
  }
  return new Memory(); 
}

// populates a map with all previous instructions and coressponding domains 
void constructMap(Instruction* I, MapType& Map) {
  auto predsInst = getPredecessors(I);
  SetVector<Instruction *> instructionPathTaken; 

  for (auto Pair : totalInstDomainMap) {
     Map[I]->emplace(std::make_pair(Pair.first, Pair.second));
  }

  while (predsInst.size() != 0) {
    for (auto pred : predsInst) {
      int sizeBefore = instructionPathTaken.size();
      instructionPathTaken.insert(pred);
      int sizeAfter = instructionPathTaken.size();

      if (sizeBefore == sizeAfter) {
       logout("(cmap) we just may be loopin. found " << *predsInst[0] << " and " << instructionPathTaken.count(predsInst[0]))
      return; 
    }


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

    logout("join i = " << I)

    logout("pre count")
    if (!Mem2->count(I)) {
      Result->emplace(std::make_pair(I, D1));
    } else {
      Domain *D2 = Mem2->at(I);
      Result->emplace(std::make_pair(I, Domain::join(D1, D2)));
    }
  }

  // Iterate over all instructions in Mem2 that are not already in the result and add them to the result
  for (auto Pair : *Mem2) {
    std::string I = Pair.first;
    Domain *D2 = Pair.second;

    logout("join i (d2) = " << I)


    if (!Result->count(I)) {
      Result->emplace(std::make_pair(I, D2));
    }
  }

  return Result;
}

// builds InMem (crucial in making the analysis work, specifically with the transfer function)
void DivZeroAnalysis::flowIn(Instruction *Inst, Memory *InMem) {
  /**
   * TODO: Write your code to implement flowIn.
   *
   * For each predecessor Pred of instruction Inst, do the following:
   *   + Get the Out Memory of Pred using OutMap.
   *   + Join the Out Memory with InMem.
   */

  std::vector<llvm::Instruction*> predsInst = getPredecessors(Inst);
  SetVector<Instruction *> instructionPathTaken; 
  instructionPathTaken.insert(Inst);


  for (auto Pair : totalInstDomainMap) {
    logout("names = " << Pair.first << " " << getInstName(Inst))
    if (Pair.first == getInstName(Inst)) continue; 
     InMem->emplace(std::make_pair(Pair.first, Pair.second));
  }

  while(predsInst.size() != 0) {
    logout("path len = " << instructionPathTaken.size())
    logout("sizeof predsinst = " << predsInst.size())

    for (llvm::Instruction* pred : predsInst) {
      logout("code goes here")
      int sizeBefore = instructionPathTaken.size();
      instructionPathTaken.insert(pred);
      int sizeAfter = instructionPathTaken.size();

      if (sizeBefore == sizeAfter) {
        logout("(flowin) we just may be loopin. found " << *predsInst[0] << " and " << instructionPathTaken.count(predsInst[0]))
        return; 
      }

      logout("pred name = " << getInstName(pred) << " and " << *pred << " with size = " << getInstName(pred).size() )
      if (getInstName(pred).size() == 0) continue; 

      Memory* outMem = OutMap[pred];
      logout("outmem = ")
      logOutMemory(outMem)
      std::string predName = getInstName(pred);

      Memory* joinedMemory = join(outMem, InMem); 
      logout("joined mem = ")
      logOutMemory(joinedMemory)
      logout("flowin-b4 make pair")

      if (joinedMemory->size() == 0) continue; 
      logout("done size check")
      if (!joinedMemory->count(predName)) continue; 
      logout("done count check")

      InMem->emplace(std::make_pair(predName, joinedMemory->at(predName))); // ⭐️ the line of code that changed everything (i.e., made it actually work)
      logout("flowin-b5 make pair")
      totalInstDomainMap.emplace(std::make_pair(predName, joinedMemory->at(predName)));
      logout("total mem map = ")
      for (auto Pair : totalInstDomainMap) {
        logout(Pair.first << " ")
        logDomain(Pair.second)
      }


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
  logout("mems = ")
  logOutMemory(Pre)
  logOutMemory(Post)

  // instruction is something that transfer function does not handle; e.g., "ret i32 0". simply do nothing with it, but construct the outmap accordingly 
  if (getInstName(Inst).size() == 0) {
    constructMap(Inst, OutMap);
    return;
  }

  // check if pre and merged differ. if they do, update outmap and construct preceeding instructions into outmap 
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

  logout("inst name = " << InstName << " for inst " << *Inst)
  logout("domains = ")
  logDomain(PreDomain)
  logDomain(PostDomain)
  logDomain(MergedDomain)

  
  
  if (!Domain::equal(*PreDomain, *MergedDomain)) {
    logout("domains differ, add successors? if they were they would be")

    std::string instName = getInstName(Inst);
    std::vector<llvm::Instruction*> succsInst = getSuccessors(Inst);
    SetVector<Instruction *> instructionPathTaken; 
    bool broken = false;
    while (succsInst.size() != 0) {
      if (broken) break; 
      for (auto succ : succsInst) {
        int sizeBefore = instructionPathTaken.size();
        instructionPathTaken.insert(succ);
        int sizeAfter = instructionPathTaken.size();

        if (sizeBefore == sizeAfter) {
          logout("(prevmem) we just may be loopin. found " << *succsInst[0] << " and " << instructionPathTaken.count(succsInst[0]))
          broken = true; 
          break; 
        }

        logout("succ = " << getInstName(succ))
        WorkSet.insert(succ);
      }
      succsInst = getSuccessors(succsInst[0]);
    }

    OutMap[Inst]->at(InstName) = MergedDomain;
  }

  logout("b4 construct map")
  constructMap(Inst, OutMap);
  logout("b5 construct map")

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
    logout("init push = " << &(*I) << " " << *I)
  }
  
  while(!WorkSet.empty()) {
    Instruction* top = WorkSet.front(); 
    Memory* topMemory = InMap[top];

    logout("inst = " << *top << " name = " << getInstName(top))
    
    logout("b4 flow in")
    flowIn(top, topMemory); // construct top memory 
    logout("b5 flow in")

    Memory* prevOutMemory = getPrevMemOfInst(top, OutMap); 
    logout("prev out memory = ")
    logOutMemory(prevOutMemory)
    logout("size of it is " << prevOutMemory->size())

    logout("b4 transfer")
    transfer(top, topMemory, *OutMap[top]); // use top memory to evaluate instruction's domain. put result into outmap 
    logout("b5 transfer")

    logout("prev out memory = ")
    logOutMemory(prevOutMemory)
    logout("size of it is " << prevOutMemory->size())

    Memory* outMemory = OutMap[top];

    

    // compare pre and post out memory for differences. if there are differneces, adjust outmap's result
    logout("b4 flowout")
    flowOut(top, prevOutMemory, outMemory, WorkSet);
    logout("b5 flowout")

    // doesnt seem like i need to add successor as code still works without a call to getSuccessors

    WorkSet.remove(WorkSet.front());
  }
  

}

} // namespace dataflow