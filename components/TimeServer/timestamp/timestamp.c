/*
   *
   *  Copyright (C) 2020, Hensoldt Cyber GmbH
*/

#include "timestamp.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>



#define ASSERT_SELF(self)               \
    if(self == NULL)                    \
        nullptr = true



#define SEC_PER_MIN             60
#define SEC_PER_HOUR            (SEC_PER_MIN * SEC_PER_MIN)
#define SEC_PER_DAY             (SEC_PER_HOUR * 24)
#define SEC_PER_YEAR            (SEC_PER_DAY * 365)



#define IS_LEAP(year)           ((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))
#define DIV(a, b)               ((a) / (b) - ((a) % (b) < 0))
#define LEAPS_THRU_END_OF(y)    (DIV (y, 4) - DIV (y, 100) + DIV (y, 400))



#define START_YEAR              1970
#define DELIMITER               ",;.:-_ "



#define GET_MONTH(str)          (                      \
    str[0]=='J' && str[1]=='a' && str[2]=='n' ? 1  :   \
    str[0]=='F' && str[1]=='e' && str[2]=='b' ? 2  :   \
    str[0]=='M' && str[1]=='a' && str[2]=='r' ? 3  :   \
    str[0]=='A' && str[1]=='p' && str[2]=='r' ? 4  :   \
    str[0]=='M' && str[1]=='a' && str[2]=='y' ? 5  :   \
    str[0]=='J' && str[1]=='u' && str[2]=='n' ? 6  :   \
    str[0]=='J' && str[1]=='u' && str[2]=='l' ? 7  :   \
    str[0]=='A' && str[1]=='u' && str[2]=='g' ? 8  :   \
    str[0]=='S' && str[1]=='e' && str[2]=='p' ? 9  :   \
    str[0]=='O' && str[1]=='c' && str[2]=='t' ? 10 :   \
    str[0]=='N' && str[1]=='o' && str[2]=='v' ? 11 :   \
    str[0]=='D' && str[1]=='e' && str[2]=='c' ? 12 : 0 \
    )



// foreward devlaration
static bool _get_number(const char* string, uint8_t* val);
static bool _fill_date(const char* string, Time_t* tm, const char* delimiter);
static bool _fill_time(const char* string, Time_t* tm, const char* delimiter);



static const Timestamp_Vtable Timestamp_vtable =
{
    .create_timestamp = Timestamp_create_timestamp,
    .get_time         = Timestamp_get_time,
    .get_timestamp    = Timestamp_get_timestamp
};



static uint16_t month_table[2][13] =
{
    // Normal years
    { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
    // Leap years
    { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
};



// Singleton
static Timestamp_t _timestamp;
static Timestamp_t* this = NULL;



Timestamp_t*
get_instance_Timestamp(void)
{
    if (this == NULL)
    {
        memset(&_timestamp, 0, sizeof (Timestamp_t));
        this = &_timestamp;

        this->vtable = &Timestamp_vtable;
    }

    return this;
}



void
Timestamp_dtor(void)
{
    memset(this, 0, sizeof (Timestamp_t));
    this = NULL;
}



bool
Timestamp_get_time(Timestamp_t* t_stamp, uint8_t hours, Time_t* tm)
{
    if (t_stamp == NULL || hours > 24 || tm == NULL)
    {
        return false;
    }

    int64_t day = 0, tmp = 0;
    uint16_t* ip = NULL, year = 0;
    uint32_t hours_off = hours * SEC_PER_HOUR;
    uint8_t month = 0;
    int64_t yy = 0;

    day = t_stamp->timestamp / SEC_PER_DAY;

    tmp = t_stamp->timestamp % SEC_PER_DAY;
    tmp += hours_off;

    while (tmp < 0)
    {
        tmp += SEC_PER_DAY;
        --day;
    }

    while (tmp >= SEC_PER_DAY)
    {
        tmp -= SEC_PER_DAY;
        ++day;
    }

    tm->hour = (uint8_t)(tmp / SEC_PER_HOUR);

    tmp %= SEC_PER_HOUR;

    tm->min = (uint8_t)(tmp / 60);
    tm->sec = tmp % 60;

    // first year was 1970
    year = (uint16_t)(START_YEAR + day / 365 - (day % 365 > 0));

    yy = year;
    while (day < 0 || day >= (IS_LEAP(year) ? 366 : 365))
    {
        int64_t yg = yy + day / 365 - (day % 365 < 0);

        /* Adjust DAYS and Y to match the guessed year.  */
        day -= ((yg - yy) * 365
                + LEAPS_THRU_END_OF (yg - 1)
                - LEAPS_THRU_END_OF (yy - 1));
        yy = yg;
    }

    tm->year = (uint16_t)year;

    ip = month_table[IS_LEAP(year)];

    for (month = 11; day < (int64_t) ip[month]; --month)
    {
        continue;
    }

    if (IS_LEAP(year) == 0)
    {
        if (day < ip[2])
        {
            day++;
        }
    }

    day -= ip[month];

    tm->month = (uint8_t)month + 1;
    tm->day = (uint8_t)(day + 1);

    return true;
}



bool
Timestamp_get_timestamp(Time_t* tm, Timestamp_t* t_stamp)
{
    bool nullptr = false;

    ASSERT_SELF(this);

    if (nullptr)
    {
        return false;
    }

    if (tm == NULL || t_stamp == NULL)
    {
        return false;
    }

    uint64_t tmp_timestamp = 0;

    tmp_timestamp += tm->sec;
    tmp_timestamp += (tm->min * SEC_PER_MIN);
    tmp_timestamp += (tm->hour * SEC_PER_HOUR);
    tmp_timestamp += ((month_table[IS_LEAP(tm->year)][tm->month - 1] + tm->day - 1)
                      * SEC_PER_DAY);
    tmp_timestamp += ((tm->year - START_YEAR) * SEC_PER_YEAR);
    tmp_timestamp += ((LEAPS_THRU_END_OF(tm->year - 1) - LEAPS_THRU_END_OF(
                           START_YEAR - 1)) * SEC_PER_DAY);

    t_stamp->timestamp = tmp_timestamp;

    return true;
}



bool
Timestamp_create_timestamp(const char* date, const char* time)
{
    bool nullptr = false;

    ASSERT_SELF(this);

    if (nullptr)
    {
        return false;
    }

    if (date == NULL || time == NULL)
    {
        return false;
    }

    Time_t t;

    _fill_date(date, &t, DELIMITER);
    _fill_time(time, &t, DELIMITER);

    this->vtable->get_timestamp(&t, this);

    return true;
}



static bool
_get_number(const char* string, uint8_t* val)
{
    if (string == NULL || val == NULL)
    {
        return false;
    }

    uint8_t tmp = 0;

    if ( (tmp = GET_MONTH(string)) != 0)
    {
        *val = tmp;
        return true;
    }

    *val = (uint8_t)atoi(string);

    return true;
}



static bool
_fill_date(const char* string, Time_t* tm, const char* delimiter)
{
    if (string == NULL || tm == NULL)
    {
        return false;
    }

    char* ptr = NULL;
    char tmp[BUFSIZ];
    memset(tmp, 0, BUFSIZ);

    strcpy(tmp, string);

    ptr = strtok(tmp, delimiter);

    for ( int i = 0 ; ptr != NULL ; i++ )
    {
        if (i == 0)
        {
            _get_number(ptr, &tm->month);
        }
        if (i == 1)
        {
            _get_number(ptr, &tm->day);
        }
        if (i == 2)
        {
            tm->year = (uint16_t)atoi(ptr);
        }

        ptr = strtok(NULL, delimiter);

        if (i == 3)
        {
            break;
        }
    }

    if (tm->year < START_YEAR)
    {
        tm->year += 2000;
    }

    return true;
}



static bool
_fill_time(const char* string, Time_t* tm, const char* delimiter)
{
    if (string == NULL || tm == NULL)
    {
        return false;
    }

    char* ptr = NULL;
    char tmp[BUFSIZ];
    memset(tmp, 0, BUFSIZ);

    strcpy(tmp, string);

    ptr = strtok(tmp, delimiter);

    for ( int i = 0 ; ptr != NULL ; i++ )
    {
        if (i == 0)
        {
            _get_number(ptr, &tm->hour);
        }
        if (i == 1)
        {
            _get_number(ptr, &tm->min);
        }
        if (i == 2)
        {
            _get_number(ptr, &tm->sec);
        }

        ptr = strtok(NULL, delimiter);

        if (i == 3)
        {
            break;
        }
    }

    return true;
}
