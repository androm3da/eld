//===- MIPSEmulation.cpp--------------------------------------------------===//
// Part of the eld Project, under the BSD License
// See https://github.com/qualcomm/eld/LICENSE.txt for license information.
// SPDX-License-Identifier: BSD-3-Clause
//===----------------------------------------------------------------------===//

#include "MIPS.h"
#include "eld/Config/LinkerConfig.h"
#include "eld/Core/LinkerScript.h"
#include "eld/Support/MsgHandling.h"
#include "eld/Support/TargetRegistry.h"
#include "eld/Target/ELFEmulation.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"

using namespace llvm;

namespace eld {

static bool ELDEmulateMIPSELF(LinkerScript &pScript, LinkerConfig &pConfig) {
  // set up bitclass and endian
  pConfig.targets().setEndian(TargetOptions::Little);
  pConfig.targets().setBitClass(32);

  if (LinkerConfig::DynObj == pConfig.codeGenType())
    pConfig.options().setGPSize(0);

  if (!ELDEmulateELF(pScript, pConfig))
    return false;

  return true;
}

//===----------------------------------------------------------------------===//
// emulateMIPSLD - the help function to emulate MIPS ld
//===----------------------------------------------------------------------===//
bool emulateMIPSLD(LinkerScript &pScript, LinkerConfig &pConfig) {
  return ELDEmulateMIPSELF(pScript, pConfig);
}

} // namespace eld

//===----------------------------------------------------------------------===//
// MIPSEmulation
//===----------------------------------------------------------------------===//
extern "C" void ELDInitializeMipsEmulation() {
  // Register the emulation
  eld::TargetRegistry::RegisterEmulation(eld::TheMIPSTarget,
                                         eld::emulateMIPSLD);
}
