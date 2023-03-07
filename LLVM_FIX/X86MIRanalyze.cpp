#include "X86.h"
#include "X86InstrBuilder.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/ScopeExit.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/SparseBitVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/MachineSSAUpdater.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/CodeGen/TargetSchedule.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/MC/MCSchedule.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include <algorithm>
#include <cassert>
#include <iterator>
#include <utility>

#include <iostream>

using namespace llvm;

#define PASS_KEY "x86-miranalysize"

static cl::opt<bool> EnableAnalysize(
    "x86-mir-analyze",
    cl::desc("start MIR Analyzing"), cl::init(false),
    cl::Hidden);

namespace {
class X86MIRAnalyzePass : public MachineFunctionPass {
public:
  X86MIRAnalyzePass() : MachineFunctionPass(ID) { }
   StringRef getPassName() const override {
    return "X86 MIR Analyser";
  }
  bool runOnMachineFunction(MachineFunction &MF) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;

  /// Pass identification, replacement for typeid.
  static char ID;
private:
  const X86Subtarget *Subtarget = nullptr;
  MachineRegisterInfo *MRI = nullptr;
  const X86InstrInfo *TII = nullptr;
  const TargetRegisterInfo *TRI = nullptr;

  // If the function has a parameter and it is used in a branch or memory_access.
  // we need to analyze it.
  void constant_timing_analze(MachineFunction &MF);

  // Track instructions that are related to function liveins 
  SmallDenseMap<unsigned, unsigned, 32> liveintrack(MachineFunction &MF);

  // Track those inputs that only leak in one branch
  // We call this register: secret
  SmallDenseMap<unsigned, unsigned, 32> secrettrack(MachineFunction &MF);


  void print_taintMI(MachineFunction &MF, SmallDenseMap<unsigned, unsigned, 32> taint_registers, SmallVector<MachineInstr *> taint_flagsets);

  SmallVector<MachineInstr *> track_flagsets(MachineFunction &MF);

  int num_predecessors(MachineBasicBlock * MBB);
};
}

char X86MIRAnalyzePass::ID = 0;

void X86MIRAnalyzePass::getAnalysisUsage(
    AnalysisUsage &AU) const {
  MachineFunctionPass::getAnalysisUsage(AU);
}

bool X86MIRAnalyzePass::runOnMachineFunction(
    MachineFunction &MF) {
  if (!EnableAnalysize)
    return false;

  Subtarget = &MF.getSubtarget<X86Subtarget>();
  MRI = &MF.getRegInfo();
  TII = Subtarget->getInstrInfo();
  TRI = Subtarget->getRegisterInfo();

  if (MF.begin() == MF.end())
    return false;
  
  //std::cout << "Begin with ---->  " << MF.getName().str() << std::endl;
  //MF.dump();

 #if 0 
  // Simple analyze on REP instruction
  for (auto &MBB : MF) {
    for (auto &MI : MBB) {
      auto InsertPt = MI.getIterator();
      const DebugLoc &Loc = MI.getDebugLoc();

      //MI.dump();
      if (MI.getOpcode() == X86::REP_MOVSB_64) {
        printf("***** Get REP ******\n");
        // Insert lfence
        BuildMI(MBB, std::prev(InsertPt), Loc, TII->get(X86::LFENCE));
      }
      // std::cout << MI.getOpcode() << std::endl;
      // printf("===\n");
    }
  }
#endif

  //return true;
  
  // For taint analysis, we need to firstly define the source
  // The source normally is the function arugment, so we need to track
  // the livein of functions.
  ///////auto taint_registers = liveintrack(MF);

  // Get all flag setting instructions (for control flow transfer)
  auto taint_flagsetMIs = track_flagsets(MF);
  SmallDenseMap<MachineBasicBlock *, int> taint_MBBs;
  for (MachineInstr *MI : taint_flagsetMIs) {
    MachineBasicBlock *MBB = MI->getParent();
    if (!taint_MBBs.count(MBB))
      ++taint_MBBs[MBB];
  }

  //printf("\n ==============********** \n");
  /////print_taintMI(MF, taint_registers, taint_flagsetMIs);

  // Let's think of a new method
  // Start from each livein, we only track the propagation of this variable
  // We filter memory access related to these variables
  // We print the function, if the variable is used for branching instructions
  
  SmallDenseMap<unsigned, unsigned, 32> tmp_registers;
  SmallDenseMap<MachineInstr *, int> taint_MIs;
  int flag_time = 0;
  MachineInstr *MI_print = NULL;
  int livein_count = 0;

  for (auto &Li : MF.begin()->liveins()) {
    
    // After this loop, we should have all registers propagated from the Li
    for (auto &MBB : MF) {
      for (auto &MI : MBB) {
        for (auto &MO : MI.explicit_operands()) {
          if (MO.isReg() && MO.isUse() && (Li.PhysReg == MO.getReg())) {
            for (auto & MO_tmp : MI.defs() )
	      if (MO_tmp.isReg())
		tmp_registers[MO_tmp.getReg()] = MO_tmp.getReg();
          } else if (MO.isReg() && MO.isUse() && tmp_registers.count(MO.getReg())) {
            for (auto & MO_tmp : MI.defs() )
	      if (MO_tmp.isReg())
		tmp_registers[MO_tmp.getReg()] = MO_tmp.getReg();
            // The secret should not be leaked via memory access
            if (MI.mayLoadOrStore())
              goto end_search1;
          }
        }
      }
    }

    // We have a strict definition on what is secret
    // Now we have a list of taint registers, check which one is used in control flow transfers
    // *** At this stage, we don't search for nested branch.
    flag_time = 0;
    MI_print = NULL;

    for (MachineInstr *MI : taint_flagsetMIs) {
      MachineBasicBlock * MBB = MI->getParent();

     





      // // The MBB should only have one predecessor and its predecessor should have 2 successors
      // // If this MBB will always be accessed, then it does not contain a secret
      // int predecessor_count = 0, successor_count = 0;
      // for (auto &MBB_iter : MBB->predecessors()) {
      //   predecessor_count += 1;
      //   for (auto &MBB_iter2 : MBB_iter->successors())
      //     successor_count += 1;
      // }

      // if (predecessor_count > 1) {
      //   //printf("Too many predecessors\n");
      //   continue;
      // } else {
      //   if (successor_count < 2) {
      //     //printf("Too few successors\n");
      //     continue;
      //   }
      // }

      // Check whether the taint register is only used once in branches.
      for (auto &MO : MI->explicit_operands()) {
        if (!MO.isReg()) continue;
        if (tmp_registers.count(MO.getReg())) {
          flag_time += 1;
          break;
        }
      }

      // It would be nice if it is comparing with a constant, sometimes CMP %1, %1 if the constant is 0
      int flag_constant = 0;

//******************* Check it 
      // If we find that more than one branch instruction uses a tainted variable, we should abort searching
      // There should be a discussion on whether we should abort. It's a very strict gadget searching definition.
      // But it should be faster in finding gadgets with high possibility to leak data.
      if (flag_time >= 2)
        goto end_search1; //break;


      for (auto &MO : MI->explicit_operands()) {
        if (MO.isImm())
          flag_constant = 1;
      }

      if (!flag_constant && MI->getNumOperands() >= 2) {
        if ((MI->getOperand(0).isReg() && MI->getOperand(1).isReg()) && (MI->getOperand(0).getReg() == MI->getOperand(1).getReg()))
          flag_constant = 1;
      }
      
      // Skip this and search for library
      //if (!flag_constant)
        //continue;

      // MI is the instruction that leaks

      for (auto &MO : MI->explicit_operands()) {
        if (!MO.isReg())  continue;
        if (tmp_registers.count(MO.getReg())) {

          // We are looking for nested branches
          for (auto &MBB_iter : MBB->predecessors()) {
            if (taint_MBBs.count(MBB_iter)) {
              if (taint_MIs.count(MI))
                break;
              ++taint_MIs[MI];
              MI_print = MI;
            } 
          }
          // An instruction may contain multiple taint variables
          break;
        }
      }
    }

    if (flag_time == 1 && MI_print != NULL) {
      
      //MI_print->getParent()->dump();
      //printf("----------\n");

      // MI_print is the instruction that leaks, it has multiple predecessors
      // We do not want one of its predecessor is the successor of the other predecessor
      // if (a) c += 1;
      // if (b) d += 1;
      SmallDenseMap<MachineBasicBlock *, int> check_predecessors;
      for (auto tmp_MBB : MI_print->getParent()->predecessors()) {
        if (!check_predecessors.count(tmp_MBB))
          ++check_predecessors[tmp_MBB];
      }

      // For each element in check_predecessor, we check if it's successor is in the predecessor list
      for (auto tmp_MBB : MI_print->getParent()->predecessors()) {
        for (auto tmp_MBB2 : tmp_MBB->successors()) {
          if (check_predecessors.count(tmp_MBB2))
            goto end_search1;
        }
      }

      int memory_access[2] = {0};
      int function_calls[2] = {0};
      int count = 0;
      for (auto MBB_successors : MI_print->getParent()->successors()) {
        



        for (auto &&MI : *MBB_successors) {
          if (MI.mayLoadOrStore())
            memory_access[count] += 1;
          if (MI.isCall()) {
            for (auto MO : MI.operands()) {
              //printf("hello\n");
              std::string str;
              raw_string_ostream OS(str);
              //MO.print(OS);
              //std::cout << OS.str() << std::endl;
              //if (MO.isGlobal())
                //std::cout << MO.getGlobal()->getSection().str() << "  " << MO.getGlobal()->getPartition().str() << std::endl;
            }
            function_calls[count] += 1;
          }
        }
        count += 1;
      }


      //printf("------------\n");
      if ((memory_access[0] != memory_access[1]) || (function_calls[0] != function_calls[1]))
        std::cout << "Found in ---->  " << MF.getName().str() << " livein " << livein_count; //<< std::endl;
      if ((memory_access[0] != memory_access[1]))
        std::cout << ", memory_access ";
      if ((function_calls[0] != function_calls[1]))
        std::cout << ", function_calls ";
      std::cout << " " << std::endl;

      //printf("Memory_access: %d <--> %d\n", memory_access[0], memory_access[1]);
      //MI_print->dump();
    }



end_search1:
    tmp_registers.clear();
    livein_count += 1;
  }


  return true;
}


// Check whether the input has been used in memory access or control-flow transfer
void X86MIRAnalyzePass::constant_timing_analze(MachineFunction &MF)
{
  // for (auto& arg_tmp : MF.getFunction().args()) {
  //   arg_tmp.dump();
  // }

  //MF.dump();
  // If we see a memory access, we track whether 
  // for (MachineBasicBlock &MBB : MF) {
  //   for (MachineInstr &MI : MBB) {
  //     liveintrack(MI);
  //     // if (MI.mayLoad())
  //     //   MI.dump();
  //   }
  // }
}

// Livin is physical or virtual?
// Okay... There is no function to track the propogation of a virtual register
// We need to define src, then we pull all functions that is related to the src
// Only define src at if (MBB == MBB.begin())
SmallDenseMap<unsigned, unsigned, 32> X86MIRAnalyzePass::liveintrack(MachineFunction &MF)
{
  SmallDenseMap<unsigned, unsigned, 32> taint_registrers;
  
  // Get function argument, we assume it only has one argument now
   MachineBasicBlock &entry = *MF.begin();

  for (auto &MBB : MF) {
    for (auto &MI : MBB) {
      for (auto &MO : MI.explicit_operands()) {
        if ((MO.isReg() && entry.isLiveIn(MO.getReg()) && MO.isUse()))  {
          // printf("Find Livin 1\n");
          // MI.dump();
          taint_registrers[MO.getReg()] = MO.getReg();
          for (auto & MO_tmp : MI.defs() )
            taint_registrers[MO_tmp.getReg()] = MO_tmp.getReg();
        }else if (MO.isReg() && MO.isUse() && taint_registrers.count(MO.getReg())) {
          // printf("Find Livin 2\n");
          // MI.dump();
          for (auto & MO_tmp : MI.defs()) 
            taint_registrers[MO_tmp.getReg()] = MO_tmp.getReg();
        }
      }
    }
  }

  return taint_registrers;
}

// Okay we need to call this function for each livein register. Needs to separate this function
SmallDenseMap<unsigned, unsigned, 32> X86MIRAnalyzePass::secrettrack(MachineFunction &MF)
{
  SmallDenseMap<unsigned, unsigned, 32> taint_registrers;
  SmallDenseMap<unsigned, unsigned, 32> initial_registers;
  SmallDenseMap<unsigned, unsigned, 32> tmp_registers;

  
  // Get function argument
  MachineBasicBlock &entry = *MF.begin();

  for (auto &Li : entry.liveins()) {
    for (auto &MBB : MF) {
      for (auto &MI : MBB) {
        for (auto &MO : MI.explicit_operands()) {
          // Add it to map if it is the inital taint Reg, each time we only evaluate one
          if (MO.isReg() && MO.isUse() && (Li.PhysReg == MO.getReg()))
            tmp_registers[MO.getReg()] = MO.getReg();
          else if (MO.isReg() && MO.isUse() && tmp_registers.count(MO.getReg()))
            tmp_registers[MO.getReg()] = MO.getReg();
        }
      }
    }
  }






#if 0  
  // Initialise the initial_registers, as we need to track them separately
  for (auto &MBB : MF) {
    for (auto &MI : MBB) {
      for (auto &MO : MI.explicit_operands()) {
        if ((MO.isReg() && entry.isLiveIn(MO.getReg()) && MO.isUse()))  {
          for (auto & MO_tmp : MI.defs() )
            initial_registers[MO_tmp.getReg()] = MO_tmp.getReg();
        }
      }
    }
  }


  initial_registers.find


  for (auto &MBB : MF) {
    for (auto &MI : MBB) {
      for (auto &MO : MI.explicit_operands()) {
        if ((MO.isReg() && entry.isLiveIn(MO.getReg()) && MO.isUse()))  {
          // printf("Find Livin 1\n");
          // MI.dump();
          taint_registrers[MO.getReg()] = MO.getReg();
          for (auto & MO_tmp : MI.defs() )
            taint_registrers[MO_tmp.getReg()] = MO_tmp.getReg();
        }else if (MO.isReg() && MO.isUse() && taint_registrers.count(MO.getReg())) {
          // printf("Find Livin 2\n");
          // MI.dump();
          for (auto & MO_tmp : MI.defs()) 
            taint_registrers[MO_tmp.getReg()] = MO_tmp.getReg();
        }
      }
    }
  }

  return taint_registrers;
#endif  
}




// TODO: Only print MI that has memory acces...
void X86MIRAnalyzePass::print_taintMI(MachineFunction &MF, SmallDenseMap<unsigned, unsigned, 32> taint_registers, SmallVector<MachineInstr *> taint_flagsets)
{
  // We need to store all MBBs here and later we check their dependency
  SmallDenseMap<MachineBasicBlock *, int> taint_MBBs;
  for (MachineInstr *MI : taint_flagsets) {
    MachineBasicBlock *MBB = MI->getParent();
    if (!taint_MBBs.count(MBB))
      ++taint_MBBs[MBB];
  }

  for (MachineInstr *MI : taint_flagsets) {
    MachineBasicBlock * MBB = MI->getParent();

    for (auto &MO : MI->explicit_operands()) {
      if (!MO.isReg())  continue;
      if (taint_registers.count(MO.getReg())) {
        //printf("+=+=+=+=+=+=\n");
        //MI->dump();
        // Check if has a taint predecessor
        for (auto &MBB_iter : MBB->predecessors()) {
          if (taint_MBBs.count(MBB_iter)) {
            std::cout << "Begin with ---->  " << MF.getName().str() << std::endl;
          #if 0
            // Printing this information does not help
            printf("******* *******\n");
            std::cout << "Found One: " << std::endl;
            std::cout << "Predecessor: " << std::endl;
            MBB_iter->dump();
            std::cout << "Successor: " << std::endl;
            MBB->dump();
            std::cout << "Nested branch to leak information: " << std::endl;
            MI->dump();
            printf("******* *******\n");
          #endif
            // Leave if we find one
            goto LEAVE_LOOP;
          }
        }
        //printf("Found a branch with taint variable\n");
        //printf("-=-=-=-=-=-=\n");
      }
      LEAVE_LOOP:
        break;
    }
  }
}

static MachineInstr* mayBranch(MachineInstr &MI) {
  if (!MI.isConditionalBranch())
    return nullptr;

  MachineBasicBlock &MBB = *MI.getParent();
  for (MachineInstr &MI_tmp : llvm::reverse(MBB)) {
    if (MI_tmp.isBranch())  continue;

    // We go through implict operands and get eflags status
    for (auto &MO : MI_tmp.implicit_operands()) {
      if (!MO.isReg() || !MO.isDef() || (MO.getReg() != X86::EFLAGS)) 
        continue;
    }
    
    return (MachineInstr*)&MI_tmp;
  }
  return nullptr;
}

SmallVector<MachineInstr *> X86MIRAnalyzePass::track_flagsets(MachineFunction &MF)
{
  SmallVector<MachineInstr *> track_flagset_MIs;
  for (auto &MBB : MF) {
    for (auto &MI : MBB) {
      MachineInstr * MI_condition = mayBranch(MI);
      if (MI_condition != nullptr) {
        //MI_condition->dump();
        track_flagset_MIs.push_back(*&MI_condition);
      }
    }
  }
  return track_flagset_MIs;
}

int X86MIRAnalyzePass::num_predecessors(MachineBasicBlock *MBB)
{
  int count = 0;
  for (auto &MBB_iter : MBB->predecessors())
    count += 1;
  return count;
}


INITIALIZE_PASS_BEGIN(X86MIRAnalyzePass, PASS_KEY,
                      "X86 MachineIR Analyser", false, false)
INITIALIZE_PASS_END(X86MIRAnalyzePass, PASS_KEY,
                    "X86 MachineIR Analyser", false, false)

FunctionPass *llvm::createX86MIRAnalyzePass() {
  return new X86MIRAnalyzePass();
}
