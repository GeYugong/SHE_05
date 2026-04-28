/**
 * Timer clock demo
 * 功能：使用硬件定时器每 1 秒触发一次回调，并输出 24 小时制时钟
 */

#include "timer.h"
#include "tcxo.h"
#include "chip_core_irq.h"
#include "common_def.h"
#include "soc_osal.h"
#include "app_init.h"

#define CLOCK_TIMER_INDEX          1
#define CLOCK_TIMER_PRIO           1
#define CLOCK_TIMER_PERIOD_US      1000000   /* 1 秒 = 1000000 微秒 */

#define CLOCK_TASK_PRIO            24
#define CLOCK_TASK_STACK_SIZE      0x1000
#define CLOCK_TASK_SLEEP_MS        5

static timer_handle_t g_clock_timer = 0;

static volatile uint32_t g_tick_count = 0;
static volatile uint8_t g_hour = 0;
static volatile uint8_t g_min = 0;
static volatile uint8_t g_sec = 0;

/* 每 1 秒进入一次该回调函数 */
static void clock_timer_callback(uintptr_t data)
{
    unused(data);

    g_sec++;

    if (g_sec >= 60) {
        g_sec = 0;
        g_min++;
    }

    if (g_min >= 60) {
        g_min = 0;
        g_hour++;
    }

    if (g_hour >= 24) {
        g_hour = 0;
    }

    g_tick_count++;

    /* 本实验要求回调函数中涉及重新开启定时器 */
    uapi_timer_start(g_clock_timer, CLOCK_TIMER_PERIOD_US, clock_timer_callback, 0);
}

static void *clock_timer_task(const char *arg)
{
    unused(arg);

    uint32_t last_tick = 0;

    uapi_timer_init();

    /* 将定时器 1 绑定到 TIMER_1_IRQN 中断 */
    uapi_timer_adapter(CLOCK_TIMER_INDEX, TIMER_1_IRQN, CLOCK_TIMER_PRIO);

    /* 创建定时器 */
    uapi_timer_create(CLOCK_TIMER_INDEX, &g_clock_timer);

    osal_printk("Clock timer start!\r\n");
    osal_printk("%02d:%02d:%02d\r\n", g_hour, g_min, g_sec);

    /* 第一次启动定时器 */
    uapi_timer_start(g_clock_timer, CLOCK_TIMER_PERIOD_US, clock_timer_callback, 0);

    while (1) {
        if (last_tick != g_tick_count) {
            last_tick = g_tick_count;

            osal_printk("%02d:%02d:%02d\r\n",
                        (int)g_hour,
                        (int)g_min,
                        (int)g_sec);
        }

        osal_msleep(CLOCK_TASK_SLEEP_MS);
    }

    return NULL;
}

static void clock_timer_entry(void)
{
    osal_task *task_handle = NULL;

    osal_kthread_lock();

    task_handle = osal_kthread_create((osal_kthread_handler)clock_timer_task,
                                      0,
                                      "ClockTimerTask",
                                      CLOCK_TASK_STACK_SIZE);

    if (task_handle != NULL) {
        osal_kthread_set_priority(task_handle, CLOCK_TASK_PRIO);
        osal_kfree(task_handle);
    }

    osal_kthread_unlock();
}

app_run(clock_timer_entry);