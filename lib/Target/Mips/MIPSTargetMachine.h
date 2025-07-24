//===- MIPSTargetMachine.h------------------------------------------------===//
// Part of the eld Project, under the BSD License
// See https://github.com/qualcomm/eld/LICENSE.txt for license information.
// SPDX-License-Identifier: BSD-3-Clause
//===----------------------------------------------------------------------===//

#ifndef MIPS_TARGET_MACHINE_H
#define MIPS_TARGET_MACHINE_H

#include "MIPS.h"
#include "eld/Target/TargetMachine.h"

namespace eld {

/** \class MIPSTargetMachine
 *  \brief Target machine for MIPS architecture
 */
class MIPSTargetMachine : public ELDTargetMachine {
public:
  MIPSTargetMachine(const llvm::Target &pLLVMTarget,
                    const eld::Target &pELDTarget, const std::string &pTriple);
};

} // namespace eld

#endif
