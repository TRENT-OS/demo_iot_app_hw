/*
   *
   *  Copyright (C) 2020, Hensoldt Cyber GmbH
*/

#pragma once


#include <stdint.h>


void
Time_server_init(void);


void
Time_server_tick_sec(void);


void
Time_server_sleep(unsigned int msec);


uint64_t
get_timestamp_relative(void);
