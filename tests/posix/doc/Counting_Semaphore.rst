==================
Counting Semaphore
==================

This lists tests for counting or "conventional" semaphores.

.. contents::
   :depth: 4

* `Explanation on Test Groups <./Overview.rst>`_

* Tests for binary semaphores are `on an extra page <./Binary_Semaphore.rst>`_

  

Functional Tests
================

The basic required properties for semaphores
are that they are an event notification mechanism.

For a counting semaphore, if there is a notification issued
with `osal_semaphore_post()`, it should
always be received. The receipt of that notification should always be
after sending it (and of course as soon as possible, too).


SemaphoreFunction, DirectWait
-----------------------------

Sets up a sender and a receiver thread, and then
waits for a semaphore notification in the receiver
thread, which is always waiting using `osal_semaphore_read)_`.
The receiving thread confirm receival using a POSIX
condition variable, and returns immediately to wait for
further events.

It is verified that for each notification, an event is
received (using `osal_semaphore_wait()`, and also that
receival of the notification is always after
the time of the semaphore_post action.

SemaphoreFunction, RandomizedDelay
----------------------------------

This test is similar to the preceding `DirectWait` test,
with the difference that before waiting for the next
event, the receiving thread waits for a random
time. This verifies that events are also received
when the `osal_semaphore_wait()` happens
after sending the notification. The check for temporal
consistency is adjusted to discern tha two
cases that waiting started before sending
data, and after sending data.


SemaphoreFunction, ParallelCount
--------------------------------

One sending thread, but multiple receivers
which can receive the events and count them.
The count is compared.

SemaphoreFunction, TimedCount
-----------------------------

Sends events from a single thread
to multiple receivers, with a delay before
sending. In this test case, the `osal_semaphore_timedwait()`
function is used. Only events are counted.

SemaphoreFunction, TryCount
---------------------------

Here, the function `osal_semaphore_trywait()` is tested,
and event counts are compared.




Ẽrror Detection Tests
=====================
