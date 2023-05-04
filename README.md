# libosal
[![Build Status](https://rmc-jenkins.robotic.dlr.de/jenkins/buildStatus/icon?job=common%2libosal%2Fmaster)](https://rmc-jenkins.robotic.dlr.de/jenkins/job/common/job/libosal/job/master/)

libosal is an <ins>**o**</ins>perating <ins>**s**</ins>ystem <ins>**a**</ins>bstraction <ins>**l**</ins>ayer Library. It's purpose is to write os-independent code for easy portability between different systems and architectures.

Features:

## Timers

The timer framework of libosal provides functions for sleeping and waiting for some defined amount of time. It is also used by the other components of libosal e.g. specifying a timeout while waiting on some events.

Here are some common example on how to use the timer frameworks:

**wait for timeout example**

This example can be used to do some work until a specified timeout occured or is finished otherwise.

```c
osal_timer_t timeout;
osal_timer_init(&timeout, 1000000000); // values are in nanoseconds

do {
  // your work goes here
} while (osal_timer_expired(&timeout) != OSAL_ERR_TIMEOUT);
```
**deterministic 1 ms loop example**

This example implements a loop which can be used as deterministic (with appropriate priority) clock source for a control appllication.

```c
osal_uint64_t cycle_rate = 1000000; // values are in nanoseconds
osal_uint64_t abs_timeout = osal_timer_gettime_nsec();
osal_retval_t ret;

do { 
  // your work goes here
  
  abs_timeout += cycle_rate;
  ret = osal_sleep_until_nsec(abs_timeout);
} while (ret == OSAL_OK);
```

## Mutexes

Mutual exclusion locks.

## Semaphores

## Binary Semaphores

Binary semaphores are a special case of a semaphore for signalling one event to one waiter.

## Conditions


## Spinlocks


## Tasks

Task/Thread abstraction.
