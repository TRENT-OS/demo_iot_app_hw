/*
   *
   *  Copyright (C) 2020, Hensoldt Cyber GmbH
*/

#include "time_server.h"
#include <platsupport/plat/timer.h>
#include <sel4utils/sel4_zf_logif.h>

#include <stdbool.h>

#include <camkes.h>



#define NS_IN_SECOND                1000000000ULL
#define NS_IN_MSEC                  1000000ULL

#define DATABUFFER_TIME_SERVER      (void *)reg
#define DELAY_OFFSET_MSEC           0



static ttc_config_t _ttc_config_TTC0_TIMER1;
static ttc_config_t _ttc_config_TTC0_TIMER2;
static ttc_t _ttc_TTC0_TIMER1;
static ttc_t _ttc_TTC0_TIMER2;
static uint64_t _timestamp_relative;



// foreward declaration
static void _set_timestamp_relative(void);



static void
_set_timestamp_relative(void)
{
    // update every 1 second
    _timestamp_relative += 1;
}



uint64_t
get_timestamp_relative(void)
{
    return _timestamp_relative;
}



void
Time_server_init()
{
    int error;

    /*
     * Provide hardware info to platsupport.
     * Note, The following only works for zynq7000 platform. Other platforms may
     * require other info. Check the definition of time_config_t and manuals.
     */
    // TTC0_TIMER1
    _ttc_config_TTC0_TIMER1.vaddr = DATABUFFER_TIME_SERVER;
    _ttc_config_TTC0_TIMER1.clk_src = 0;
    _ttc_config_TTC0_TIMER1.id = TTC0_TIMER1;

    error = ttc_init(&_ttc_TTC0_TIMER1, _ttc_config_TTC0_TIMER1);
    assert(error == 0);

    // TTC0_TIMER2
    _ttc_config_TTC0_TIMER2.vaddr = DATABUFFER_TIME_SERVER;
    _ttc_config_TTC0_TIMER2.clk_src = 0;
    _ttc_config_TTC0_TIMER2.id = TTC0_TIMER2;

    error = ttc_init(&_ttc_TTC0_TIMER2, _ttc_config_TTC0_TIMER2);
    assert(error == 0);

    _timestamp_relative = 0;
}



void
Time_server_tick_sec(void)
{
    ttc_set_timeout(&_ttc_TTC0_TIMER1,
                    ((DELAY_OFFSET_MSEC + 1000) * NS_IN_SECOND) / 1000, false);

    ttc_start(&_ttc_TTC0_TIMER1);

    int error = sem_ticks_wait();
    ZF_LOGF_IF(error != 0, "Failed to wait on semaphore");
}



void
Time_server_sleep(unsigned int msec)
{
    ttc_set_timeout(&_ttc_TTC0_TIMER2,
                    ((uint64_t)msec * (DELAY_OFFSET_MSEC + 1000) * NS_IN_MSEC) / 1000, false);

    ttc_start(&_ttc_TTC0_TIMER2);

    int error = sem_sleep_wait();
    ZF_LOGF_IF(error != 0, "Failed to wait on semaphore");
}



/* this callback handler is meant to be invoked when the first interrupt
 * arrives on the interrupt event interface.
 * Note: the callback handler must be explicitly registered before the
 * callback will be invoked.
 * Also the registration is one-shot only, if it wants to be invoked
 * when a new interrupt arrives then it must re-register itself.  Or it can
 * also register a different handler.
 */
void
irq_ticks_handle(void)
{
    int error;

    ttc_handle_irq(&_ttc_TTC0_TIMER1);

    ttc_stop(&_ttc_TTC0_TIMER1);

    /* Signal the RPC interface. */
    error = sem_ticks_post();
    ZF_LOGF_IF(error != 0, "Failed to post to semaphore");

    error = irq_ticks_acknowledge();
    ZF_LOGF_IF(error != 0, "Failed to acknowledge interrupt");

    _set_timestamp_relative();
}



void
irq_sleep_handle(void)
{
    int error;

    ttc_handle_irq(&_ttc_TTC0_TIMER2);

    ttc_stop(&_ttc_TTC0_TIMER2);

    /* Signal the RPC interface. */
    error = sem_sleep_post();
    ZF_LOGF_IF(error != 0, "Failed to post to semaphore");

    error = irq_sleep_acknowledge();
    ZF_LOGF_IF(error != 0, "Failed to acknowledge interrupt");
}
