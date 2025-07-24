//===- MIPSLDBackend.cpp--------------------------------------------------===//
// Part of the eld Project, under the BSD License
// See https://github.com/qualcomm/eld/LICENSE.txt for license information.
// SPDX-License-Identifier: BSD-3-Clause
//===----------------------------------------------------------------------===//

#include "MIPSLDBackend.h"
#include "MIPS.h"
#include "MIPSELFDynamic.h"
#include "MIPSGOT.h"
#include "MIPSInfo.h"
#include "MIPSPLT.h"
#include "MIPSRelocator.h"
#include "eld/Config/LinkerConfig.h"
#include "eld/Fragment/FillFragment.h"
#include "eld/Fragment/RegionFragment.h"
#include "eld/Fragment/Stub.h"
#include "eld/Input/ELFObjectFile.h"
#include "eld/Object/ObjectBuilder.h"
#include "eld/Object/ObjectLinker.h"
#include "eld/Readers/Relocation.h"
#include "eld/Support/MemoryArea.h"
#include "eld/Support/MsgHandling.h"
#include "eld/Support/RegisterTimer.h"
#include "eld/Support/TargetRegistry.h"
#include "eld/SymbolResolver/IRBuilder.h"
#include "eld/Target/ELFFileFormat.h"
#include "eld/Target/ELFSegmentFactory.h"
#include "llvm/ADT/Hashing.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/Memory.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Program.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/TargetParser/Triple.h"
#include <string>

using namespace eld;
using namespace llvm;

//===----------------------------------------------------------------------===//
// MIPSLDBackend
//===----------------------------------------------------------------------===//
MIPSLDBackend::MIPSLDBackend(Module &pModule, MIPSInfo *pInfo)
    : GNULDBackend(pModule, pInfo), m_pRelocator(nullptr), m_pDynamic(nullptr),
      m_pEndOfImage(nullptr), m_pGlobalPointer(nullptr), m_GPValue(0),
      m_pGOT(nullptr), m_pPLT(nullptr) {}

MIPSLDBackend::~MIPSLDBackend() {
  delete m_pRelocator;
  delete m_pDynamic;
}

void MIPSLDBackend::initializeAttributes() {
  // MIPS-specific initialization
}

bool MIPSLDBackend::initRelocator() {
  if (nullptr == m_pRelocator) {
    m_pRelocator = new MIPSRelocator(*this, config(), m_Module);
  }
  return true;
}

Relocator *MIPSLDBackend::getRelocator() const {
  assert(nullptr != m_pRelocator);
  return m_pRelocator;
}

unsigned int
MIPSLDBackend::getTargetSectionOrder(const ELFSection &pSectHdr) const {
  if (m_Module.getScript().linkerScriptHasSectionsCommand())
    return SHO_UNDEFINED;

  if (LinkerConfig::Object != config().codeGenType()) {
    if (pSectHdr.name() == ".got") {
      return SHO_DATA;
    }

    if (pSectHdr.name() == ".plt") {
      return SHO_PLT;
    }
  }

  return SHO_UNDEFINED;
}

void MIPSLDBackend::initTargetSections(ObjectBuilder &pBuilder) {
  // Initialize MIPS-specific sections when doing dynamic linking
  if (config().isCodeStatic() && !config().options().forceDynamic())
    return;

  // Create dynamic section for dynamic linking
  if (nullptr == m_pDynamic)
    m_pDynamic = new MIPSELFDynamic(*this, config());

  // TODO: Create .got section for MIPS
  // TODO: Create .plt section for MIPS
  // TODO: Create .MIPS.stubs section if needed
}

void MIPSLDBackend::initTargetSymbols() {
  if (config().codeGenType() == LinkerConfig::Object)
    return;

  // Create __end symbol
  m_pEndOfImage =
      m_Module.getIRBuilder()->addSymbol<IRBuilder::Force, IRBuilder::Resolve>(
          m_Module.getInternalInput(Module::Script), "__end",
          ResolveInfo::NoType, ResolveInfo::Define, ResolveInfo::Absolute,
          0x0, // size
          0x0, // value
          FragmentRef::null());
  if (m_pEndOfImage)
    m_pEndOfImage->setShouldIgnore(false);

  // Create _gp symbol (Global Pointer)
  m_pGlobalPointer =
      m_Module.getIRBuilder()->addSymbol<IRBuilder::Force, IRBuilder::Resolve>(
          m_Module.getInternalInput(Module::Script), "_gp", ResolveInfo::NoType,
          ResolveInfo::Define, ResolveInfo::Absolute,
          0x0, // size
          0x0, // value
          FragmentRef::null());
  if (m_pGlobalPointer)
    m_pGlobalPointer->setShouldIgnore(false);
}

bool MIPSLDBackend::initBRIslandFactory() {
  // MIPS doesn't need branch islands for basic functionality
  return true;
}

bool MIPSLDBackend::initStubFactory() {
  // MIPS doesn't need stubs for basic functionality
  return true;
}

/// finalizeSymbol - finalize the symbol value
bool MIPSLDBackend::finalizeTargetSymbols() {
  if (config().codeGenType() == LinkerConfig::Object)
    return true;

  // Get the pointer to the real end of the image.
  if (m_pEndOfImage && !m_pEndOfImage->scriptDefined()) {
    uint64_t imageEnd = 0;
    for (auto &seg : elfSegmentTable()) {
      if (seg->type() != llvm::ELF::PT_LOAD)
        continue;
      uint64_t segSz = seg->paddr() + seg->memsz();
      if (imageEnd < segSz)
        imageEnd = segSz;
    }

    m_pEndOfImage->setValue(imageEnd);
  }

  // TODO: Calculate Global Pointer value properly
  // For now, just set a default GP value
  m_GPValue = 0x10000;

  return true;
}

uint64_t
MIPSLDBackend::getValueForDiscardedRelocations(const Relocation *R) const {
  // Return 0 for discarded relocations
  return 0;
}

ELFDynamic *MIPSLDBackend::dynamic() {
  if (!m_pDynamic)
    m_pDynamic = new MIPSELFDynamic(*this, config());
  return m_pDynamic;
}

void MIPSLDBackend::doCreateProgramHdrs() {
  // Create MIPS-specific program headers
  return;
}

void MIPSLDBackend::doPreLayout() {
  // Only handle dynamic sections if we're doing dynamic linking
  if (config().isCodeStatic() && !config().options().forceDynamic())
    return;

  // Perform MIPS-specific pre-layout tasks
  // This method sizes and adds dynamic relocation sections if they exist

  // Handle .rela.plt section
  if (getRelaPLT()) {
    getRelaPLT()->setSize(getRelaPLT()->getRelocations().size() *
                          getRelaEntrySize());
    m_Module.addOutputSection(getRelaPLT());
  }

  // Handle .rela.dyn section
  if (getRelaDyn()) {
    getRelaDyn()->setSize(getRelaDyn()->getRelocations().size() *
                          getRelaEntrySize());
    m_Module.addOutputSection(getRelaDyn());
  }
}

bool MIPSLDBackend::readSection(InputFile &pInput, ELFSection *pInputSectHdr) {
  if (!pInputSectHdr)
    return false;

  switch (pInputSectHdr->getType()) {
  case llvm::ELF::SHT_MIPS_REGINFO:
    // MIPS register usage information section
    // For now, just accept it - full implementation would parse and merge
    // reginfo
    return true;
  case llvm::ELF::SHT_MIPS_OPTIONS:
    // MIPS options section
    // For now, just accept it - full implementation would parse MIPS options
    return true;
  case llvm::ELF::SHT_MIPS_ABIFLAGS:
    // MIPS ABI flags section (for newer MIPS ABIs)
    // For now, just accept it - full implementation would parse ABI flags
    return true;
  default:
    // For other sections, use the default implementation
    return GNULDBackend::readSection(pInput, pInputSectHdr);
  }
}

MIPSGOT *MIPSLDBackend::createGOT(GOT::GOTType type, ELFObjectFile *pObj,
                                  ResolveInfo *pSym) {
  // Symbol tracing support
  if (pSym != nullptr && ((config().options().isSymbolTracingRequested() &&
                           config().options().traceSymbol(*pSym)) ||
                          m_Module.getPrinter()->traceDynamicLinking()))
    config().raise(Diag::create_got_entry) << pSym->name();

  // Always create .got.plt if creating a GOT
  if (!getGOTPLT()->getFragmentList().size()) {
    LDSymbol *Dynamic = m_Module.getNamePool().findSymbol("_DYNAMIC");
    // Create basic GOTPLT0 entry - for now use regular MIPSGOT
    make<MIPSGOT>(GOT::GOTPLT0, getGOTPLT(),
                  Dynamic ? Dynamic->resolveInfo() : nullptr);
  }

  MIPSGOT *got = nullptr;
  bool isGOT = true;

  switch (type) {
  case GOT::Regular:
    got = MIPSGOT::Create(pObj->getGOT(), pSym);
    break;
  case GOT::GOTPLT0:
    // Return the first fragment in GOTPLT as GOTPLT0
    got = llvm::dyn_cast<MIPSGOT>(*getGOTPLT()->getFragmentList().begin());
    isGOT = false;
    break;
  case GOT::GOTPLTN:
    // Create GOTPLT entry for PLT - for now use regular MIPSGOT
    got = make<MIPSGOT>(GOT::GOTPLTN, pObj->getGOTPLT(), pSym);
    isGOT = false;
    break;
  case GOT::TLS_GD:
    // TLS Global Dynamic - use regular MIPSGOT for now
    got = MIPSGOT::Create(pObj->getGOT(), pSym);
    break;
  case GOT::TLS_LD:
    // TLS Local Dynamic - use regular MIPSGOT for now
    got = MIPSGOT::Create(pObj->getGOT(), pSym);
    break;
  case GOT::TLS_LE:
    // TLS Local Exec - use regular MIPSGOT for now
    got = MIPSGOT::Create(pObj->getGOT(), pSym);
    break;
  default:
    // Unknown GOT type
    return nullptr;
  }

  // Record the GOT entry for later lookup
  if (pSym) {
    if (isGOT)
      recordGOT(pSym, got);
    else
      recordGOTPLT(pSym, got);
  }

  return got;
}

MIPSPLT *MIPSLDBackend::createPLT(ELFObjectFile *pObj, ResolveInfo *pSym) {
  // Symbol tracing support
  if (pSym != nullptr && ((config().options().isSymbolTracingRequested() &&
                           config().options().traceSymbol(*pSym)) ||
                          m_Module.getPrinter()->traceDynamicLinking()))
    config().raise(Diag::create_plt_entry) << pSym->name();

  // Create PLT0 if this is the first PLT entry
  if (!getPLT()->getFragmentList().size()) {
    // Create PLT0 entry - for now use regular MIPSPLT with GOTPLT0
    MIPSGOT *gotplt0 = createGOT(GOT::GOTPLT0, nullptr, nullptr);
    make<MIPSPLT>(PLT::PLT0, *m_Module.getIRBuilder(), gotplt0, getPLT(),
                  nullptr, 4, 16);
  }

  // Create PLT entry with associated GOT entry
  MIPSGOT *gotEntry = createGOT(GOT::GOTPLTN, pObj, pSym);
  MIPSPLT *pltEntry = make<MIPSPLT>(PLT::PLTN, *m_Module.getIRBuilder(),
                                    gotEntry, getPLT(), pSym, 4, 16);

  // Create corresponding relocation entry in .rela.plt
  Relocation *relEntry = pObj->getRelaPLT()->createOneReloc();
  relEntry->setType(
      llvm::ELF::R_MIPS_JUMP_SLOT); // MIPS-specific PLT relocation type
  relEntry->setTargetRef(make<FragmentRef>(*gotEntry, 0));
  relEntry->setSymInfo(pSym);

  // Record PLT entry for later lookup
  if (pSym)
    recordPLT(pSym, pltEntry);

  return pltEntry;
}

//===----------------------------------------------------------------------===//
// GOT and PLT Management
//===----------------------------------------------------------------------===//
void MIPSLDBackend::recordGOT(ResolveInfo *pInfo, MIPSGOT *pGOT) {
  m_GOTMap[pInfo] = pGOT;
}

void MIPSLDBackend::recordGOTPLT(ResolveInfo *pInfo, MIPSGOT *pGOT) {
  m_GOTPLTMap[pInfo] = pGOT;
}

MIPSGOT *MIPSLDBackend::findEntryInGOT(ResolveInfo *pInfo) const {
  auto Entry = m_GOTMap.find(pInfo);
  if (Entry == m_GOTMap.end())
    return nullptr;
  return Entry->second;
}

void MIPSLDBackend::recordPLT(ResolveInfo *pInfo, MIPSPLT *pPLT) {
  m_PLTMap[pInfo] = pPLT;
}

MIPSPLT *MIPSLDBackend::findEntryInPLT(ResolveInfo *pInfo) const {
  auto Entry = m_PLTMap.find(pInfo);
  if (Entry == m_PLTMap.end())
    return nullptr;
  return Entry->second;
}

void MIPSLDBackend::setDefaultConfigs() {
  // Set MIPS-specific default configuration values
  // For now, minimal implementation similar to x86_64
  if (config().options().threadsEnabled() &&
      !config().isGlobalThreadingEnabled()) {
    config().disableThreadOptions(LinkerConfig::EnableThreadsOpt::AllThreads);
  }
}

//===----------------------------------------------------------------------===//
/// createMIPSLDBackend - the help function to create corresponding
/// MIPSLDBackend
//===----------------------------------------------------------------------===//
namespace eld {

GNULDBackend *createMIPSLDBackend(Module &pModule) {
  return make<MIPSLDBackend>(pModule, make<MIPSInfo>(pModule.getConfig()));
}

} // namespace eld

//===----------------------------------------------------------------------===//
// Force static initialization.
//===----------------------------------------------------------------------===//
extern "C" void ELDInitializeMipsLDBackend() {
  // Register the linker backend
  eld::TargetRegistry::RegisterGNULDBackend(eld::TheMIPSTarget,
                                            eld::createMIPSLDBackend);
}
