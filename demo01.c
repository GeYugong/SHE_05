/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * Description: DEMO01 — BH1750 + CW2015 I2C read demo.
 */
#include "pinctrl.h"
#include "i2c.h"
#include "soc_osal.h"
#include "app_init.h"
#include "bh1750.h"
#include "cw2015.h"

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

#define I2C_MASTER_ADDR         0x0
#define I2C_SET_BAUDRATE        400000
#define DEMO01_TASK_DURATION_MS 10000

#define DEMO01_TASK_PRIO        24
#define DEMO01_TASK_STACK_SIZE  0x1000

static void app_i2c_init_pin(void)
{
    uapi_pin_set_mode(CONFIG_I2C_SCL_MASTER_PIN, CONFIG_I2C_MASTER_PIN_MODE);
    uapi_pin_set_mode(CONFIG_I2C_SDA_MASTER_PIN, CONFIG_I2C_MASTER_PIN_MODE);
}

static void *demo01_task(const char *arg)
{
    unused(arg);

    uint32_t baudrate = I2C_SET_BAUDRATE;
    uint8_t hscode = I2C_MASTER_ADDR;

    /* I2C master init. */
    app_i2c_init_pin();
    uapi_i2c_master_init(CONFIG_I2C_MASTER_BUS_ID, baudrate, hscode);

    /* ========== 实验 1：BH1750 ========== */
    // bh1750_init();
    // while (1) {
    //     osal_msleep(DEMO01_TASK_DURATION_MS);
    //     uint16_t lightness = bh1750_GetLightIntensity();
    //     osal_printk("BH1750:%05d\r\n", lightness);
    // }

    //========== 实验 2：CW2015（做第二个实验时，注释掉上面 BH1750 那段，取消下面注释）==========
    cw2015_init();
    while (1) {
        osal_msleep(DEMO01_TASK_DURATION_MS);
        uint32_t BatteryVoltage = cw2015_GetBatteryVoltage();
        osal_printk("CW2015:%010d\r\n", BatteryVoltage);
    }

    return NULL;
}

static void demo01_entry(void)
{
    osal_task *task_handle = NULL;
    osal_kthread_lock();
    task_handle = osal_kthread_create((osal_kthread_handler)demo01_task, 0,
                                      "Demo01Task", DEMO01_TASK_STACK_SIZE);
    if (task_handle != NULL) {
        osal_kthread_set_priority(task_handle, DEMO01_TASK_PRIO);
        osal_kfree(task_handle);
    }
    osal_kthread_unlock();
}

/* Run the demo01_entry. */
app_run(demo01_entry);