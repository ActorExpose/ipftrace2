/*
 * SPDX-License-Identifier: (GPL-2.0-only OR BSD-2-Clause)
 * Copyright (C) 2020 Yutaro Hayakawa
 */
#pragma once
#include <linux/bpf.h>

#define BPF_ALU64_REG(OP, DST, SRC)                                            \
  ((struct bpf_insn){.code = BPF_ALU64 | BPF_OP(OP) | BPF_X,                   \
                     .dst_reg = DST,                                           \
                     .src_reg = SRC,                                           \
                     .off = 0,                                                 \
                     .imm = 0})

#define BPF_ALU32_REG(OP, DST, SRC)                                            \
  ((struct bpf_insn){.code = BPF_ALU | BPF_OP(OP) | BPF_X,                     \
                     .dst_reg = DST,                                           \
                     .src_reg = SRC,                                           \
                     .off = 0,                                                 \
                     .imm = 0})

#define BPF_ALU64_IMM(OP, DST, IMM)                                            \
  ((struct bpf_insn){.code = BPF_ALU64 | BPF_OP(OP) | BPF_K,                   \
                     .dst_reg = DST,                                           \
                     .src_reg = 0,                                             \
                     .off = 0,                                                 \
                     .imm = IMM})

#define BPF_ALU32_IMM(OP, DST, IMM)                                            \
  ((struct bpf_insn){.code = BPF_ALU | BPF_OP(OP) | BPF_K,                     \
                     .dst_reg = DST,                                           \
                     .src_reg = 0,                                             \
                     .off = 0,                                                 \
                     .imm = IMM})

#define BPF_ENDIAN(TYPE, DST, LEN)                                             \
  ((struct bpf_insn){.code = BPF_ALU | BPF_END | BPF_SRC(TYPE),                \
                     .dst_reg = DST,                                           \
                     .src_reg = 0,                                             \
                     .off = 0,                                                 \
                     .imm = LEN})

#define BPF_MOV64_REG(DST, SRC)                                                \
  ((struct bpf_insn){.code = BPF_ALU64 | BPF_MOV | BPF_X,                      \
                     .dst_reg = DST,                                           \
                     .src_reg = SRC,                                           \
                     .off = 0,                                                 \
                     .imm = 0})

#define BPF_MOV32_REG(DST, SRC)                                                \
  ((struct bpf_insn){.code = BPF_ALU | BPF_MOV | BPF_X,                        \
                     .dst_reg = DST,                                           \
                     .src_reg = SRC,                                           \
                     .off = 0,                                                 \
                     .imm = 0})

#define BPF_MOV64_IMM(DST, IMM)                                                \
  ((struct bpf_insn){.code = BPF_ALU64 | BPF_MOV | BPF_K,                      \
                     .dst_reg = DST,                                           \
                     .src_reg = 0,                                             \
                     .off = 0,                                                 \
                     .imm = IMM})

#define BPF_MOV32_IMM(DST, IMM)                                                \
  ((struct bpf_insn){.code = BPF_ALU | BPF_MOV | BPF_K,                        \
                     .dst_reg = DST,                                           \
                     .src_reg = 0,                                             \
                     .off = 0,                                                 \
                     .imm = IMM})

#define BPF_LD_IMM64(DST, IMM) BPF_LD_IMM64_RAW(DST, 0, IMM)

#define BPF_LD_IMM64_RAW(DST, SRC, IMM)                                        \
  ((struct bpf_insn){.code = BPF_LD | BPF_DW | BPF_IMM,                        \
                     .dst_reg = DST,                                           \
                     .src_reg = SRC,                                           \
                     .off = 0,                                                 \
                     .imm = (__u32)(IMM)}),                                    \
      ((struct bpf_insn){.code = 0,                                            \
                         .dst_reg = 0,                                         \
                         .src_reg = 0,                                         \
                         .off = 0,                                             \
                         .imm = ((__u64)(IMM)) >> 32})

#define BPF_LD_MAP_FD(DST, MAP_FD)                                             \
  BPF_LD_IMM64_RAW(DST, BPF_PSEUDO_MAP_FD, MAP_FD)

#define BPF_MOV64_RAW(TYPE, DST, SRC, IMM)                                     \
  ((struct bpf_insn){.code = BPF_ALU64 | BPF_MOV | BPF_SRC(TYPE),              \
                     .dst_reg = DST,                                           \
                     .src_reg = SRC,                                           \
                     .off = 0,                                                 \
                     .imm = IMM})

#define BPF_MOV32_RAW(TYPE, DST, SRC, IMM)                                     \
  ((struct bpf_insn){.code = BPF_ALU | BPF_MOV | BPF_SRC(TYPE),                \
                     .dst_reg = DST,                                           \
                     .src_reg = SRC,                                           \
                     .off = 0,                                                 \
                     .imm = IMM})

#define BPF_LDX_MEM(SIZE, DST, SRC, OFF)                                       \
  ((struct bpf_insn){.code = BPF_LDX | BPF_SIZE(SIZE) | BPF_MEM,               \
                     .dst_reg = DST,                                           \
                     .src_reg = SRC,                                           \
                     .off = OFF,                                               \
                     .imm = 0})

#define BPF_STX_MEM(SIZE, DST, SRC, OFF)                                       \
  ((struct bpf_insn){.code = BPF_STX | BPF_SIZE(SIZE) | BPF_MEM,               \
                     .dst_reg = DST,                                           \
                     .src_reg = SRC,                                           \
                     .off = OFF,                                               \
                     .imm = 0})

#define BPF_STX_XADD(SIZE, DST, SRC, OFF)                                      \
  ((struct bpf_insn){.code = BPF_STX | BPF_SIZE(SIZE) | BPF_XADD,              \
                     .dst_reg = DST,                                           \
                     .src_reg = SRC,                                           \
                     .off = OFF,                                               \
                     .imm = 0})

#define BPF_ST_MEM(SIZE, DST, OFF, IMM)                                        \
  ((struct bpf_insn){.code = BPF_ST | BPF_SIZE(SIZE) | BPF_MEM,                \
                     .dst_reg = DST,                                           \
                     .src_reg = 0,                                             \
                     .off = OFF,                                               \
                     .imm = IMM})

#define BPF_JMP_REG(OP, DST, SRC, OFF)                                         \
  ((struct bpf_insn){.code = BPF_JMP | BPF_OP(OP) | BPF_X,                     \
                     .dst_reg = DST,                                           \
                     .src_reg = SRC,                                           \
                     .off = OFF,                                               \
                     .imm = 0})

#define BPF_JMP_IMM(OP, DST, IMM, OFF)                                         \
  ((struct bpf_insn){.code = BPF_JMP | BPF_OP(OP) | BPF_K,                     \
                     .dst_reg = DST,                                           \
                     .src_reg = 0,                                             \
                     .off = OFF,                                               \
                     .imm = IMM})

#define BPF_CALL_INSN(FUNC)                                                    \
  ((struct bpf_insn){.code = BPF_JMP | BPF_CALL,                               \
                     .dst_reg = 0,                                             \
                     .src_reg = 0,                                             \
                     .off = 0,                                                 \
                     .imm = (FUNC)})

#define BPF_EXIT_INSN()                                                        \
  ((struct bpf_insn){.code = BPF_JMP | BPF_EXIT,                               \
                     .dst_reg = 0,                                             \
                     .src_reg = 0,                                             \
                     .off = 0,                                                 \
                     .imm = 0})

#define BPF_RAW_INSN(CODE, DST, SRC, OFF, IMM)                                 \
  ((struct bpf_insn){                                                          \
      .code = CODE, .dst_reg = DST, .src_reg = SRC, .off = OFF, .imm = IMM})
