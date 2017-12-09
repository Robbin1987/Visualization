#include "llvm/IR/CallSite.h"
#include "llvm/IR/InstIterator.h"

#include "llvm/IR/IRBuilder.h"

#include "boost/range.hpp"
#include "avy/AvyDebug.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace 
{
  class PromoteMalloc : public FunctionPass
  {
  public:
    static char ID;
    
    PromoteMalloc () : FunctionPass (ID) {} 

    bool runOnFunction (Function &F)
    {
      if (F.empty ()) return false;
      
      // -- only promote mallocs in top level functions
      if (!F.getName ().equals ("main")) return false;
      
      bool changed = false;

      SmallVector<Instruction*, 16> kill;
      
      for (auto &I : boost::make_iterator_range (inst_begin (F), inst_end (F)))
      {
        if (!isa<CallInst> (&I)) continue;

        Value *v = I.stripPointerCasts ();
        CallSite CS (v);
        
        const Function *fn = CS.getCalledFunction ();
        if (!fn && CS.getCalledValue ())
          fn = dyn_cast<const Function> (CS.getCalledValue ()->stripPointerCasts ());
        
        if (fn && fn->getName ().equals ("malloc"))
        {
          
          Value *nv = new AllocaInst (v->getType ()->getPointerElementType (),
                                      CS.getArgument (0), "malloc", &I);
          v->replaceAllUsesWith (nv);
          
          changed = true;
        }
        else if (fn && fn->getName ().equals ("free"))
          kill.push_back (&I);
      }
      
      // -- remove all calls to free(). This is too much, but ensures
      // -- that all promoted mallocs() are not free'ed by mistake
      for (auto *I : kill) I->eraseFromParent ();
      
      return changed;
    }

    void getAnalysisUsage (AnalysisUsage &AU) const
    {AU.setPreservesAll ();}
    
    virtual const char *getPassName () const {return "PromoteMalloc";}
    
  };

  char PromoteMalloc::ID = 0;
}

namespace seahorn
{
  Pass *createPromoteMallocPass () {return new PromoteMalloc ();} 
}

static llvm::RegisterPass<PromoteMalloc> 
X ("promote-malloc", "Promote top-level malloc calls to alloca");
