//===- MIPS.h-------------------------------------------------------------===//
// Part of the eld Project, under the BSD License
// See https://github.com/qualcomm/eld/LICENSE.txt for license information.
// SPDX-License-Identifier: BSD-3-Clause
//===----------------------------------------------------------------------===//

#ifndef ELD_TARGET_MIPS_H
#define ELD_TARGET_MIPS_H
#include <string>

namespace llvm {
class Target;
} // namespace llvm

namespace eld {

struct Target;
class GNULDBackend;
class LinkerScript;
class LinkerConfig;
class Module;

extern eld::Target TheMIPSTarget;

GNULDBackend *createMIPSLDBackend(Module &pModule);

bool emulateMIPSLD(LinkerScript &, LinkerConfig &);

} // namespace eld

#endif
