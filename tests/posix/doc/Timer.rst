================
Timer Operations
================


.. contents::
   :depth: 4

* `Explanation on Test Groups <./Overview.rst>`_
  

Functional Tests
================

Function of timers is compared by running
a nanosleep command between timer events,
and measuring the time stamps before and after.

If the timers work properly, the measured time
stamp differences need to be at least as large
as the configured timer interval. When the
system runs with real-time scheduling,
we can also put some loose restrictions on
the maximum interval (though that can't be
measured and tested reliably in an automated way
without a dedicated system setup, so these tests are
optional).


TimerExpiredFunction, SingleThreaded
------------------------------------

Checks the `osal_timer_expired() function`, as described
above against a range of wait time.


TimerExpiredFunction, MultiThreaded
-----------------------------------

Checks the `osal_timer_expired()` function in parallel
threads, with a randomized set of wait times.

TimerSleepFunction, SingleThreaded
----------------------------------

Checks `osal_sleep()` in a single thread,
as described above.

TimerSleepFunction, MultiThreaded
---------------------------------

Checks `osal_sleep()` in multiple threads.


TimerSleepUntilFunction, SingleThreaded
---------------------------------------

Checks `osal_sleep_until()` in a single thread.



TimerSleepUntilFunction, MultiThreaded
--------------------------------------

Checks `osal_sleep_until()` in multiple threads.

TimerFunction, BusyWait
-----------------------

Tests the `osal_busy_wait_until_nsec()` function.



