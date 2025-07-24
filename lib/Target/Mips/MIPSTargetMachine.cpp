//===- MIPSTargetMachine.cpp----------------------------------------------===//
// Part of the eld Project, under the BSD License
// See https://github.com/qualcomm/eld/LICENSE.txt for license information.
// SPDX-License-Identifier: BSD-3-Clause
//===----------------------------------------------------------------------===//

#include "MIPSTargetMachine.h"
#include "MIPS.h"
#include "eld/Support/TargetRegistry.h"

extern "C" void ELDInitializeMipsLDTarget() {
  // Register createTargetMachine function pointer to eld::Target
  eld::RegisterTargetMachine<eld::MIPSTargetMachine> X(eld::TheMIPSTarget);
}

using namespace eld;

//===----------------------------------------------------------------------===//
// MIPSTargetMachine
//===----------------------------------------------------------------------===//
MIPSTargetMachine::MIPSTargetMachine(const llvm::Target &pLLVMTarget,
                                     const eld::Target &pELDTarget,
                                     const std::string &pTriple)
    : ELDTargetMachine() {}
