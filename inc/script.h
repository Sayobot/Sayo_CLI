#pragma once

#define S_NOP 1
#define S_JMP 2
#define S_SJMP 3
#define S_AJMP 4
#define S_SLEEP_X256 5         //长延迟
#define S_SLEEP 6              //短延迟
#define S_SLEEP_RAND_X256 7    //长随机延迟
#define S_SLEEP_RAND 8         //随机延迟
#define S_SLEEP_X256_VAL 9     //延时长 变量
#define S_SLEEP_VAL 10         //延时短 变量
#define S_SLEEP_RAND_X8_VAL 11 //延时随机 变量
#define S_SLEEP_RAND_VAL 12    //延时随机 变量

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

#define S_C2K 0x30

#define S_WHILE_UP 0xF9
#define S_WHILE_DOWN 0xFA //按键抬起之前阻塞
#define S_IF_UP_EXIT 0xFB
#define S_IF_DOWN_EXIT 0xFC
#define S_IF_KA_EXIT 0xFD
#define S_RES 0xFE
#define S_EXIT 0xFF
#define S_EXIT_NOT_CLEAN 0

#define S_JZ 0x40
#define S_JNZ 0x41
#define S_DJNZ 0x42
#define S_CJNE 0x43
#define S_CALL 0x44
#define S_RET 0x45
#define S_ANL 0x46
#define S_ANLD 0x47

#define S_ADD_DPTR_DATA 0x48
#define S_ADD_R_DATA 0x49
#define S_ADD_DPTR_R 0x48
#define S_ADD_R_R 0x49
#define S_DEC_DATA 0x4c
#define S_DEC 0x4d

#define S_DJNZ_VAL 0x52

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

#define CV0 \
    {       \
    }
#define CV1                                  \
    {                                        \
        this->script[num].value[0] = buf[i]; \
        i++;                                 \
    }
#define CV2                                  \
    {                                        \
        this->script[num].value[0] = buf[i]; \
        i++;                                 \
        this->script[num].value[1] = buf[i]; \
        i++;                                 \
    }
#define CV3                                  \
    {                                        \
        this->script[num].value[0] = buf[i]; \
        i++;                                 \
        this->script[num].value[1] = buf[i]; \
        i++;                                 \
        this->script[num].value[2] = buf[i]; \
        i++;                                 \
    }
#define CV4                                  \
    {                                        \
        this->script[num].value[0] = buf[i]; \
        i++;                                 \
        this->script[num].value[1] = buf[i]; \
        i++;                                 \
        this->script[num].value[2] = buf[i]; \
        i++;                                 \
        this->script[num].value[3] = buf[i]; \
        i++;                                 \
    }

#define WCV0 \
    {        \
    }
#define WCV1                                  \
    {                                         \
        buf[i] = this->script[step].value[0]; \
        i++;                                  \
    }
#define WCV2                                  \
    {                                         \
        buf[i] = this->script[step].value[0]; \
        i++;                                  \
        buf[i] = this->script[step].value[1]; \
        i++;                                  \
    }
#define WCV3                                  \
    {                                         \
        buf[i] = this->script[step].value[0]; \
        i++;                                  \
        buf[i] = this->script[step].value[1]; \
        i++;                                  \
        buf[i] = this->script[step].value[2]; \
        i++;                                  \
    }
#define WCV4                                  \
    {                                         \
        buf[i] = this->script[step].value[0]; \
        i++;                                  \
        buf[i] = this->script[step].value[1]; \
        i++;                                  \
        buf[i] = this->script[step].value[2]; \
        i++;                                  \
        buf[i] = this->script[step].value[3]; \
        i++;                                  \
    }
