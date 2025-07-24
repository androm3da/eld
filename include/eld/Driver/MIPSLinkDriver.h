//===- MIPSLinkDriver.h---------------------------------------------------===//
// Part of the eld Project, under the BSD License
// See https://github.com/qualcomm/eld/LICENSE.txt for license information.
// SPDX-License-Identifier: BSD-3-Clause
//===----------------------------------------------------------------------===//
#ifndef ELD_DRIVER_MIPSLINKDRIVER_H
#define ELD_DRIVER_MIPSLINKDRIVER_H

#include "eld/Config/LinkerConfig.h"
#include "eld/Core/LinkerScript.h"
#include "eld/Core/Module.h"
#include "eld/Driver/GnuLdDriver.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/TargetParser/Triple.h"
#include <optional>

namespace eld {
class DiagnosticEngine;
}

// Create OptTable class for parsing actual command line arguments
class OPT_MIPSLinkOptTable : public llvm::opt::GenericOptTable {
public:
  enum {
    INVALID = 0,
#define OPTION(PREFIXES_OFFSET, PREFIXED_NAME_OFFSET, ID, KIND, GROUP, ALIAS,  \
               ALIASARGS, FLAGS, VISIBILITY, PARAM, HELPTEXT,                  \
               HELPTEXTSFORVARIANTS, METAVAR, VALUES)                          \
  ID,
#include "eld/Driver/MIPSLinkerOptions.inc"
#undef OPTION
  };

  OPT_MIPSLinkOptTable();
};

class MIPSLinkDriver : public GnuLdDriver {
public:
  static MIPSLinkDriver *Create(eld::LinkerConfig &C, Flavor F,
                                std::string Triple);

  MIPSLinkDriver(eld::LinkerConfig &C, Flavor F, std::string Triple);

  virtual ~MIPSLinkDriver() {}

  // Main entry point.
  int link(llvm::ArrayRef<const char *> Args,
           llvm::ArrayRef<llvm::StringRef> ELDFlagsArgs) override;

  // Parse Options.
  llvm::opt::OptTable *parseOptions(llvm::ArrayRef<const char *> ArgsArr,
                                    llvm::opt::InputArgList &ArgList) override;

  // Add linker script library search file for this target
  static llvm::StringRef getLinkerScriptPathBySoname(llvm::StringRef soname);

  // Emulation validation
  static bool isValidEmulation(llvm::StringRef Emulation);

protected:
  // Template method implementations for the ELD pipeline
  template <typename T> bool checkOptions(llvm::opt::InputArgList &Args);

  template <typename T> bool processOptions(llvm::opt::InputArgList &Args);

  template <typename T> bool processLLVMOptions(llvm::opt::InputArgList &Args);

  template <typename T>
  bool processTargetOptions(llvm::opt::InputArgList &Args);

  template <typename T>
  bool createInputActions(llvm::opt::InputArgList &Args,
                          std::vector<eld::InputAction *> &actions);

protected:
  // Options table
  std::unique_ptr<llvm::opt::OptTable> Table;
};

#endif
