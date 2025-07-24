//===- MIPSRelocationInfo.h-----------------------------------------------===//
// Part of the eld Project, under the BSD License
// See https://github.com/qualcomm/eld/LICENSE.txt for license information.
// SPDX-License-Identifier: BSD-3-Clause
//===----------------------------------------------------------------------===//

#ifndef MIPS_RELOCATION_INFO_H
#define MIPS_RELOCATION_INFO_H

namespace eld {

// MIPS 32-bit ELF relocation types
enum {
  R_MIPS_NONE = 0,     // No relocation
  R_MIPS_16 = 1,       // 16-bit relocation
  R_MIPS_32 = 2,       // 32-bit relocation
  R_MIPS_REL32 = 3,    // 32-bit relative relocation
  R_MIPS_26 = 4,       // 26-bit jump relocation
  R_MIPS_HI16 = 5,     // High 16 bits relocation
  R_MIPS_LO16 = 6,     // Low 16 bits relocation
  R_MIPS_GPREL16 = 7,  // GP-relative 16-bit relocation
  R_MIPS_LITERAL = 8,  // Literal relocation
  R_MIPS_GOT16 = 9,    // GOT 16-bit relocation
  R_MIPS_PC16 = 10,    // PC-relative 16-bit relocation
  R_MIPS_CALL16 = 11,  // Call 16-bit relocation
  R_MIPS_GPREL32 = 12, // GP-relative 32-bit relocation

  // Extended relocations (18-51)
  R_MIPS_64 = 18,            // 64-bit relocation
  R_MIPS_GOT_DISP = 19,      // GOT displacement
  R_MIPS_GOT_PAGE = 20,      // GOT page
  R_MIPS_GOT_OFST = 21,      // GOT offset
  R_MIPS_GOT_HI16 = 22,      // GOT high 16
  R_MIPS_GOT_LO16 = 23,      // GOT low 16
  R_MIPS_SUB = 24,           // Subtraction
  R_MIPS_INSERT_A = 25,      // Insert A
  R_MIPS_INSERT_B = 26,      // Insert B
  R_MIPS_DELETE = 27,        // Delete
  R_MIPS_HIGHER = 28,        // Higher 16 bits
  R_MIPS_HIGHEST = 29,       // Highest 16 bits
  R_MIPS_CALL_HI16 = 30,     // Call high 16
  R_MIPS_CALL_LO16 = 31,     // Call low 16
  R_MIPS_SCN_DISP = 32,      // Section displacement
  R_MIPS_REL16 = 33,         // Relative 16
  R_MIPS_ADD_IMMEDIATE = 34, // Add immediate
  R_MIPS_PJUMP = 35,         // Packed jump
  R_MIPS_RELGOT = 36,        // Relative GOT
  R_MIPS_JALR = 37,          // Jump and link register

  // TLS relocations
  R_MIPS_TLS_DTPMOD32 = 38,    // TLS module ID (32-bit)
  R_MIPS_TLS_DTPREL32 = 39,    // TLS offset (32-bit)
  R_MIPS_TLS_DTPMOD64 = 40,    // TLS module ID (64-bit)
  R_MIPS_TLS_DTPREL64 = 41,    // TLS offset (64-bit)
  R_MIPS_TLS_GD = 42,          // TLS general dynamic
  R_MIPS_TLS_LDM = 43,         // TLS local dynamic module
  R_MIPS_TLS_DTPREL_HI16 = 44, // TLS offset high 16
  R_MIPS_TLS_DTPREL_LO16 = 45, // TLS offset low 16
  R_MIPS_TLS_GOTTPREL = 46,    // TLS GOT offset
  R_MIPS_TLS_TPREL32 = 47,     // TLS thread pointer offset (32-bit)
  R_MIPS_TLS_TPREL64 = 48,     // TLS thread pointer offset (64-bit)
  R_MIPS_TLS_TPREL_HI16 = 49,  // TLS thread pointer offset high 16
  R_MIPS_TLS_TPREL_LO16 = 50,  // TLS thread pointer offset low 16
  R_MIPS_GLOB_DAT = 51         // Global data
};

// MIPS ELF header flags
enum {
  EF_MIPS_NOREORDER = 0x00000001,   // Don't reorder instructions
  EF_MIPS_PIC = 0x00000002,         // Position independent code
  EF_MIPS_CPIC = 0x00000004,        // Call PIC functions
  EF_MIPS_XGOT = 0x00000008,        // Extended GOT
  EF_MIPS_64BIT_WHIRL = 0x00000010, // 64-bit whirl
  EF_MIPS_ABI2 = 0x00000020,        // ABI2
  EF_MIPS_ABI_ON32 = 0x00000040,    // ABI on 32
  EF_MIPS_FP64 = 0x00000200,        // 64-bit FP registers
  EF_MIPS_NAN2008 = 0x00000400,     // IEEE 754-2008 NaN encoding

  // Architecture level
  EF_MIPS_ARCH = 0xf0000000,      // Architecture mask
  EF_MIPS_ARCH_1 = 0x00000000,    // MIPS I
  EF_MIPS_ARCH_2 = 0x10000000,    // MIPS II
  EF_MIPS_ARCH_3 = 0x20000000,    // MIPS III
  EF_MIPS_ARCH_4 = 0x30000000,    // MIPS IV
  EF_MIPS_ARCH_5 = 0x40000000,    // MIPS V
  EF_MIPS_ARCH_32 = 0x50000000,   // MIPS32
  EF_MIPS_ARCH_64 = 0x60000000,   // MIPS64
  EF_MIPS_ARCH_32R2 = 0x70000000, // MIPS32 R2
  EF_MIPS_ARCH_64R2 = 0x80000000  // MIPS64 R2
};

} // namespace eld

#endif
