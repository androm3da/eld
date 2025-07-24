//===- MIPSTargetInfo.cpp-------------------------------------------------===//
// Part of the eld Project, under the BSD License
// See https://github.com/qualcomm/eld/LICENSE.txt for license information.
// SPDX-License-Identifier: BSD-3-Clause
//===----------------------------------------------------------------------===//

#include "eld/Support/Target.h"
#include "eld/Support/TargetRegistry.h"

namespace eld {

eld::Target TheMIPSTarget;
eld::Target TheMIPSLittleEndianTarget;

extern "C" void ELDInitializeMipsLDTargetInfo() {
  // register into eld::TargetRegistry
  eld::RegisterTarget<llvm::Triple::mips> X(TheMIPSTarget, "mips");
  eld::RegisterTarget<llvm::Triple::mipsel> Y(TheMIPSLittleEndianTarget,
                                              "mipsel");
}

} // namespace eld
