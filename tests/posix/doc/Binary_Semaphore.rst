==================
Binary Semaphores
==================

This lists tests for binary semaphores. These differ from
counting semaphores that they only register the presence
of an event until it is received, which means that
with overlapping semaphore_post() calls, sent event's
can be lost in the sense that there can be less
received events than sent events.


.. contents::
   :depth: 4

* `Explanation on Test Groups <./Overview.rst>`_

* Tests for counting semaphores are `on an extra page <./Counting_Semaphore.rst>`_

  

Functional Tests
================

BinarySemaphoreFunction, DirectWait
-----------------------------------

This tests the binary in two threads,
a sender and a receiver. The sender stores
a random number and posts to the semaphore,
then waits for conformation. The receiver
waits for the semaphore, and reads the random
value. Sent and received values are compared,
as well as times for sending and receival.

BinarySemaphoreFunction, RandomizedDelay
----------------------------------------


Like `BinarySemaphoreFunction, DirectWait`,
but introduce a random delay before doing
the wait step. Because each new event if
only sent after the receival of the previous
event was confirmed, no events are lost.

BinarySemaphoreFunction, ParallelCount
--------------------------------------

This test sends a number of notification
events from one sending thread to multiple
receiving threads. In this variant, events
can become superseded by new notifications
(`osal_binary_semaphore_post()` calls), so that
the test criterion is relaxed.


