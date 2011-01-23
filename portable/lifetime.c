/*
 * Lifetime conversion routine for Kerberos libraries that don't have it.
 *
 * This file contains an implementation of krb_life_to_time for Kerberos
 * libraries that don't have it.  This implements the Kerberos v4 long
 * lifetime semantics unless SHORT_LIFETIME is defined.  If SHORT_LIFETIME is
 * defined, the normal Kerberos v4 lifetime semantics are used.
 *
 * Copyright 2003, 2004, 2007, 2008
 *     The Board of Trustees of the Leland Stanford Junior University
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <config.h>
#include <portable/krb4.h>

#include <time.h>

/* The lifetime value indicating a ticket that never expires. */
#define TKTLIFENOEXPIRE 0xFF

/* The seconds value for a ticket that never expires. */
#ifndef NEVERDATE
# define NEVERDATE ((time_t) -1L)
#endif

#ifdef SHORT_LIFETIME

/*
 * Convert a Kerberos v4 lifetime to the expiration time in seconds.  The
 * conversion without long lifetime support is trivial: just multiply the
 * lifetime by five to get minutes.  Takes the starting point for the
 * calculation and the lifetime and returns the expiration time in seconds
 * since epoch.
 */
time_t
krb_life_to_time(time_t start, int life)
{
    /*
     * Lifetime semantics require wrapping at 255, which can be implemented by
     * forcing the lifetime into an unsigned char.
     */
    life = (unsigned char) life;
    if (life == TKTLIFENOEXPIRE)
        return NEVERDATE;
    else
        return start + life * 5 * 60;
}


/*
 * Convert a start and end time for a ticket to a Kerberos ticket lifetime
 * char.  The conversion without long lifetime support is trivial: just round
 * up to the next highest five minute interval after doing range checking.
 */
int
krb_time_to_life(KRB4_32 start, KRB4_32 end)
{
    KRB4_32 lifetime;
    int i;

    if (end == NEVERDATE)
        return TKTLIFENOEXPIRE;
    lifetime = end - start;
    if (lifetime > 254 * 5 * 60 || lifetime <= 0)
        return 0;
    return (lifetime + 5 * 60 - 1) / (5 * 60);
}

#else /* !SHORT LIFETIME */

#define TKTLIFENUMFIXED 64      /* Size of the lifetime table. */
#define TKTLIFEMINFIXED 0x80    /* The first special lifetime. */
#define TKTLIFEMAXFIXED 0xBF    /* The last special lifetime. */

/** The maximum ticket lifetime. */
#define MAXTKTLIFETIME (30*24*3600)

/* Lookup table for long ticket lifetimes.
 *
 * This defines the table used to lookup lifetime for the fixed part of the
 * one byte lifetime field.  Values less than 0x80 are intrpreted as the
 * number of 5 minute intervals.  Values from 0x80 to 0xBF should be looked up
 * in this table.  The value of 0x80 is the same using both methods:  10 and
 * two-thirds hours.  The lifetime of 0xBF is 30 days.  The intervening values
 * of have a fixed ratio of roughly 1.06914.
 *
 * The value 0xFF is defined to mean a ticket has no expiration time.  This
 * should be used advisedly since individual servers may impose defacto upper
 * bounds on ticket lifetimes.
 */
static int tkt_lifetimes[TKTLIFENUMFIXED] = {
    38400,                      /* 10.67 hours, 0.44 days */
    41055,                      /* 11.40 hours, 0.48 days */
    43894,                      /* 12.19 hours, 0.51 days */
    46929,                      /* 13.04 hours, 0.54 days */
    50174,                      /* 13.94 hours, 0.58 days */
    53643,                      /* 14.90 hours, 0.62 days */
    57352,                      /* 15.93 hours, 0.66 days */
    61318,                      /* 17.03 hours, 0.71 days */
    65558,                      /* 18.21 hours, 0.76 days */
    70091,                      /* 19.47 hours, 0.81 days */
    74937,                      /* 20.82 hours, 0.87 days */
    80119,                      /* 22.26 hours, 0.93 days */
    85658,                      /* 23.79 hours, 0.99 days */
    91581,                      /* 25.44 hours, 1.06 days */
    97914,                      /* 27.20 hours, 1.13 days */
    104684,                     /* 29.08 hours, 1.21 days */
    111922,                     /* 31.09 hours, 1.30 days */
    119661,                     /* 33.24 hours, 1.38 days */
    127935,                     /* 35.54 hours, 1.48 days */
    136781,                     /* 37.99 hours, 1.58 days */
    146239,                     /* 40.62 hours, 1.69 days */
    156350,                     /* 43.43 hours, 1.81 days */
    167161,                     /* 46.43 hours, 1.93 days */
    178720,                     /* 49.64 hours, 2.07 days */
    191077,                     /* 53.08 hours, 2.21 days */
    204289,                     /* 56.75 hours, 2.36 days */
    218415,                     /* 60.67 hours, 2.53 days */
    233517,                     /* 64.87 hours, 2.70 days */
    249664,                     /* 69.35 hours, 2.89 days */
    266926,                     /* 74.15 hours, 3.09 days */
    285383,                     /* 79.27 hours, 3.30 days */
    305116,                     /* 84.75 hours, 3.53 days */
    326213,                     /* 90.61 hours, 3.78 days */
    348769,                     /* 96.88 hours, 4.04 days */
    372885,                     /* 103.58 hours, 4.32 days */
    398668,                     /* 110.74 hours, 4.61 days */
    426234,                     /* 118.40 hours, 4.93 days */
    455705,                     /* 126.58 hours, 5.27 days */
    487215,                     /* 135.34 hours, 5.64 days */
    520904,                     /* 144.70 hours, 6.03 days */
    556921,                     /* 154.70 hours, 6.45 days */
    595430,                     /* 165.40 hours, 6.89 days */
    636601,                     /* 176.83 hours, 7.37 days */
    680618,                     /* 189.06 hours, 7.88 days */
    727680,                     /* 202.13 hours, 8.42 days */
    777995,                     /* 216.11 hours, 9.00 days */
    831789,                     /* 231.05 hours, 9.63 days */
    889303,                     /* 247.03 hours, 10.29 days */
    950794,                     /* 264.11 hours, 11.00 days */
    1016537,                    /* 282.37 hours, 11.77 days */
    1086825,                    /* 301.90 hours, 12.58 days */
    1161973,                    /* 322.77 hours, 13.45 days */
    1242318,                    /* 345.09 hours, 14.38 days */
    1328218,                    /* 368.95 hours, 15.37 days */
    1420057,                    /* 394.46 hours, 16.44 days */
    1518247,                    /* 421.74 hours, 17.57 days */
    1623226,                    /* 450.90 hours, 18.79 days */
    1735464,                    /* 482.07 hours, 20.09 days */
    1855462,                    /* 515.41 hours, 21.48 days */
    1983758,                    /* 551.04 hours, 22.96 days */
    2120925,                    /* 589.15 hours, 24.55 days */
    2267576,                    /* 629.88 hours, 26.25 days */
    2424367,                    /* 673.44 hours, 28.06 days */
    2592000                     /* 720.00 hours, 30.00 days */
};


/*
 * Convert a Kerberos v4 lifetime to the expiration time in seconds.  Takes a
 * start time and a Kerberos standard lifetime char and returns the
 * corresponding end time.
 *
 * There are four simple cases to be handled.  The first is a life of 0xff,
 * meaning no expiration, and results in an end time of 0xffffffff.  The
 * second is when life is less than the values covered by the table; in this
 * case, the end time is the start time plus the number of 5 minute intervals
 * specified by life.  The third case returns start plus the MAXTKTLIFETIME if
 * life is greater than TKTLIFEMAXFIXED.  The last case uses the life value
 * (minus TKTLIFEMINFIXED) as an index into the table to extract the lifetime
 * in seconds, which is added to start to produce the end time.
 *
 * Takes the starting point for the calculation and the lifetime and returns
 * the expiration time in seconds since epoch.
 */
time_t
krb_life_to_time(time_t start, int life)
{
    /*
     * Lifetime semantics require wrapping at 255, which can be implemented by
     * forcing the lifetime into an unsigned char.
     */
    life = (unsigned char) life;
    if (life == TKTLIFENOEXPIRE)
        return NEVERDATE;
    if (life < TKTLIFEMINFIXED)
        return start + life * 5 * 60;
    if (life > TKTLIFEMAXFIXED)
        return start + MAXTKTLIFETIME;
    return start + tkt_lifetimes[life - TKTLIFEMINFIXED];
}


/*
 * Given the start and end times for a ticket, return a Kerberos standard
 * lifetime char, possibly using the tkt_lifetimes table for lifetimes above
 * 127 * 5 minutes.
 *
 * First, the special case of end == NEVERDATE is mapped to no expiration.
 * Then, negative lifetimes and those greater than the maximum ticket lifetime
 * are rejected.  then lifetimes less than the first tame entry are handled by
 * rounding the requested lifetime up to the next five minute interval.
 * Finally, for lifetimes that map into the table, search for the smallest
 * entry greater than or equal to the requested lifetime.
 */
int
krb_time_to_life(KRB4_32 start, KRB4_32 end)
{
    KRB4_32 lifetime;
    int i;

    if (end == NEVERDATE)
        return TKTLIFENOEXPIRE;
    lifetime = end - start;
    if (lifetime > MAXTKTLIFETIME || lifetime <= 0)
        return 0;
    if (lifetime < tkt_lifetimes[0])
        return (lifetime + 5 * 60 - 1) / (5 * 60);
    for (i = 0; i < TKTLIFENUMFIXED; i++)
        if (lifetime <= tkt_lifetimes[i])
            return i + TKTLIFEMINFIXED;
    return 0;
}

#endif /* !SHORT_LIFETIME */
