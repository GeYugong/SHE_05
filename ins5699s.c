#include "pinctrl.h"
#include "common_def.h"
#include "soc_osal.h"
#include "i2c.h"
#include "ins5699s.h"

#define I2C_MASTER_BUS_ID      1
#define I2C_BAUDRATE           400000
#define I2C_HS_MASTER_CODE     0

#define I2C_SCL_PIN            15
#define I2C_SDA_PIN            16
#define I2C_PIN_MODE           2

static uint8_t dec_to_bcd(uint8_t val)
{
    return (uint8_t)(((val / 10) << 4) | (val % 10));
}

static uint8_t bcd_to_dec(uint8_t val)
{
    return (uint8_t)(((val >> 4) * 10) + (val & 0x0F));
}

/* week: 0=周日，1=周一，2=周二，...，6=周六 */
static uint8_t week_to_reg(uint8_t week)
{
    if (week > 6) {
        week = 0;
    }

    return (uint8_t)(1U << week);
}

static uint8_t reg_to_week(uint8_t reg)
{
    for (uint8_t i = 0; i < 7; i++) {
        if (reg == (uint8_t)(1U << i)) {
            return i;
        }
    }

    return 0;
}

static void ins5699s_SendREG(uint8_t reg, uint8_t reg_data)
{
    uint8_t buffer[2] = {reg, reg_data};

    i2c_data_t data = {0};
    data.send_buf = buffer;
    data.send_len = sizeof(buffer);

    errcode_t ret = uapi_i2c_master_write(I2C_MASTER_BUS_ID,
                                          INS5699S_ADDR >> 1,
                                          &data);

    if (ret != 0) {
        osal_printk("INS5699S write reg 0x%02X failed, ret = 0x%X\r\n", reg, ret);
    }
}

static uint8_t ins5699s_ReadREG(uint8_t reg)
{
    uint8_t send_buffer[1] = {reg};
    uint8_t read_buffer[1] = {0};

    i2c_data_t data = {0};
    data.send_buf = send_buffer;
    data.send_len = sizeof(send_buffer);
    data.receive_buf = read_buffer;
    data.receive_len = sizeof(read_buffer);

    errcode_t ret = uapi_i2c_master_writeread(I2C_MASTER_BUS_ID,
                                              INS5699S_ADDR >> 1,
                                              &data);

    if (ret != 0) {
        osal_printk("INS5699S read reg 0x%02X failed, ret = 0x%X\r\n", reg, ret);
        return 0;
    }

    return read_buffer[0];
}

void ins5699s_init(void)
{
    errcode_t ret;

    uapi_pin_set_mode(I2C_SCL_PIN, I2C_PIN_MODE);
    uapi_pin_set_mode(I2C_SDA_PIN, I2C_PIN_MODE);

    ret = uapi_i2c_master_init(I2C_MASTER_BUS_ID,
                               I2C_BAUDRATE,
                               I2C_HS_MASTER_CODE);

    if (ret != 0) {
        osal_printk("INS5699S I2C init failed, ret = 0x%X\r\n", ret);
        return;
    }

    osal_msleep(100);
    osal_printk("INS5699S I2C init success.\r\n");
}

void ins5699s_SetTime(ins5699s_time time)
{
    ins5699s_SendREG(INS5699S_REG_SEC,   dec_to_bcd(time.sec));
    ins5699s_SendREG(INS5699S_REG_MIN,   dec_to_bcd(time.min));
    ins5699s_SendREG(INS5699S_REG_HOUR,  dec_to_bcd(time.hour));
    ins5699s_SendREG(INS5699S_REG_WEEK,  week_to_reg(time.week));
    ins5699s_SendREG(INS5699S_REG_DAY,   dec_to_bcd(time.day));
    ins5699s_SendREG(INS5699S_REG_MONTH, dec_to_bcd(time.month));
    ins5699s_SendREG(INS5699S_REG_YEAR,  dec_to_bcd(time.year));
}

ins5699s_time ins5699s_GetTime(void)
{
    ins5699s_time time = {0};

    time.sec   = bcd_to_dec(ins5699s_ReadREG(INS5699S_REG_SEC));
    time.min   = bcd_to_dec(ins5699s_ReadREG(INS5699S_REG_MIN));
    time.hour  = bcd_to_dec(ins5699s_ReadREG(INS5699S_REG_HOUR));
    time.week  = reg_to_week(ins5699s_ReadREG(INS5699S_REG_WEEK));
    time.day   = bcd_to_dec(ins5699s_ReadREG(INS5699S_REG_DAY));
    time.month = bcd_to_dec(ins5699s_ReadREG(INS5699S_REG_MONTH));
    time.year  = bcd_to_dec(ins5699s_ReadREG(INS5699S_REG_YEAR));

    return time;
}