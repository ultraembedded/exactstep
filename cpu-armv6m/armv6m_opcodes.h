//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#ifndef ARMV6M_OPCODES_H
#define ARMV6M_OPCODES_H

#define INST_IGRP0          0
#define INST_IGRP1          1
#define INST_IGRP2          2
#define INST_IGRP3          3
#define INST_IGRP4          4
#define INST_IGRP5          5
#define INST_IGRP6          6
#define INST_IGRP7          7
#define INST_IGRP8          8

#define INST_IGRP0_MASK     0xF000
#define INST_IGRP1_MASK     0xF800
#define INST_IGRP2_MASK     0xFE00
#define INST_IGRP3_MASK     0xFF00
#define INST_IGRP4_MASK     0xFF80
#define INST_IGRP5_MASK     0xFFC0
#define INST_IGRP6_MASK     0xFFE0
#define INST_IGRP7_MASK     0xFFF0
#define INST_IGRP8_MASK     0xFFFF

#define INST_ADCS_MASK      0xFFC0
#define INST_ADDS_MASK      0xFE00
#define INST_ADDS_1_MASK    0xF800
#define INST_ADDS_2_MASK    0xFE00
#define INST_ADD_MASK       0xFF00
#define INST_ADD_1_MASK     0xF800
#define INST_ADD_2_MASK     0xFF80
#define INST_ADR_MASK       0xF800
#define INST_ANDS_MASK      0xFFC0
#define INST_ASRS_MASK      0xF800
#define INST_ASRS_1_MASK    0xFFC0
#define INST_BCC_MASK       0xF000
#define INST_B_MASK         0xF800
#define INST_BICS_MASK      0xFFC0
#define INST_BKPT_MASK      0xFF00
#define INST_BL_MASK        0xF800
#define INST_BLX_MASK       0xFF80
#define INST_BX_MASK        0xFF80
#define INST_CMN_MASK       0xFFC0
#define INST_CMP_MASK       0xF800
#define INST_CMP_1_MASK     0xFFC0
#define INST_CMP_2_MASK     0xFF00
#define INST_CPS_MASK       0xFFE0
#define INST_DMB_MASK       0xFFF0
#define INST_DSB_MASK       0xFFF0
#define INST_EORS_MASK      0xFFC0
#define INST_ISB_MASK       0xFFF0
#define INST_LDM_MASK       0xF800
#define INST_LDM_1_MASK     0xF800
#define INST_LDR_MASK       0xF800
#define INST_LDR_1_MASK     0xF800
#define INST_LDR_2_MASK     0xF800
#define INST_LDR_3_MASK     0xFE00
#define INST_LDRB_MASK      0xF800
#define INST_LDRB_1_MASK    0xFE00
#define INST_LDRH_MASK      0xF800
#define INST_LDRH_1_MASK    0xFE00
#define INST_LDRSB_MASK     0xFE00
#define INST_LDRSH_MASK     0xFE00
#define INST_LSLS_MASK      0xF800
#define INST_LSLS_1_MASK    0xFFC0
#define INST_LSRS_MASK      0xF800
#define INST_LSRS_1_MASK    0xFFC0
#define INST_MOVS_MASK      0xF800
#define INST_MOV_MASK       0xFF00
#define INST_MOVS_1_MASK    0xFFC0
#define INST_MRS_MASK       0xFFE0
#define INST_MSR_MASK       0xFFE0
#define INST_MULS_MASK      0xFFC0
#define INST_MVNS_MASK      0xFFC0
#define INST_NOP_MASK       0xFFFF
#define INST_ORRS_MASK      0xFFC0
#define INST_POP_MASK       0xFE00
#define INST_PUSH_MASK      0xFE00
#define INST_REV_MASK       0xFFC0
#define INST_REV16_MASK     0xFFC0
#define INST_REVSH_MASK     0xFFC0
#define INST_RORS_MASK      0xFFC0
#define INST_RSBS_MASK      0xFFC0
#define INST_SBCS_MASK      0xFFC0
#define INST_SEV_MASK       0xFFFF
#define INST_STM_MASK       0xF800
#define INST_STR_MASK       0xF800
#define INST_STR_1_MASK     0xF800
#define INST_STR_2_MASK     0xFE00
#define INST_STRB_MASK      0xF800
#define INST_STRB_1_MASK    0xFE00
#define INST_STRH_MASK      0xF800
#define INST_STRH_1_MASK    0xFE00
#define INST_SUBS_MASK      0xFE00
#define INST_SUBS_1_MASK    0xF800
#define INST_SUBS_2_MASK    0xFE00
#define INST_SUB_MASK       0xFF80
#define INST_SVC_MASK       0xFF00
#define INST_SXTB_MASK      0xFFC0
#define INST_SXTH_MASK      0xFFC0
#define INST_TST_MASK       0xFFC0
#define INST_UDF_MASK       0xFF00
#define INST_UDF_W_MASK     0xFFF0
#define INST_UXTB_MASK      0xFFC0
#define INST_UXTH_MASK      0xFFC0
#define INST_WFE_MASK       0xFFFF
#define INST_WFI_MASK       0xFFFF
#define INST_YIELD_MASK     0xFFFF



#define INST_ADCS_OPCODE        0x4140
#define INST_ADDS_OPCODE        0x1C00
#define INST_ADDS_1_OPCODE      0x3000
#define INST_ADDS_2_OPCODE      0x1800
#define INST_ADD_OPCODE         0x4400
#define INST_ADD_1_OPCODE       0xA800
#define INST_ADD_2_OPCODE       0xB000
#define INST_ADR_OPCODE         0xA000
#define INST_ANDS_OPCODE        0x4000
#define INST_ASRS_OPCODE        0x1000
#define INST_ASRS_1_OPCODE      0x4100
#define INST_BCC_OPCODE         0xD000
#define INST_B_OPCODE           0xE000
#define INST_BICS_OPCODE        0x4380
#define INST_BKPT_OPCODE        0xBE00
#define INST_BL_OPCODE          0xF000
#define INST_BLX_OPCODE         0x4780
#define INST_BX_OPCODE          0x4700
#define INST_CMN_OPCODE         0x42C0
#define INST_CMP_OPCODE         0x2800
#define INST_CMP_1_OPCODE       0x4280
#define INST_CMP_2_OPCODE       0x4500
#define INST_CPS_OPCODE         0xB660
#define INST_DMB_OPCODE         0xF3B0
#define INST_DSB_OPCODE         0xF3B0
#define INST_EORS_OPCODE        0x4040
#define INST_ISB_OPCODE         0xF3B0
#define INST_LDM_OPCODE         0xC800
#define INST_LDM_1_OPCODE       0xC800
#define INST_LDR_OPCODE         0x6800
#define INST_LDR_1_OPCODE       0x9800
#define INST_LDR_2_OPCODE       0x4800
#define INST_LDR_3_OPCODE       0x5800
#define INST_LDRB_OPCODE        0x7800
#define INST_LDRB_1_OPCODE      0x5C00
#define INST_LDRH_OPCODE        0x8800
#define INST_LDRH_1_OPCODE      0x5A00
#define INST_LDRSB_OPCODE       0x5600
#define INST_LDRSH_OPCODE       0x5E00
#define INST_LSLS_OPCODE        0x0000
#define INST_LSLS_1_OPCODE      0x4080
#define INST_LSRS_OPCODE        0x0800
#define INST_LSRS_1_OPCODE      0x40C0
#define INST_MOVS_OPCODE        0x2000
#define INST_MOV_OPCODE         0x4600
#define INST_MOVS_1_OPCODE      0x0000
#define INST_MRS_OPCODE         0xF3E0
#define INST_MSR_OPCODE         0xF380
#define INST_MULS_OPCODE        0x4340
#define INST_MVNS_OPCODE        0x43C0
#define INST_NOP_OPCODE         0xBF00
#define INST_ORRS_OPCODE        0x4300
#define INST_POP_OPCODE         0xBC00
#define INST_PUSH_OPCODE        0xB400
#define INST_REV_OPCODE         0xBA00
#define INST_REV16_OPCODE       0xBA40
#define INST_REVSH_OPCODE       0xBAC0
#define INST_RORS_OPCODE        0x41C0
#define INST_RSBS_OPCODE        0x4240
#define INST_SBCS_OPCODE        0x4180
#define INST_SEV_OPCODE         0xBF40
#define INST_STM_OPCODE         0xC000
#define INST_STR_OPCODE         0x6000
#define INST_STR_1_OPCODE       0x9000
#define INST_STR_2_OPCODE       0x5000
#define INST_STRB_OPCODE        0x7000
#define INST_STRB_1_OPCODE      0x5400
#define INST_STRH_OPCODE        0x8000
#define INST_STRH_1_OPCODE      0x5200
#define INST_SUBS_OPCODE        0x1E00
#define INST_SUBS_1_OPCODE      0x3800
#define INST_SUBS_2_OPCODE      0x1A00
#define INST_SUB_OPCODE         0xB080
#define INST_SVC_OPCODE         0xDF00
#define INST_SXTB_OPCODE        0xB240
#define INST_SXTH_OPCODE        0xB200
#define INST_TST_OPCODE         0x4200
#define INST_UDF_OPCODE         0xDE00
#define INST_UDF_W_OPCODE       0xF7F0
#define INST_UXTB_OPCODE        0xB2C0
#define INST_UXTH_OPCODE        0xB280
#define INST_WFE_OPCODE         0xBF20
#define INST_WFI_OPCODE         0xBF30
#define INST_YIELD_OPCODE       0xBF10


struct cm0_inst
{
    unsigned int opcode;
    unsigned int mask;
    const char *desc;
};

static struct cm0_inst instr_details[] = 
{
{ INST_ADCS_OPCODE, INST_ADCS_MASK, "ADCS <Rdn>,<Rm>" },
{ INST_ADDS_OPCODE, INST_ADDS_MASK, "ADDS <Rd>,<Rn>,#<imm3>" },
{ INST_ADDS_1_OPCODE, INST_ADDS_1_MASK, "ADDS <Rdn>,#<imm8>" },
{ INST_ADDS_2_OPCODE, INST_ADDS_2_MASK, "ADDS <Rd>,<Rn>,<Rm>" },
{ INST_ADD_OPCODE, INST_ADD_MASK, "ADD <Rdn>,<Rm>" },
{ INST_ADD_1_OPCODE, INST_ADD_1_MASK, "ADD <Rd>,SP,#<imm8>" },
{ INST_ADD_2_OPCODE, INST_ADD_2_MASK, "ADD SP,SP,#<imm7>" },
{ INST_ADR_OPCODE, INST_ADR_MASK, "ADR <Rd>,<label>" },
{ INST_ANDS_OPCODE, INST_ANDS_MASK, "ANDS <Rdn>,<Rm>" },
{ INST_ASRS_OPCODE, INST_ASRS_MASK, "ASRS <Rd>,<Rm>,#<imm5>" },
{ INST_ASRS_1_OPCODE, INST_ASRS_1_MASK, "ASRS <Rdn>,<Rm>" },
{ INST_BCC_OPCODE, INST_BCC_MASK, "BCC <label>" },
{ INST_B_OPCODE, INST_B_MASK, "B <label>" },
{ INST_BICS_OPCODE, INST_BICS_MASK, "BICS <Rdn>,<Rm>" },
{ INST_BKPT_OPCODE, INST_BKPT_MASK, "BKPT #<imm8>" },
{ INST_BL_OPCODE, INST_BL_MASK, "BL <label>" },
{ INST_BLX_OPCODE, INST_BLX_MASK, "BLX <Rm>" },
{ INST_BX_OPCODE, INST_BX_MASK, "BX <Rm>" },
{ INST_CMN_OPCODE, INST_CMN_MASK, "CMN <Rn>,<Rm>" },
{ INST_CMP_OPCODE, INST_CMP_MASK, "CMP <Rn>,#<imm8>" },
{ INST_CMP_1_OPCODE, INST_CMP_1_MASK, "CMP <Rn>,<Rm> <Rn> and <Rm> both from R0-R7" },
{ INST_CMP_2_OPCODE, INST_CMP_2_MASK, "CMP <Rn>,<Rm> <Rn> and <Rm> not both from R0-R7" },
{ INST_DMB_OPCODE, INST_DMB_MASK, "DMB #<option>" },
{ INST_DSB_OPCODE, INST_DSB_MASK, "DSB #<option>" },
{ INST_EORS_OPCODE, INST_EORS_MASK, "EORS <Rdn>,<Rm>" },
{ INST_ISB_OPCODE, INST_ISB_MASK, "ISB #<option>" },
{ INST_LDM_OPCODE, INST_LDM_MASK, "LDM <Rn>!,<registers> <Rn> not included in <registers>" },
{ INST_LDM_1_OPCODE, INST_LDM_1_MASK, "LDM <Rn>,<registers> <Rn> included in <registers>" },
{ INST_LDR_OPCODE, INST_LDR_MASK, "LDR <Rt>, [<Rn>{,#<imm5>}]" },
{ INST_LDR_1_OPCODE, INST_LDR_1_MASK, "LDR <Rt>,[SP{,#<imm8>}]" },
{ INST_LDR_2_OPCODE, INST_LDR_2_MASK, "LDR <Rt>,<label>" },
{ INST_LDR_3_OPCODE, INST_LDR_3_MASK, "LDR <Rt>,[<Rn>,<Rm>]" },
{ INST_LDRB_OPCODE, INST_LDRB_MASK, "LDRB <Rt>,[<Rn>{,#<imm5>}]" },
{ INST_LDRB_1_OPCODE, INST_LDRB_1_MASK, "LDRB <Rt>,[<Rn>,<Rm>]" },
{ INST_LDRH_OPCODE, INST_LDRH_MASK, "LDRH <Rt>,[<Rn>{,#<imm5>}]" },
{ INST_LDRH_1_OPCODE, INST_LDRH_1_MASK, "LDRH <Rt>,[<Rn>,<Rm>]" },
{ INST_LDRSB_OPCODE, INST_LDRSB_MASK, "LDRSB <Rt>,[<Rn>,<Rm>]" },
{ INST_LDRSH_OPCODE, INST_LDRSH_MASK, "LDRSH <Rt>,[<Rn>,<Rm>]" },
{ INST_LSLS_OPCODE, INST_LSLS_MASK, "LSLS <Rd>,<Rm>,#<imm5>" },
{ INST_LSLS_1_OPCODE, INST_LSLS_1_MASK, "LSLS <Rdn>,<Rm>" },
{ INST_LSRS_OPCODE, INST_LSRS_MASK, "LSRS <Rd>,<Rm>,#<imm5>" },
{ INST_LSRS_1_OPCODE, INST_LSRS_1_MASK, "LSRS <Rdn>,<Rm>" },
{ INST_MOVS_OPCODE, INST_MOVS_MASK, "MOVS <Rd>,#<imm8>" },
{ INST_MOV_OPCODE, INST_MOV_MASK, "MOV <Rd>,<Rm> Otherwise all versions of the Thumb instruction set." },
{ INST_MOVS_1_OPCODE, INST_MOVS_1_MASK, "MOVS <Rd>,<Rm>" },
{ INST_MRS_OPCODE, INST_MRS_MASK, "MRS <Rd>,<spec_reg>" },
{ INST_MSR_OPCODE, INST_MSR_MASK, "MSR <spec_reg>,<Rn>" },
{ INST_MULS_OPCODE, INST_MULS_MASK, "MULS <Rdm>,<Rn>,<Rdm>" },
{ INST_MVNS_OPCODE, INST_MVNS_MASK, "MVNS <Rd>,<Rm>" },
{ INST_NOP_OPCODE, INST_NOP_MASK, "NOP" },
{ INST_ORRS_OPCODE, INST_ORRS_MASK, "ORRS <Rdn>,<Rm>" },
{ INST_POP_OPCODE, INST_POP_MASK, "POP <registers>" },
{ INST_PUSH_OPCODE, INST_PUSH_MASK, "PUSH <registers>" },
{ INST_REV_OPCODE, INST_REV_MASK, "REV <Rd>,<Rm>" },
{ INST_REV16_OPCODE, INST_REV16_MASK, "REV16 <Rd>,<Rm>" },
{ INST_REVSH_OPCODE, INST_REVSH_MASK, "REVSH <Rd>,<Rm>" },
{ INST_RORS_OPCODE, INST_RORS_MASK, "RORS <Rdn>,<Rm>" },
{ INST_RSBS_OPCODE, INST_RSBS_MASK, "RSBS <Rd>,<Rn>,#0" },
{ INST_SBCS_OPCODE, INST_SBCS_MASK, "SBCS <Rdn>,<Rm>" },
{ INST_SEV_OPCODE, INST_SEV_MASK, "SEV" },
{ INST_STM_OPCODE, INST_STM_MASK, "STM <Rn>!,<registers>" },
{ INST_STR_OPCODE, INST_STR_MASK, "STR <Rt>, [<Rn>{,#<imm5>}]" },
{ INST_STR_1_OPCODE, INST_STR_1_MASK, "STR <Rt>,[SP,#<imm8>]" },
{ INST_STR_2_OPCODE, INST_STR_2_MASK, "STR <Rt>,[<Rn>,<Rm>]" },
{ INST_STRB_OPCODE, INST_STRB_MASK, "STRB <Rt>,[<Rn>,#<imm5>]" },
{ INST_STRB_1_OPCODE, INST_STRB_1_MASK, "STRB <Rt>,[<Rn>,<Rm>]" },
{ INST_STRH_OPCODE, INST_STRH_MASK, "STRH <Rt>,[<Rn>{,#<imm5>}]" },
{ INST_STRH_1_OPCODE, INST_STRH_1_MASK, "STRH <Rt>,[<Rn>,<Rm>]" },
{ INST_SUBS_OPCODE, INST_SUBS_MASK, "SUBS <Rd>,<Rn>,#<imm3>" },
{ INST_SUBS_1_OPCODE, INST_SUBS_1_MASK, "SUBS <Rdn>,#<imm8>" },
{ INST_SUBS_2_OPCODE, INST_SUBS_2_MASK, "SUBS <Rd>,<Rn>,<Rm>" },
{ INST_SUB_OPCODE, INST_SUB_MASK, "SUB SP,SP,#<imm7>" },
{ INST_SVC_OPCODE, INST_SVC_MASK, "SVC #<imm8>" },
{ INST_SXTB_OPCODE, INST_SXTB_MASK, "SXTB <Rd>,<Rm>" },
{ INST_SXTH_OPCODE, INST_SXTH_MASK, "SXTH <Rd>,<Rm>" },
{ INST_TST_OPCODE, INST_TST_MASK, "TST <Rn>,<Rm>" },
{ INST_UDF_OPCODE, INST_UDF_MASK, "UDF #<imm8>" },
{ INST_UDF_W_OPCODE, INST_UDF_W_MASK, "UDF_W #<imm16>" },
{ INST_UXTB_OPCODE, INST_UXTB_MASK, "UXTB <Rd>,<Rm>" },
{ INST_UXTH_OPCODE, INST_UXTH_MASK, "UXTH <Rd>,<Rm>" },
{ INST_WFE_OPCODE, INST_WFE_MASK, "WFE" },
{ INST_WFI_OPCODE, INST_WFI_MASK, "WFI" },
{ INST_YIELD_OPCODE, INST_YIELD_MASK, "YIELD" },
{ 0, 0, 0 }
};

#endif

