//===- MIPSGOT.h----------------------------------------------------------===//
// Part of the eld Project, under the BSD License
// See https://github.com/qualcomm/eld/LICENSE.txt for license information.
// SPDX-License-Identifier: BSD-3-Clause
//===----------------------------------------------------------------------===//

#ifndef MIPS_GOT_H
#define MIPS_GOT_H

#include "eld/Fragment/GOT.h"
#include "eld/Support/Memory.h"
#include "eld/Target/GNULDBackend.h"

namespace eld {

/** \class MIPSGOT
 *  \brief MIPS Global Offset Table.
 */
class MIPSGOT : public GOT {
public:
  // Helper constructor for GOT.
  MIPSGOT(GOTType T, ELFSection *O, ResolveInfo *R) : GOT(T, O, R, 4, 4) {
    if (O)
      O->addFragmentAndUpdateSize(this);
  }

  virtual ~MIPSGOT() {}

  virtual MIPSGOT *getFirst() { return this; }

  virtual MIPSGOT *getNext() { return nullptr; }

  virtual llvm::ArrayRef<uint8_t> getContent() const override {
    // Convert uint32_t to ArrayRef.
    typedef union {
      uint32_t a;
      uint8_t b[4];
    } C;
    C Content;
    Content.a = 0;
    // If the GOT contents needs to reflect a symbol value, then we use the
    // symbol value.
    if (getValueType() == GOT::SymbolValue)
      Content.a = symInfo()->outSymbol()->value();
    if (getValueType() == GOT::TLSStaticSymbolValue)
      Content.a =
          symInfo()->outSymbol()->value() - GNULDBackend::getTLSTemplateSize();
    std::memcpy((void *)Value, (void *)&Content.a, sizeof(Value));
    return llvm::ArrayRef(Value);
  }

  static MIPSGOT *Create(ELFSection *O, ResolveInfo *R) {
    return make<MIPSGOT>(GOT::Regular, O, R);
  }

private:
  uint8_t Value[4] = {0};
};

} // namespace eld

#endif
