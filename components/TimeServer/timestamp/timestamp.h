/*
   *
   *  Copyright (C) 2020, Hensoldt Cyber GmbH
*/

#pragma once


#include <stdint.h>
#include <stdbool.h>


typedef struct Timestamp_t Timestamp_t;
typedef struct Time_t Time_t;


typedef bool
(*Timestamp_create_timestampT)(const char* date, const char* time);


typedef bool
(*Timestamp_get_timeT)(Timestamp_t* t_stamp, uint8_t hours, Time_t* tm);


typedef bool
(*Timestamp_get_timestampT)(Time_t* tm, Timestamp_t* t_stamp);


struct Time_t
{
    uint8_t  sec;
    uint8_t  min;
    uint8_t  hour;
    uint8_t  day;
    uint8_t  month;
    uint16_t year;
};


typedef struct
{
    Timestamp_create_timestampT create_timestamp;
    Timestamp_get_timeT         get_time;
    Timestamp_get_timestampT    get_timestamp;
} Timestamp_Vtable;


struct Timestamp_t
{
    uint64_t               timestamp;
    const Timestamp_Vtable* vtable;
};


Timestamp_t*
get_instance_Timestamp(void);


void
Timestamp_dtor(void);


bool
Timestamp_create_timestamp(const char* date, const char* time);


bool
Timestamp_get_time(Timestamp_t* t_stamp, uint8_t hours, Time_t* tm);


bool
Timestamp_get_timestamp(Time_t* tm, Timestamp_t* t_stamp);
