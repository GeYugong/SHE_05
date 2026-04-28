#include "pinctrl.h"
#include "common_def.h"
#include "soc_osal.h"
#include "gpio.h"
#include "hal_gpio.h"
#include "app_init.h"
#include "bh1750.h"
#include "i2c.h"

#ifndef CONFIG_I2C_MASTER_BUS_ID
#define CONFIG_I2C_MASTER_BUS_ID        1
#endif
#ifndef CONFIG_I2C_SCL_MASTER_PIN
#define CONFIG_I2C_SCL_MASTER_PIN       16
#endif
#ifndef CONFIG_I2C_SDA_MASTER_PIN
#define CONFIG_I2C_SDA_MASTER_PIN       15
#endif
#ifndef CONFIG_I2C_MASTER_PIN_MODE
#define CONFIG_I2C_MASTER_PIN_MODE      2
#endif

uint16_t bh1750_buff_H = 0;
uint16_t bh1750_buff_L = 0;

void bh1750_SendCMD(uint8_t cmd)
{
    uint8_t buffer[] = {cmd};
    i2c_data_t data = {0};
    data.send_buf = buffer;
    data.send_len = sizeof(buffer);
    errcode_t ret = uapi_i2c_master_write(CONFIG_I2C_MASTER_BUS_ID,
                                          BH1750_ADDR >> 1, &data);
    if (ret != 0) {
        printf("BH1750:I2cWriteCMD(%02X) failed, %0X!\n", cmd, ret);
        return;
    }
}

void bh1750_ReadData(void)
{
    uint8_t buffer[2] = {0};
    i2c_data_t data;
    data.receive_len = sizeof(buffer);
    data.receive_buf = buffer;
    errcode_t ret = uapi_i2c_master_read(CONFIG_I2C_MASTER_BUS_ID,
                                         BH1750_ADDR >> 1, &data);
    if (ret != 0) {
        printf("BH1750:I2cRead(len:%d) failed, %0X!\n", data.receive_len, ret);
        return;
    }
    bh1750_buff_H = data.receive_buf[0];
    bh1750_buff_L = data.receive_buf[1];
}

uint16_t bh1750_GetLightIntensity(void)
{
    bh1750_ReadData();
    uint16_t data;
    data = (bh1750_buff_H << 8) | bh1750_buff_L;
    return (data * BH1750_RES) / 1.2;
}

void bh1750_init(void)
{
    bh1750_SendCMD(BH1750_POWER_ON);
    osal_msleep(200);
    bh1750_SendCMD(BH1750_CONTINUE_H_MODE);
    osal_msleep(200);
    osal_printk("BH1750 Init SUCC!\r\n");
}