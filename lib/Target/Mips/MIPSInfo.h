//===- MIPSInfo.h---------------------------------------------------------===//
// Part of the eld Project, under the BSD License
// See https://github.com/qualcomm/eld/LICENSE.txt for license information.
// SPDX-License-Identifier: BSD-3-Clause
//===----------------------------------------------------------------------===//

#ifndef ELD_TARGET_MIPS_GNU_INFO_H
#define ELD_TARGET_MIPS_GNU_INFO_H
#include "eld/Config/TargetOptions.h"
#include "eld/Target/TargetInfo.h"
#include "llvm/BinaryFormat/ELF.h"

namespace eld {

class MIPSInfo : public TargetInfo {
public:
  MIPSInfo(LinkerConfig &m_Config);

  bool initialize() override;

  uint32_t machine() const override { return llvm::ELF::EM_MIPS; }

  /// flags - the value of ElfXX_Ehdr::e_flags
  uint64_t flags() const override;

  uint8_t OSABI() const override;

  bool checkFlags(uint64_t flags, const InputFile *pInput) const override;

  std::string flagString(uint64_t pFlag) const override;

  int32_t cmdLineFlag() const override { return m_CmdLineFlag; }

  int32_t outputFlag() const override { return m_OutputFlag; }

  bool needEhdr(Module &pModule, bool linkerScriptHasSectionsCmd,
                bool isPhdr) override {
    return false & isPhdr;
  }

  bool processNoteGNUSTACK() override { return true; }

  llvm::StringRef getOutputMCPU() const override;

  std::string getMachineStr() const override { return "mips"; }

  uint64_t startAddr(bool linkerScriptHasSectionsCommand, bool isDynExec,
                     bool loadPhdr) const override {
    if (linkerScriptHasSectionsCommand)
      return 0;
    if (!isDynExec)
      return 0;
    return 0x400000; // Default MIPS start address
  }

private:
  uint64_t translateFlag(uint64_t pFlag) const;
  int32_t m_CmdLineFlag;
  mutable int32_t m_OutputFlag;
};

} // namespace eld

#endif
