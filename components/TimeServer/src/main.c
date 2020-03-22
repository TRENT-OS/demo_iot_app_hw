/*
   *  Main file for the TimeServer component.
   *
   *  Copyright (C) 2020, Hensoldt Cyber GmbH
*/

#include "time_server.h"
#include "timestamp.h"

#include <stdio.h>

#include <camkes.h>



static Timestamp_t* timestamp;



void
api_time_server__init(void)
{
    Time_server_init();

    timestamp = get_instance_Timestamp();

    Timestamp_create_timestamp(__DATE__, __TIME__);

    Time_t tm;
    Timestamp_get_time(timestamp, 0, &tm);



    printf("Time Server Timestamp:\n");
    printf("%s - %s\n", __DATE__, __TIME__);
    printf("%02d.%02d.%04d - %02d:%02d:%02d\n", tm.day, tm.month, tm.year, tm.hour,
           tm.min, tm.sec);
}



uint64_t
api_time_server_get_timestamp(void)
{
    uint64_t t_stamp = 0;

    timestamp = get_instance_Timestamp();

    t_stamp = timestamp->timestamp + get_timestamp_relative();

    return t_stamp;
}



void
api_time_server_sleep(unsigned int msec)
{
    Time_server_sleep(msec);
}



int run(void)
{
    while (1)
    {
        Time_server_tick_sec();
    }

    return 0;
}
