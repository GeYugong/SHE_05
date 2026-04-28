#include "timer.h"
#include "tcxo.h"
#include "chip_core_irq.h"
#include "common_def.h"
#include "soc_osal.h"
#include "app_init.h"

#define CLOCK_TIMER_INDEX          1
#define CLOCK_TIMER_PRIO           1
#define CLOCK_TIMER_PERIOD_US      1000000

#define CLOCK_TASK_PRIO            24
#define CLOCK_TASK_STACK_SIZE      0x1000
#define CLOCK_TASK_SLEEP_MS        5

static timer_handle_t g_clock_timer = 0;

static volatile uint8_t g_print_flag = 0;
static volatile uint8_t g_hour = 0;
static volatile uint8_t g_min = 0;
static volatile uint8_t g_sec = 0;

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

    g_print_flag = 1;

    uapi_timer_start(g_clock_timer,
                     CLOCK_TIMER_PERIOD_US,
                     clock_timer_callback,
                     0);
}

static void *clock_timer_task(const char *arg)
{
    unused(arg);

    uapi_timer_init();
    uapi_timer_adapter(CLOCK_TIMER_INDEX, TIMER_1_IRQN, CLOCK_TIMER_PRIO);
    uapi_timer_create(CLOCK_TIMER_INDEX, &g_clock_timer);

    osal_printk("1s clock timer start.\r\n");
    osal_printk("%02d:%02d:%02d\r\n", g_hour, g_min, g_sec);

    uapi_timer_start(g_clock_timer,
                     CLOCK_TIMER_PERIOD_US,
                     clock_timer_callback,
                     0);

    while (1) {
        if (g_print_flag) {
            g_print_flag = 0;

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