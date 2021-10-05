#pragma once

#define S_TYPE_UCHAR 0
#define S_TYPE_USHORT 1
#define S_TYPE_ULONG 2
#define S_TYPE_CHAR 3
#define S_TYPE_SHORT 4
#define S_TYPE_LONG 5

#define S_KEY_LAY (0x7)
#define S_KEY_ENC (1<<3)
#define S_JOG_MODE (1 << 4)
#define S_CY (1 << 5)
#define S_RUN (1 << 7)

#define S_NOP 1
#define S_JMP 2
#define S_SJMP 3
#define S_AJMP 4
#define S_SLEEP_X256 5
#define S_SLEEP 6
#define S_SLEEP_RAND_X256 7
#define S_SLEEP_RAND 8
#define S_SLEEP_X256_VAL 9
#define S_SLEEP_VAL 10
#define S_SLEEP_RAND_X8_VAL 11
#define S_SLEEP_RAND_VAL 12

#define S_SK 0x10
#define S_GK 0X11
#define S_MK 0X12
#define S_MU 0x13

#define S_SK_VAL 0x14
#define S_GK_VAL 0X15
#define S_MK_VAL 0X16
#define S_MU_VAL 0x17

#define S_USK 0X18
#define S_UGK 0X19
#define S_UMK 0X1A
#define S_UMU 0x1B

#define S_USK_VAL 0X1C
#define S_UGK_VAL 0X1D
#define S_UMK_VAL 0X1E
#define S_UMU_VAL 0x1F

#define S_UPDATE 0x20

#define MO_X 0
#define MO_Y 1
#define MO_S 2

#define S_MO_XYZ 0x21
#define S_MO_XYZ_VAL 0x22
#define S_GA_XYZ 0x23
#define S_GA_XYZ_VAL 0x24
#define S_TB_XY 0x25
#define S_TB_XY_VAL 0x26



#define S_GAK 0x2C
#define S_GAK_VAL 0x2D
#define S_UGAK 0x2E
#define S_UGAK_VAL 0x2F

#define S_C2K 0x30
#define S_U2K 0x31
#define S_C2K_RAND 0x32

#define S_JFC 0x48
#define S_JFNC 0x49
#define S_JFZ 0x4a
#define S_JFNZ 0x4b
#define S_DJFNZ 0x4c
#define S_CJFNE 0x4d       //CY

#define S_JC 0x4e
#define S_JNC 0x4f
#define S_JZ 0x50
#define S_JNZ 0x51
#define S_DJNZ 0x52
#define S_CJNE 0x53       //CY
#define S_CALL 0x54
#define S_RET 0x55
#define S_ANL 0x56
#define S_ANLD 0x57

#define S_ADD 0x58
#define S_ADDD 0x59
#define S_SUB 0x5A
#define S_SUBD 0x5B
#define S_ORL 0x5c
#define S_ORLD 0x5d
#define S_DEC 0x5e
#define S_INC 0x5F

#define S_MUL 0x60
#define S_DIV 0x61
#define S_XRL 0x62
#define S_XRLD 0x63
#define S_RL 0x64
#define S_RLD 0x65
#define S_RR 0x66
#define S_RRD 0x67

#define S_CLR 0x68
#define S_CPL 0x69
#define S_XCH 0x6A
#define S_PUSH 0x6c
#define S_POP 0x6d
#define S_MOV 0x6e
#define S_MOVD 0x6f


#define S_LED_CTRL 0xE0
enum S_LED_CTRL_NUM
{
	LED_SELECT = 0,
	LED_OFF = 128,
	LED_ON,
	LED_RELOAD = 0xff
};
#define S_LED_COL 0xE1

#define S_WHILE_UPDATE 0xF4

#define S_MOV_PC2REG 0xF6
#define S_VALUE_RELOAD 0xF7
#define S_MODE_JOG 0xF8
#define S_WHILE_UP 0xF9
#define S_WHILE_DOWN 0xFA
#define S_IF_UP_EXIT 0xFB
#define S_IF_DOWN_EXIT 0xFC
#define S_IF_KA_EXIT 0xFD
#define S_RES 0xFE
#define S_EXIT 0xFF
#define S_EXIT_NOT_CLEAN 0

#define S_DATA_V_0 0
#define S_DATA_V_1 1
#define S_DATA_V_2 2
#define S_DATA_V_3 3
#define S_DATA_R0 4
#define S_DATA_R1 5
#define S_DATA_R2 6
#define S_DATA_R3 7
#define S_DATA_RET 8
#define S_DATA_DPTR 9
#define S_DATA_IO 10
#define S_P_R0 11
#define S_P_R1 12
#define S_P_R2 13
#define S_P_R3 14
#define S_P_DPTR 15
#define S_DATA_A 16
#define S_DATA_B 17
#define S_SYS_TIME_MS 18
#define S_SYS_TIME_S 19
#define S_SYS_KBLED 20



#define CV0 {}
#define CV1 {this->script[num].value[0] = buf[i];i++;}
#define CV2 {this->script[num].value[0] = buf[i];i++;this->script[num].value[1] = buf[i];i++;}
#define CV3 {this->script[num].value[0] = buf[i];i++;this->script[num].value[1] = buf[i];i++;this->script[num].value[2] = buf[i];i++;}
#define CV4 {this->script[num].value[0] = buf[i];i++;this->script[num].value[1] = buf[i];i++;this->script[num].value[2] = buf[i];i++;this->script[num].value[3] = buf[i];i++;}

#define WCV0 {}
#define WCV1 {buf[i] = this->script[step].value[0];i++;}
#define WCV2 {buf[i] = this->script[step].value[0];i++;buf[i] = this->script[step].value[1];i++;}
#define WCV3 {buf[i] = this->script[step].value[0];i++;buf[i] = this->script[step].value[1];i++;buf[i] = this->script[step].value[2];i++;}
#define WCV4 {buf[i] = this->script[step].value[0];i++;buf[i] = this->script[step].value[1];i++;buf[i] = this->script[step].value[2];i++;buf[i] = this->script[step].value[3];i++;}

