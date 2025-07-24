//===- MIPSLDBackend.h----------------------------------------------------===//
// Part of the eld Project, under the BSD License
// See https://github.com/qualcomm/eld/LICENSE.txt for license information.
// SPDX-License-Identifier: BSD-3-Clause
//===----------------------------------------------------------------------===//

#ifndef MIPS_LDBACKEND_H
#define MIPS_LDBACKEND_H

#include "eld/Config/LinkerConfig.h"
#include "eld/Fragment/GOT.h"
#include "eld/Object/ObjectBuilder.h"
#include "eld/Readers/ELFSection.h"
#include "eld/SymbolResolver/IRBuilder.h"
#include "eld/Target/GNULDBackend.h"
#include "llvm/ADT/DenseMap.h"

namespace eld {

class ELFDynamic;
class LinkerConfig;
class MIPSInfo;
class MIPSGOT;
class MIPSPLT;

//===----------------------------------------------------------------------===//
/// MIPSLDBackend - linker backend of MIPS target of GNU ELF format
///
class MIPSLDBackend : public GNULDBackend {
public:
  MIPSLDBackend(Module &pModule, MIPSInfo *pInfo);

  ~MIPSLDBackend();

  void initializeAttributes() override;

  /// initRelocator - create and initialize Relocator.
  bool initRelocator() override;

  /// getRelocator - return relocator.
  Relocator *getRelocator() const override;

  void initTargetSections(ObjectBuilder &pBuilder) override;

  void initTargetSymbols() override;

  bool initBRIslandFactory() override;

  bool initStubFactory() override;

  /// getTargetSectionOrder - compute the layout order of target section
  unsigned int getTargetSectionOrder(const ELFSection &pSectHdr) const override;

  /// finalizeTargetSymbols - finalize the symbol value
  bool finalizeTargetSymbols() override;

  uint64_t getValueForDiscardedRelocations(const Relocation *R) const override;

  ELFDynamic *dynamic() override;

  void doCreateProgramHdrs() override;

  /// doPreLayout - do MIPS-specific tasks before layout
  void doPreLayout() override;

  /// readSection - read target dependent sections
  bool readSection(InputFile &pInput, ELFSection *pInputSectHdr) override;

  /// MIPS-specific GOT/PLT management
  MIPSGOT *createGOT(GOT::GOTType type, ELFObjectFile *pObj, ResolveInfo *pSym);
  MIPSPLT *createPLT(ELFObjectFile *pObj, ResolveInfo *pSym);

  /// GP (Global Pointer) management
  bool hasGP() const { return true; }
  uint64_t getGPValue() const { return m_GPValue; }
  void setGPValue(uint64_t pValue) { m_GPValue = pValue; }

  /// Set default configuration values
  void setDefaultConfigs() override;

private:
  /// getRelEntrySize - the size in BYTE of rel type relocation
  size_t getRelEntrySize() override { return 8; }

  /// getRelaEntrySize - the size in BYTE of rela type relocation
  size_t getRelaEntrySize() override { return 12; }

  uint64_t maxBranchOffset() override {
    // MIPS 26-bit jump instruction has 28-bit range (shifted by 2)
    return 0x0FFFFFC;
  }

  /// MIPS doesn't use branch island stubs
  Stub *getBranchIslandStub(Relocation *pReloc,
                            int64_t pTargetValue) const override {
    return nullptr;
  }

  /// Number of PLT entries
  std::size_t PLTEntriesCount() const override { return m_PLTMap.size(); }

  /// Number of GOT entries
  std::size_t GOTEntriesCount() const override { return m_GOTMap.size(); }

  /// GOT Support
  void recordGOT(ResolveInfo *pInfo, MIPSGOT *pGOT);
  void recordGOTPLT(ResolveInfo *pInfo, MIPSGOT *pGOT);
  MIPSGOT *findEntryInGOT(ResolveInfo *pInfo) const;

  /// PLT Support
  void recordPLT(ResolveInfo *pInfo, MIPSPLT *pPLT);
  MIPSPLT *findEntryInPLT(ResolveInfo *pInfo) const;

private:
  Relocator *m_pRelocator;
  ELFDynamic *m_pDynamic;
  llvm::BumpPtrAllocator _alloc;

  LDSymbol *m_pEndOfImage;
  LDSymbol *m_pGlobalPointer;
  uint64_t m_GPValue;

  // MIPS-specific sections
  Fragment *m_pGOT;
  Fragment *m_pPLT;

  // GOT and PLT maps for tracking entries
  llvm::DenseMap<ResolveInfo *, MIPSGOT *> m_GOTMap;
  llvm::DenseMap<ResolveInfo *, MIPSGOT *> m_GOTPLTMap;
  llvm::DenseMap<ResolveInfo *, MIPSPLT *> m_PLTMap;
};

} // namespace eld

#endif
