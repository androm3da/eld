//===- MIPSELFDynamic.h---------------------------------------------------===//
// Part of the eld Project, under the BSD License
// See https://github.com/qualcomm/eld/LICENSE.txt for license information.
// SPDX-License-Identifier: BSD-3-Clause
//===----------------------------------------------------------------------===//

#ifndef ELD_MIPS_ELFDYNAMIC_SECTION_H
#define ELD_MIPS_ELFDYNAMIC_SECTION_H

#include "eld/Target/ELFDynamic.h"

namespace eld {

class MIPSELFDynamic : public ELFDynamic {
public:
  MIPSELFDynamic(GNULDBackend &pParent, LinkerConfig &pConfig);
  ~MIPSELFDynamic();

private:
  void reserveTargetEntries() override;
  void applyTargetEntries() override;
};

} // namespace eld

#endif