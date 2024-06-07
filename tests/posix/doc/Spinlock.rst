==============
Spinlock Tests
==============

.. contents::
   :depth: 4

* `Explanation on Test Groups <./Overview.rst>`_

The spinlock tests are very similar to the mutex tests. Spin locks
usually have less error detection capability, with most usage errors
leading to undefined behavior, so most error cases are not tested.

  
Functional Tests
================

SpinlockFunction, SingleThreadedNoRelease
-----------------------------------------

Just initializes, locks and unlocks a spinlock
to check basic function.

SpinlockFunction, SingleThreadedWithRelease
-------------------------------------------

Just initializes a spinlocks and locks and
releases it multiple times in a single thread.

SpinlockFunction, ParallelMultithreading
----------------------------------------

Tests a spinlock in multiple threads, in
a way that would provoke race conditions
if the spinlock would not provide mutual
exclusion.

SpinlockMultithreading, RandomizedPlusWait
------------------------------------------

Tests a spinlock in multiple threads, with
a random wait time between actions.


