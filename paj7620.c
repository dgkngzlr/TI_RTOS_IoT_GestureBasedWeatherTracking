/*
 * paj7620.c
 *
 *  Created on: 10 Ara 2021
 *      Author: Dogukan
 */
/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/knl/Task.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>

/* Board Header file */
#include "paj7620.h"
#include "Board.h"

const unsigned short initRegisterArray[] = {

        0xEF00,
        0x3229,
        0x3301,
        0x3400,
        0x3501,
        0x3600,
        0x3707,
        0x3817,
        0x3906,
        0x3A12,
        0x3F00,
        0x4002,
        0x41FF,
        0x4201,
        0x462D,
        0x470F,
        0x483C,
        0x4900,
        0x4A1E,
        0x4B00,
        0x4C20,
        0x4D00,
        0x4E1A,
        0x4F14,
        0x5000,
        0x5110,
        0x5200,
        0x5C02,
        0x5D00,
        0x5E10,
        0x5F3F,
        0x6027,
        0x6128,
        0x6200,
        0x6303,
        0x64F7,
        0x6503,
        0x66D9,
        0x6703,
        0x6801,
        0x69C8,
        0x6A40,
        0x6D04,
        0x6E00,
        0x6F00,
        0x7080,
        0x7100,
        0x7200,
        0x7300,
        0x74F0,
        0x7500,
        0x8042,
        0x8144,
        0x8204,
        0x8320,
        0x8420,
        0x8500,
        0x8610,
        0x8700,
        0x8805,
        0x8918,
        0x8A10,
        0x8B01,
        0x8C37,
        0x8D00,
        0x8EF0,
        0x8F81,
        0x9006,
        0x9106,
        0x921E,
        0x930D,
        0x940A,
        0x950A,
        0x960C,
        0x9705,
        0x980A,
        0x9941,
        0x9A14,
        0x9B0A,
        0x9C3F,
        0x9D33,
        0x9EAE,
        0x9FF9,
        0xA048,
        0xA113,
        0xA210,
        0xA308,
        0xA430,
        0xA519,
        0xA610,
        0xA708,
        0xA824,
        0xA904,
        0xAA1E,
        0xAB1E,
        0xCC19,
        0xCD0B,
        0xCE13,
        0xCF64,
        0xD021,
        0xD10F,
        0xD288,
        0xE001,
        0xE104,
        0xE241,
        0xE3D6,
        0xE400,
        0xE50C,
        0xE60A,
        0xE700,
        0xE800,
        0xE900,
        0xEE07,
        0xEF01,
        0x001E,
        0x011E,
        0x020F,
        0x0310,
        0x0402,
        0x0500,
        0x06B0,
        0x0704,
        0x080D,
        0x090E,
        0x0A9C,
        0x0B04,
        0x0C05,
        0x0D0F,
        0x0E02,
        0x0F12,
        0x1002,
        0x1102,
        0x1200,
        0x1301,
        0x1405,
        0x1507,
        0x1605,
        0x1707,
        0x1801,
        0x1904,
        0x1A05,
        0x1B0C,
        0x1C2A,
        0x1D01,
        0x1E00,
        0x2100,
        0x2200,
        0x2300,
        0x2501,
        0x2600,
        0x2739,
        0x287F,
        0x2908,
        0x3003,
        0x3100,
        0x321A,
        0x331A,
        0x3407,
        0x3507,
        0x3601,
        0x37FF,
        0x3836,
        0x3907,
        0x3A00,
        0x3EFF,
        0x3F00,
        0x4077,
        0x4140,
        0x4200,
        0x4330,
        0x44A0,
        0x455C,
        0x4600,
        0x4700,
        0x4858,
        0x4A1E,
        0x4B1E,
        0x4C00,
        0x4D00,
        0x4EA0,
        0x4F80,
        0x5000,
        0x5100,
        0x5200,
        0x5300,
        0x5400,
        0x5780,
        0x5910,
        0x5A08,
        0x5B94,
        0x5CE8,
        0x5D08,
        0x5E3D,
        0x5F99,
        0x6045,
        0x6140,
        0x632D,
        0x6402,
        0x6596,
        0x6600,
        0x6797,
        0x6801,
        0x69CD,
        0x6A01,
        0x6BB0,
        0x6C04,
        0x6D2C,
        0x6E01,
        0x6F32,
        0x7100,
        0x7201,
        0x7335,
        0x7400,
        0x7533,
        0x7631,
        0x7701,
        0x7C84,
        0x7D03,
        0x7E01};

uint8_t paj7620Init(I2C_Handle i2c, I2C_Transaction i2cTransaction){
    uint8_t recv_buf[10];
    uint8_t high, low;
    uint16_t word;
    uint8_t ret1, ret2;
    uint8_t suc = 1;

    int i = 0;

    paj7620SelectBank(i2c, i2cTransaction, 0);
    paj7620SelectBank(i2c, i2cTransaction, 0);

    ret1 = paj7620ReadReg(i2c, i2cTransaction, 0, 1, recv_buf);
    ret2 = paj7620ReadReg(i2c, i2cTransaction, 1, 1, recv_buf);

    if ((ret1 != 0x20) || (ret2 != 0x76)) {
        return 0;
    }
    if (ret1 == 0x20) {
        System_printf("Wake up finish\n");
        System_flush();
    }
    System_printf("Initializing ...\n");
    System_flush();
    for (i = 0; i < INIT_REG_ARRAY_SIZE; i++){
        word = initRegisterArray[i];
        high = (word & 0xFF00) >> 8;
        low = (word & 0x00FF);

        suc = paj7620WriteReg(i2c,i2cTransaction,high, low); // HIGH ADDR, LOW CMD

        if (!suc){
            System_printf("Init fault\n");
            System_flush();
            return suc;
        }
    }
    System_printf("[OK] Sensor initialized\n");
    System_flush();
    paj7620SelectBank(i2c, i2cTransaction, 1);
    suc = paj7620WriteReg(i2c, i2cTransaction, PAJ7620_ADDR_GAMING_MODE, PAJ7620_GAMING_VAL);
    paj7620SelectBank(i2c, i2cTransaction,0);
    return suc;
}

void paj7620SelectBank(I2C_Handle i2c, I2C_Transaction i2cTransaction, int bank){

    if (bank == 0){
       paj7620WriteReg(i2c, i2cTransaction, PAJ7620_REGITER_BANK_SEL, PAJ7620_BANK0);
    }else{
       paj7620WriteReg(i2c, i2cTransaction,PAJ7620_REGITER_BANK_SEL, PAJ7620_BANK1);
    }
}

uint8_t paj7620WriteReg(I2C_Handle i2c, I2C_Transaction i2cTransaction,uint8_t addr, uint8_t cmd){
    uint8_t txBuffer[2];
    uint8_t rxBuffer[2];
    uint8_t suc = 0;

    txBuffer[0] = addr;
    txBuffer[1] = cmd;
    i2cTransaction.slaveAddress = PAJ7620_ID;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 0;

    if (I2C_transfer(i2c, &i2cTransaction)) {
            suc = 1;
            //System_printf("WRITE >> ADDR: %d -- CMD: %d SUC:%d\n", txBuffer[0], txBuffer[1], suc);
    }
    else {
        System_printf("I2C Bus fault\n");
        suc = 0;
    }
    System_flush();
    return suc;
}
uint8_t paj7620ReadReg(I2C_Handle i2c, I2C_Transaction i2cTransaction, uint8_t addr, uint8_t no_of_bytes, uint8_t *buf){
    uint8_t txBuffer[1];
    txBuffer[0] = addr; // Register addr
    i2cTransaction.slaveAddress = PAJ7620_ID;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = buf;
    i2cTransaction.readCount = no_of_bytes;

    if (I2C_transfer(i2c, &i2cTransaction)) {
            //System_printf("READ >> ADDR: %d -- VAL:%d\n", addr, buf[0]);
        }
    else {
        System_printf("I2C Bus fault\n");
    }
    System_flush();

    return *buf;
}


