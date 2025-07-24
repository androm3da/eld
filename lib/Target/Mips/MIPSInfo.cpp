//===- MIPSInfo.cpp-------------------------------------------------------===//
// Part of the eld Project, under the BSD License
// See https://github.com/qualcomm/eld/LICENSE.txt for license information.
// SPDX-License-Identifier: BSD-3-Clause
//===----------------------------------------------------------------------===//

#include "MIPSInfo.h"
#include "MIPSRelocationInfo.h"
#include "eld/Config/Config.h"
#include "eld/Support/MsgHandling.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/BinaryFormat/ELF.h"

using namespace eld;

#define UNKNOWN -1

// MIPS architecture constants for internal representation
enum MIPSArch {
  MIPS_ARCH_1 = 0,
  MIPS_ARCH_2,
  MIPS_ARCH_3,
  MIPS_ARCH_4,
  MIPS_ARCH_5,
  MIPS_ARCH_32,
  MIPS_ARCH_64,
  MIPS_ARCH_32R2,
  MIPS_ARCH_64R2
};

// Map ELF flags to internal arch representation
static const uint64_t ELFFlagToArch[] = {
    EF_MIPS_ARCH_1,    // MIPS_ARCH_1
    EF_MIPS_ARCH_2,    // MIPS_ARCH_2
    EF_MIPS_ARCH_3,    // MIPS_ARCH_3
    EF_MIPS_ARCH_4,    // MIPS_ARCH_4
    EF_MIPS_ARCH_5,    // MIPS_ARCH_5
    EF_MIPS_ARCH_32,   // MIPS_ARCH_32
    EF_MIPS_ARCH_64,   // MIPS_ARCH_64
    EF_MIPS_ARCH_32R2, // MIPS_ARCH_32R2
    EF_MIPS_ARCH_64R2  // MIPS_ARCH_64R2
};

static const char *ArchStrings[] = {
    "mips1",    // MIPS_ARCH_1
    "mips2",    // MIPS_ARCH_2
    "mips3",    // MIPS_ARCH_3
    "mips4",    // MIPS_ARCH_4
    "mips5",    // MIPS_ARCH_5
    "mips32",   // MIPS_ARCH_32
    "mips64",   // MIPS_ARCH_64
    "mips32r2", // MIPS_ARCH_32R2
    "mips64r2"  // MIPS_ARCH_64R2
};

static const char *MCPUStrings[] = {
    "mips1",    // MIPS_ARCH_1
    "mips2",    // MIPS_ARCH_2
    "mips3",    // MIPS_ARCH_3
    "mips4",    // MIPS_ARCH_4
    "mips5",    // MIPS_ARCH_5
    "mips32",   // MIPS_ARCH_32
    "mips64",   // MIPS_ARCH_64
    "mips32r2", // MIPS_ARCH_32R2
    "mips64r2"  // MIPS_ARCH_64R2
};

std::string MIPSInfo::flagString(uint64_t flag) const {
  uint64_t archFlag = flag & EF_MIPS_ARCH;
  switch (archFlag) {
  case EF_MIPS_ARCH_1:
    return "mips1";
  case EF_MIPS_ARCH_2:
    return "mips2";
  case EF_MIPS_ARCH_3:
    return "mips3";
  case EF_MIPS_ARCH_4:
    return "mips4";
  case EF_MIPS_ARCH_5:
    return "mips5";
  case EF_MIPS_ARCH_32:
    return "mips32";
  case EF_MIPS_ARCH_64:
    return "mips64";
  case EF_MIPS_ARCH_32R2:
    return "mips32r2";
  case EF_MIPS_ARCH_64R2:
    return "mips64r2";
  default:
    return "unknown";
  }
}

llvm::StringRef MIPSInfo::getOutputMCPU() const {
  uint64_t arch = translateFlag(flags());
  if (arch < sizeof(MCPUStrings) / sizeof(MCPUStrings[0]))
    return MCPUStrings[arch];
  return "mips32"; // Default fallback
}

//===----------------------------------------------------------------------===//
// MIPSInfo
//===----------------------------------------------------------------------===//
MIPSInfo::MIPSInfo(LinkerConfig &pConfig) : TargetInfo(pConfig) {
  // Initialize with MIPS32 as default
  m_CmdLineFlag = UNKNOWN;
  m_OutputFlag = UNKNOWN;
}

bool MIPSInfo::initialize() {
  if (m_Config.targets().getTargetCPU().empty()) {
    // Default to MIPS32 if no specific CPU is set
    m_CmdLineFlag = MIPS_ARCH_32;
    m_OutputFlag = m_CmdLineFlag;
    return true;
  }

  // Parse target CPU string to determine architecture
  m_CmdLineFlag = llvm::StringSwitch<int32_t>(m_Config.targets().getTargetCPU())
                      .Case("mips1", MIPS_ARCH_1)
                      .Case("mips2", MIPS_ARCH_2)
                      .Case("mips3", MIPS_ARCH_3)
                      .Case("mips4", MIPS_ARCH_4)
                      .Case("mips5", MIPS_ARCH_5)
                      .Case("mips32", MIPS_ARCH_32)
                      .Case("mips64", MIPS_ARCH_64)
                      .Case("mips32r2", MIPS_ARCH_32R2)
                      .Case("mips64r2", MIPS_ARCH_64R2)
                      .Default(UNKNOWN);

  if (m_CmdLineFlag == UNKNOWN) {
    m_Config.raise(Diag::fatal_unsupported_emulation)
        << m_Config.targets().getTargetCPU();
    return false;
  }

  m_OutputFlag = m_CmdLineFlag;
  return true;
}

uint64_t MIPSInfo::translateFlag(uint64_t pFlag) const {
  // Extract architecture bits from ELF flag and convert to internal
  // representation
  uint64_t archFlag = pFlag & EF_MIPS_ARCH;
  switch (archFlag) {
  case EF_MIPS_ARCH_1:
    return MIPS_ARCH_1;
  case EF_MIPS_ARCH_2:
    return MIPS_ARCH_2;
  case EF_MIPS_ARCH_3:
    return MIPS_ARCH_3;
  case EF_MIPS_ARCH_4:
    return MIPS_ARCH_4;
  case EF_MIPS_ARCH_5:
    return MIPS_ARCH_5;
  case EF_MIPS_ARCH_32:
    return MIPS_ARCH_32;
  case EF_MIPS_ARCH_64:
    return MIPS_ARCH_64;
  case EF_MIPS_ARCH_32R2:
    return MIPS_ARCH_32R2;
  case EF_MIPS_ARCH_64R2:
    return MIPS_ARCH_64R2;
  default:
    return MIPS_ARCH_32; // Default to MIPS32 for unknown architectures
  }
}

bool MIPSInfo::checkFlags(uint64_t pFlag, const InputFile *pInput) const {
  if (!pFlag)
    return true;

  // Extract architecture from input file
  uint64_t inputArch = translateFlag(pFlag);
  std::string inputArchStr = flagString(pFlag);

  // Update output flag to highest architecture seen
  if (m_OutputFlag == UNKNOWN || m_OutputFlag < (int64_t)inputArch) {
    m_OutputFlag = inputArch;
  }

  // Check for basic compatibility - allow mixing within reasonable bounds
  // For now, we're permissive but could add more strict checking later
  if (m_CmdLineFlag != UNKNOWN) {
    // Check if command line architecture is compatible with input file
    if (m_CmdLineFlag != UNKNOWN && m_CmdLineFlag < (int64_t)inputArch) {
      // Warn if input file requires higher architecture than command line
      if (!m_Config.options().noWarnMismatch()) {
        m_Config.raise(Diag::incompatible_input_architecture)
            << pInput->getInput()->decoratedPath() << inputArchStr
            << ArchStrings[m_CmdLineFlag];
      }
    }
  }

  return true;
}

/// flags - the value of ElfXX_Ehdr::e_flags
uint64_t MIPSInfo::flags() const {
  int32_t outputFlag = m_OutputFlag;

  // Use command line flag if no input files processed yet
  if (m_CmdLineFlag != UNKNOWN) {
    if (outputFlag == UNKNOWN)
      outputFlag = m_CmdLineFlag;
    else if (outputFlag < m_CmdLineFlag)
      outputFlag = m_CmdLineFlag;
  }

  // Convert internal representation back to ELF flag
  if (outputFlag != UNKNOWN &&
      outputFlag < (int32_t)(sizeof(ELFFlagToArch) / sizeof(ELFFlagToArch[0])))
    return ELFFlagToArch[outputFlag];

  // Default to MIPS32 if nothing else specified
  return EF_MIPS_ARCH_32;
}

uint8_t MIPSInfo::OSABI() const { return llvm::ELF::ELFOSABI_NONE; }
