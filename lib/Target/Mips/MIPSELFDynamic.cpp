//===- MIPSELFDynamic.cpp-------------------------------------------------===//
// Part of the eld Project, under the BSD License
// See https://github.com/qualcomm/eld/LICENSE.txt for license information.
// SPDX-License-Identifier: BSD-3-Clause
//===----------------------------------------------------------------------===//

#include "MIPSELFDynamic.h"
#include "MIPSLDBackend.h"
#include "eld/Target/ELFFileFormat.h"
#include "llvm/BinaryFormat/ELF.h"

using namespace eld;

MIPSELFDynamic::MIPSELFDynamic(GNULDBackend &pParent, LinkerConfig &pConfig)
    : ELFDynamic(pParent, pConfig) {}

MIPSELFDynamic::~MIPSELFDynamic() {}

void MIPSELFDynamic::reserveTargetEntries() {
  // Reserve space for MIPS-specific dynamic entries
  reserveOne(llvm::ELF::DT_RELCOUNT);

  // MIPS-specific dynamic entries that may be needed:
  // DT_MIPS_RLD_VERSION, DT_MIPS_FLAGS, DT_MIPS_BASE_ADDRESS, etc.
  // For now, just reserve the basic ones
}

void MIPSELFDynamic::applyTargetEntries() {
  // Apply MIPS-specific dynamic entries
  uint32_t relCount = 0;

  // Count relative relocations (MIPS uses R_MIPS_REL32 for relative relocs)
  if (m_Backend.getRelaDyn()) {
    for (auto &R : m_Backend.getRelaDyn()->getRelocations()) {
      if (R->type() == llvm::ELF::R_MIPS_REL32)
        relCount++;
    }
  }

  applyOne(llvm::ELF::DT_RELCOUNT, relCount);
}