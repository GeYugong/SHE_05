#include "timer.h"
#include "tcxo.h"
#include "chip_core_irq.h"
#include "common_def.h"
#include "soc_osal.h"
#include "app_init.h"
#include "ins5699s.h"

#define RTC_TIMER_INDEX          1
#define RTC_TIMER_PRIO           1
#define RTC_TIMER_PERIOD_US      1000000

#define RTC_TASK_PRIO            24
#define RTC_TASK_STACK_SIZE      0x1000
#define RTC_TASK_SLEEP_MS        5

static timer_handle_t g_rtc_timer = 0;
static volatile uint8_t g_rtc_read_flag = 0;

static void rtc_timer_callback(uintptr_t data)
{
    unused(data);

    g_rtc_read_flag = 1;

    uapi_timer_start(g_rtc_timer,
                     RTC_TIMER_PERIOD_US,
                     rtc_timer_callback,
                     0);
}

static void print_rtc_time(ins5699s_time time)
{
    osal_printk("RTC Time: 20%02d-%02d-%02d  %02d:%02d:%02d  week=%d\r\n",
                (int)time.year,
                (int)time.month,
                (int)time.day,
                (int)time.hour,
                (int)time.min,
                (int)time.sec,
                (int)time.week);
}

static void *rtc_task(const char *arg)
{
    unused(arg);

    ins5699s_time set_time = {
        .sec = 0,
        .min = 30,
        .hour = 20,
        .week = 2,
        .day = 28,
        .month = 4,
        .year = 26
    };

    uapi_timer_init();
    uapi_timer_adapter(RTC_TIMER_INDEX, TIMER_1_IRQN, RTC_TIMER_PRIO);
    uapi_timer_create(RTC_TIMER_INDEX, &g_rtc_timer);

    ins5699s_init();

    osal_printk("Set INS5699S RTC time...\r\n");
    ins5699s_SetTime(set_time);

    osal_msleep(200);

    osal_printk("Start reading INS5699S RTC every 1 second.\r\n");

    uapi_timer_start(g_rtc_timer,
                     RTC_TIMER_PERIOD_US,
                     rtc_timer_callback,
                     0);

    while (1) {
        if (g_rtc_read_flag) {
            g_rtc_read_flag = 0;

            ins5699s_time now = ins5699s_GetTime();
            print_rtc_time(now);
        }

        osal_msleep(RTC_TASK_SLEEP_MS);
    }

    return NULL;
}

static void rtc_entry(void)
{
    osal_task *task_handle = NULL;

    osal_kthread_lock();

    task_handle = osal_kthread_create((osal_kthread_handler)rtc_task,
                                      0,
                                      "RtcTask",
                                      RTC_TASK_STACK_SIZE);

    if (task_handle != NULL) {
        osal_kthread_set_priority(task_handle, RTC_TASK_PRIO);
        osal_kfree(task_handle);
    }

    osal_kthread_unlock();
}

app_run(rtc_entry);