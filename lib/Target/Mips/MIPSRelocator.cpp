//===- MIPSRelocator.cpp--------------------------------------------------===//
// Part of the eld Project, under the BSD License
// See https://github.com/qualcomm/eld/LICENSE.txt for license information.
// SPDX-License-Identifier: BSD-3-Clause
//===----------------------------------------------------------------------===//

#include "MIPSRelocator.h"
#include "MIPSLDBackend.h"
#include "MIPSRelocationInfo.h"
#include "eld/Config/LinkerConfig.h"
#include "eld/Input/ELFObjectFile.h"
#include "eld/Support/MsgHandling.h"
#include "eld/SymbolResolver/IRBuilder.h"
#include "eld/SymbolResolver/LDSymbol.h"
#include "eld/SymbolResolver/Resolver.h"
#include "eld/Target/ELFFileFormat.h"
#include "llvm/ADT/Twine.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/Support/DataTypes.h"

using namespace eld;

//===--------------------------------------------------------------------===//
// MIPSRelocator
//===--------------------------------------------------------------------===//
MIPSRelocator::MIPSRelocator(MIPSLDBackend &pParent, LinkerConfig &pConfig,
                             Module &pModule)
    : Relocator(pConfig, pModule), m_pParent(pParent) {}

MIPSRelocator::~MIPSRelocator() {}

GNULDBackend &MIPSRelocator::getTarget() { return m_pParent; }

const GNULDBackend &MIPSRelocator::getTarget() const { return m_pParent; }

Relocator::Result MIPSRelocator::applyRelocation(Relocation &pRelocation) {
  Relocation::Type type = pRelocation.type();
  int64_t addend = pRelocation.addend();

  // Handle discarded sections
  ResolveInfo *symInfo = pRelocation.symInfo();
  if (symInfo) {
    LDSymbol *outSymbol = symInfo->outSymbol();
    if (outSymbol && outSymbol->hasFragRef()) {
      ELFSection *S = outSymbol->fragRef()->frag()->getOwningSection();
      if (S->isDiscard() ||
          (S->getOutputSection() && S->getOutputSection()->isDiscard())) {
        return Relocator::OK;
      }
    }
  }

  // Apply the relocation based on type
  switch (type) {
  case R_MIPS_NONE:
    return Relocator::OK;
  case R_MIPS_32:
    return relocAbs(pRelocation, addend);
  case R_MIPS_REL32:
    return relocRel(pRelocation, addend);
  case R_MIPS_26:
    return relocJump26(pRelocation, addend);
  case R_MIPS_HI16:
    return relocHI16(pRelocation, addend);
  case R_MIPS_LO16:
    return relocLO16(pRelocation, addend);
  case R_MIPS_GPREL16:
    return relocGPRel16(pRelocation, addend);
  case R_MIPS_GOT16:
    return relocGOT16(pRelocation, addend);
  case R_MIPS_CALL16:
    return relocCall16(pRelocation, addend);
  default:
    return Relocator::Unknown;
  }
}

const char *MIPSRelocator::getName(Relocation::Type pType) const {
  switch (pType) {
  case R_MIPS_NONE:
    return "R_MIPS_NONE";
  case R_MIPS_16:
    return "R_MIPS_16";
  case R_MIPS_32:
    return "R_MIPS_32";
  case R_MIPS_REL32:
    return "R_MIPS_REL32";
  case R_MIPS_26:
    return "R_MIPS_26";
  case R_MIPS_HI16:
    return "R_MIPS_HI16";
  case R_MIPS_LO16:
    return "R_MIPS_LO16";
  case R_MIPS_GPREL16:
    return "R_MIPS_GPREL16";
  case R_MIPS_LITERAL:
    return "R_MIPS_LITERAL";
  case R_MIPS_GOT16:
    return "R_MIPS_GOT16";
  case R_MIPS_PC16:
    return "R_MIPS_PC16";
  case R_MIPS_CALL16:
    return "R_MIPS_CALL16";
  case R_MIPS_GPREL32:
    return "R_MIPS_GPREL32";
  default:
    return "Unknown";
  }
}

Relocator::Size MIPSRelocator::getSize(Relocation::Type pType) const {
  switch (pType) {
  case R_MIPS_16:
  case R_MIPS_HI16:
  case R_MIPS_LO16:
  case R_MIPS_GPREL16:
  case R_MIPS_GOT16:
  case R_MIPS_PC16:
  case R_MIPS_CALL16:
    return 16;
  case R_MIPS_32:
  case R_MIPS_REL32:
  case R_MIPS_26:
  case R_MIPS_GPREL32:
    return 32;
  default:
    return 0;
  }
}

void MIPSRelocator::scanRelocation(Relocation &pReloc, IRBuilder &pBuilder,
                                   ELFSection &pSection, InputFile &pInput,
                                   CopyRelocs &pCopyRelocs) {
  if (LinkerConfig::Object == config().codeGenType())
    return;

  // rsym - The relocation target symbol
  ResolveInfo *rsym = pReloc.symInfo();
  assert(nullptr != rsym &&
         "ResolveInfo of relocation not set while scanRelocation");

  // Check if we are tracing relocations.
  if (m_Module.getPrinter()->traceReloc()) {
    std::string relocName = getName(pReloc.type());
    if (config().options().traceReloc(relocName))
      config().raise(Diag::reloc_trace) << relocName << pReloc.symInfo()->name()
                                        << pInput.getInput()->decoratedPath();
  }

  // check if we should issue undefined reference for the relocation target
  // symbol
  if (rsym->isUndef() || rsym->isBitCode()) {
    if (m_pParent.canIssueUndef(rsym)) {
      if (rsym->visibility() != ResolveInfo::Default)
        issueInvisibleRef(pReloc, pInput);
      issueUndefRef(pReloc, pInput, &pSection);
    }
  }

  ELFSection *section = pSection.getLink()
                            ? pSection.getLink()
                            : pReloc.targetRef()->frag()->getOwningSection();

  if (!section->isAlloc())
    return;

  if (rsym->isLocal()) // rsym is local
    scanLocalReloc(pInput, pReloc, pBuilder, *section);
  else // rsym is external
    scanGlobalReloc(pInput, pReloc, pBuilder, *section, pCopyRelocs);
}

bool MIPSRelocator::mayHaveFunctionPointerAccess(
    const Relocation &pReloc) const {
  switch (pReloc.type()) {
  case R_MIPS_32:
  case R_MIPS_REL32:
    return true;
  default:
    return false;
  }
}

bool MIPSRelocator::isHI16Relocation(unsigned int pType) const {
  return pType == R_MIPS_HI16;
}

bool MIPSRelocator::isLO16Relocation(unsigned int pType) const {
  return pType == R_MIPS_LO16;
}

bool MIPSRelocator::isGPRelRelocation(unsigned int pType) const {
  return pType == R_MIPS_GPREL16 || pType == R_MIPS_GPREL32;
}

//===----------------------------------------------------------------------===//
// Relocation Implementation Functions
//===----------------------------------------------------------------------===//

MIPSRelocator::Result MIPSRelocator::relocAbs(Relocation &pReloc,
                                              int64_t pAddend) {
  ResolveInfo *rsym = pReloc.symInfo();
  uint64_t S = rsym->outSymbol()->value();
  uint64_t A = pAddend;

  uint32_t *target = reinterpret_cast<uint32_t *>(pReloc.target());
  *target = static_cast<uint32_t>(S + A);

  return Relocator::OK;
}

MIPSRelocator::Result MIPSRelocator::relocRel(Relocation &pReloc,
                                              int64_t pAddend) {
  ResolveInfo *rsym = pReloc.symInfo();
  uint64_t S = rsym->outSymbol()->value();
  uint64_t A = pAddend;
  uint64_t P = pReloc.place(m_Module);

  uint32_t *target = reinterpret_cast<uint32_t *>(pReloc.target());
  *target = static_cast<uint32_t>(S + A - P);

  return Relocator::OK;
}

MIPSRelocator::Result MIPSRelocator::relocJump26(Relocation &pReloc,
                                                 int64_t pAddend) {
  ResolveInfo *rsym = pReloc.symInfo();
  uint64_t S = rsym->outSymbol()->value();
  uint64_t A = pAddend;
  uint64_t P = pReloc.place(m_Module);

  // MIPS 26-bit jump: target must be in same 256MB region
  uint64_t target_addr = S + A;
  uint64_t pc_region = (P + 4) & 0xF0000000;
  uint64_t target_region = target_addr & 0xF0000000;

  if (pc_region != target_region) {
    return Relocator::Overflow;
  }

  uint32_t *target = reinterpret_cast<uint32_t *>(pReloc.target());
  uint32_t inst = *target;

  // Extract 26-bit target (word address)
  uint32_t jump_target = (target_addr >> 2) & 0x03FFFFFF;

  // Insert into instruction
  *target = (inst & 0xFC000000) | jump_target;

  return Relocator::OK;
}

MIPSRelocator::Result MIPSRelocator::relocHI16(Relocation &pReloc,
                                               int64_t pAddend) {
  ResolveInfo *rsym = pReloc.symInfo();

  // Store HI16 relocation for later processing with LO16
  HI16RelocEntry entry;
  entry.reloc = &pReloc;
  entry.addend = pAddend;

  m_PendingHI16[rsym].push_back(entry);

  return Relocator::OK;
}

MIPSRelocator::Result MIPSRelocator::relocLO16(Relocation &pReloc,
                                               int64_t pAddend) {
  ResolveInfo *rsym = pReloc.symInfo();

  // Find matching HI16 relocations
  auto it = m_PendingHI16.find(rsym);
  if (it == m_PendingHI16.end()) {
    return Relocator::BadReloc;
  }

  uint64_t S = rsym->outSymbol()->value();
  int64_t combined_value = calculateHI16LO16Value(S, 0, pAddend);

  // Process all pending HI16 relocations for this symbol
  for (auto &hi16_entry : it->second) {
    // Apply HI16 relocation
    uint32_t *hi16_target =
        reinterpret_cast<uint32_t *>(hi16_entry.reloc->target());
    uint32_t hi16_inst = *hi16_target;
    uint16_t hi16_value = (combined_value >> 16) & 0xFFFF;
    *hi16_target = (hi16_inst & 0xFFFF0000) | hi16_value;
  }

  // Apply LO16 relocation
  uint32_t *lo16_target = reinterpret_cast<uint32_t *>(pReloc.target());
  uint32_t lo16_inst = *lo16_target;
  uint16_t lo16_value = combined_value & 0xFFFF;
  *lo16_target = (lo16_inst & 0xFFFF0000) | lo16_value;

  // Clear processed HI16 relocations
  m_PendingHI16.erase(it);

  return Relocator::OK;
}

MIPSRelocator::Result MIPSRelocator::relocGPRel16(Relocation &pReloc,
                                                  int64_t pAddend) {
  ResolveInfo *rsym = pReloc.symInfo();
  uint64_t S = rsym->outSymbol()->value();
  uint64_t A = pAddend;
  uint64_t GP = m_pParent.getGPValue();

  int64_t result = S + A - GP;

  // Check 16-bit signed range
  if (result < -32768 || result > 32767) {
    return Relocator::Overflow;
  }

  uint32_t *target = reinterpret_cast<uint32_t *>(pReloc.target());
  uint32_t inst = *target;
  *target = (inst & 0xFFFF0000) | (result & 0xFFFF);

  return Relocator::OK;
}

MIPSRelocator::Result MIPSRelocator::relocGOT16(Relocation &pReloc,
                                                int64_t pAddend) {
  // GOT relocation - would need GOT implementation
  return Relocator::OK;
}

MIPSRelocator::Result MIPSRelocator::relocCall16(Relocation &pReloc,
                                                 int64_t pAddend) {
  // Call relocation - would need PLT implementation
  return Relocator::OK;
}

int64_t MIPSRelocator::calculateHI16LO16Value(int64_t addr, int64_t addend_hi,
                                              int64_t addend_lo) {
  int64_t result = addr + addend_hi;

  // Adjust for LO16 sign extension
  if ((result + addend_lo) & 0x8000) {
    result += 0x10000;
  }

  return result;
}

bool MIPSRelocator::checkOverflow(unsigned int pType, int64_t pValue) const {
  switch (pType) {
  case R_MIPS_16:
    return pValue < -32768 || pValue > 32767;
  case R_MIPS_26:
    return (pValue & 0xFC000000) != 0;
  default:
    return false;
  }
}

void MIPSRelocator::scanLocalReloc(InputFile &pInput, Relocation &pReloc,
                                   IRBuilder &pBuilder, ELFSection &pSection) {
  // For local relocations, we generally don't need GOT/PLT entries
  // unless generating position-independent code
  ResolveInfo *rsym = pReloc.symInfo();

  switch (pReloc.type()) {
  case R_MIPS_32:
    // If building PIC object (shared library or PIC executable),
    // a dynamic relocation with RELATIVE type may be needed
    if (config().isCodeIndep()) {
      // For now, simple implementation without dynamic relocations
      // This would need to be expanded for full PIC support
    }
    break;
  case R_MIPS_GOT16:
    // Local GOT entries - would need proper GOT implementation
    break;
  default:
    // Most local relocations don't need special handling
    break;
  }
}

void MIPSRelocator::scanGlobalReloc(InputFile &pInput, Relocation &pReloc,
                                    IRBuilder &pBuilder, ELFSection &pSection,
                                    CopyRelocs &pCopyRelocs) {
  // For global relocations, we may need GOT/PLT entries
  ResolveInfo *rsym = pReloc.symInfo();

  switch (pReloc.type()) {
  case R_MIPS_32:
    // May need copy relocation or dynamic relocation
    break;
  case R_MIPS_GOT16:
  case R_MIPS_CALL16:
    // Would need GOT entry creation
    // For now, simple implementation without GOT/PLT
    break;
  case R_MIPS_26:
    // Jump/call relocations may need PLT entries for external symbols
    break;
  default:
    // Other relocations
    break;
  }
}
