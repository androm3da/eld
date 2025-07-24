//===- MIPSRelocator.h----------------------------------------------------===//
// Part of the eld Project, under the BSD License
// See https://github.com/qualcomm/eld/LICENSE.txt for license information.
// SPDX-License-Identifier: BSD-3-Clause
//===----------------------------------------------------------------------===//

#ifndef MIPS_RELOCATOR_H
#define MIPS_RELOCATOR_H

#include "eld/Readers/Relocation.h"
#include "eld/Target/Relocator.h"
#include "llvm/ADT/DenseMap.h"

namespace eld {

class MIPSLDBackend;
class ResolveInfo;

/** \class MIPSRelocator
 *  \brief MIPSRelocator creates and performs the MIPS relocations.
 */
class MIPSRelocator : public Relocator {
public:
  /// Constructor
  MIPSRelocator(MIPSLDBackend &pParent, LinkerConfig &pConfig, Module &pModule);

  /// Destructor
  ~MIPSRelocator();

  /// Apply relocation
  Result applyRelocation(Relocation &pRelocation) override;

  /// scanRelocation - determine the empty entries are needed or not and
  /// create the empty entries if needed.
  void scanRelocation(Relocation &pReloc, IRBuilder &pBuilder,
                      ELFSection &pSection, InputFile &pInput,
                      CopyRelocs &pCopyRelocs) override;

  /// Name for debugging and profiling
  const char *getName(Relocation::Type pType) const override;

  /// Size of relocation in bits
  Size getSize(Relocation::Type pType) const override;

  /// Whether a relocation should be issued for the specific symbol
  bool mayHaveFunctionPointerAccess(const Relocation &pReloc) const;

  /// MIPS-specific relocation processing
  bool isHI16Relocation(unsigned int pType) const;
  bool isLO16Relocation(unsigned int pType) const;
  bool isGPRelRelocation(unsigned int pType) const;

private:
  /// Parent LDBackend
  MIPSLDBackend &m_pParent;

  /// HI16/LO16 pairing management
  struct HI16RelocEntry {
    Relocation *reloc;
    uint64_t addend;
  };
  llvm::DenseMap<ResolveInfo *, std::vector<HI16RelocEntry>> m_PendingHI16;

  /// Scan methods for relocation processing
  void scanLocalReloc(InputFile &pInput, Relocation &pReloc,
                      IRBuilder &pBuilder, ELFSection &pSection);
  void scanGlobalReloc(InputFile &pInput, Relocation &pReloc,
                       IRBuilder &pBuilder, ELFSection &pSection,
                       CopyRelocs &pCopyRelocs);

  /// Helper methods
  MIPSRelocator::Result relocAbs(Relocation &pReloc, int64_t pAddend);
  MIPSRelocator::Result relocRel(Relocation &pReloc, int64_t pAddend);
  MIPSRelocator::Result relocJump26(Relocation &pReloc, int64_t pAddend);
  MIPSRelocator::Result relocHI16(Relocation &pReloc, int64_t pAddend);
  MIPSRelocator::Result relocLO16(Relocation &pReloc, int64_t pAddend);
  MIPSRelocator::Result relocGPRel16(Relocation &pReloc, int64_t pAddend);
  MIPSRelocator::Result relocGOT16(Relocation &pReloc, int64_t pAddend);
  MIPSRelocator::Result relocCall16(Relocation &pReloc, int64_t pAddend);

  /// Calculate combined HI16/LO16 value
  int64_t calculateHI16LO16Value(int64_t addr, int64_t addend_hi,
                                 int64_t addend_lo);

  /// Check relocation overflow
  bool checkOverflow(unsigned int pType, int64_t pValue) const;

  /// Get target backend
  GNULDBackend &getTarget() override;
  const GNULDBackend &getTarget() const override;

  /// Get number of relocations
  uint32_t getNumRelocs() const override { return 0; }
};

} // namespace eld

#endif
