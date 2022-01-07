/*
 * paj7620.h
 *
 *  Created on: 10 Ara 2021
 *      Author: Dogukan
 */

#ifndef PAJ7620_H_
#define PAJ7620_H_

// REGISTER BANK SELECT
#define PAJ7620_REGITER_BANK_SEL    0xEF    //W

// DEVICE ID
#define PAJ7620_ID  0x73

// REGISTER BANK 0
#define PAJ7620_ADDR_SUSPEND_CMD            0x03    //W
#define PAJ7620_ADDR_GES_PS_DET_MASK_0      0x41    //RW
#define PAJ7620_ADDR_GES_PS_DET_MASK_1      0x42    //RW
#define PAJ7620_ADDR_GES_PS_DET_FLAG_0      0x43    //R
#define PAJ7620_ADDR_GES_PS_DET_FLAG_1      0x44    //R
#define PAJ7620_ADDR_STATE_INDICATOR        0x45    //R
#define PAJ7620_ADDR_PS_HIGH_THRESHOLD      0x69    //RW
#define PAJ7620_ADDR_PS_LOW_THRESHOLD       0x6A    //RW
#define PAJ7620_ADDR_PS_APPROACH_STATE      0x6B    //R
#define PAJ7620_ADDR_PS_RAW_DATA            0x6C    //R

// REGISTER BANK 1
#define PAJ7620_ADDR_PS_GAIN            0x44    //RW
#define PAJ7620_ADDR_GAMING_MODE        0x65    //W
#define PAJ7620_ADDR_IDLE_S1_STEP_0     0x67    //RW
#define PAJ7620_ADDR_IDLE_S1_STEP_1     0x68    //RW
#define PAJ7620_ADDR_IDLE_S2_STEP_0     0x69    //RW
#define PAJ7620_ADDR_IDLE_S2_STEP_1     0x6A    //RW
#define PAJ7620_ADDR_OP_TO_S1_STEP_0    0x6B    //RW
#define PAJ7620_ADDR_OP_TO_S1_STEP_1    0x6C    //RW
#define PAJ7620_ADDR_OP_TO_S2_STEP_0    0x6D    //RW
#define PAJ7620_ADDR_OP_TO_S2_STEP_1    0x6E    //RW
#define PAJ7620_ADDR_OPERATION_ENABLE   0x72    //RW


#define PAJ7620_GAMING_VAL        0x12    //W
// PAJ7620_REGITER_BANK_SEL
#define PAJ7620_BANK0       0x00
#define PAJ7620_BANK1       0x01

// PAJ7620_ADDR_SUSPEND_CMD
#define PAJ7620_I2C_WAKEUP  0x01
#define PAJ7620_I2C_SUSPEND 0x00

// PAJ7620_ADDR_OPERATION_ENABLE
#define PAJ7620_ENABLE      0x01
#define PAJ7620_DISABLE     0x00

typedef enum {
    BANK0 = 0,
    BANK1,
} bank_e;

#define GES_RIGHT_FLAG              0x01
#define GES_LEFT_FLAG               0x02
#define GES_UP_FLAG                 0x04
#define GES_DOWN_FLAG               0x08
#define GES_FORWARD_FLAG            0x10
#define GES_BACKWARD_FLAG           0x20
#define GES_CLOCKWISE_FLAG          0x40
#define GES_COUNT_CLOCKWISE_FLAG    0x80
#define GES_WAVE_FLAG               0x01

#define INIT_REG_ARRAY_SIZE (sizeof(initRegisterArray)/sizeof(initRegisterArray[0]))

uint8_t paj7620Init(I2C_Handle i2c, I2C_Transaction i2cTransaction);
uint8_t paj7620WriteReg(I2C_Handle i2c, I2C_Transaction i2cTransaction,uint8_t addr, uint8_t cmd);
uint8_t paj7620ReadReg(I2C_Handle i2c, I2C_Transaction i2cTransaction, uint8_t addr, uint8_t no_of_bytes, uint8_t *buf);
void paj7620SelectBank(I2C_Handle i2c, I2C_Transaction i2cTransaction,int bank);

#endif /* PAJ7620_H_ */
