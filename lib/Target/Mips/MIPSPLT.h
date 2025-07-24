//===- MIPSPLT.h----------------------------------------------------------===//
// Part of the eld Project, under the BSD License
// See https://github.com/qualcomm/eld/LICENSE.txt for license information.
// SPDX-License-Identifier: BSD-3-Clause
//===----------------------------------------------------------------------===//

#ifndef MIPS_PLT_H
#define MIPS_PLT_H

#include "MIPSGOT.h"
#include "eld/Fragment/PLT.h"
#include "eld/SymbolResolver/IRBuilder.h"

namespace eld {

class MIPSGOT;
class IRBuilder;

class MIPSPLT : public PLT {
public:
  MIPSPLT(PLT::PLTType T, eld::IRBuilder &I, MIPSGOT *G, ELFSection *P,
          ResolveInfo *R, uint32_t Align, uint32_t Size)
      : PLT(T, G, P, R, Align, Size) {}

  virtual ~MIPSPLT() {}

  virtual llvm::ArrayRef<uint8_t> getContent() const override {
    // Simple placeholder content for now
    static const uint8_t mips_plt_content[16] = {0};
    return llvm::ArrayRef<uint8_t>(mips_plt_content, 16);
  }

  virtual MIPSPLT *getFirst() { return this; }

  virtual MIPSPLT *getNext() { return nullptr; }

  static bool classof(const Fragment *F) {
    return F->getKind() == Fragment::Plt;
  }

  static bool classof(const MIPSPLT *) { return true; }
};

} // namespace eld

#endif
