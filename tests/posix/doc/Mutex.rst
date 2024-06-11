===========
Mutex Tests
===========

.. contents::
   :depth: 4

* `Explanation on Test Groups <./Overview.rst>`_

	   

Simple Mutexes
**************

Functional Tests
================


MutexFunction, SingleThreadedNoRelease
--------------------------------------

Tests construction, locking using osal_mutex_lock
and unlcking once in a single thread.

MutexFunction, SingleThreadedWithRelease
----------------------------------------

Tests multiple locks / unlocks in a single thread


MutexFunction, ParallelMultiThreading
-------------------------------------

Tests prevention of data race conditions in multilke threads.

MutexFunction, MultithreadingPlusRandomizedWait
-----------------------------------------------

Tests prevention of race conditions with random waits in
each thread.


MutexFunction, TryLock
----------------------

Tests osal_mutex_trylock of a mutex that is already locked,
verifying correct detection of the locked state.
   



Tests essential function of the priority-ceiling mutex variant.

MutexFunction, TestRecursive
----------------------------

Tests function of the recursive mutex, which allows for repeated
locking from the same thread.

Error Detection in Simple Mutexes
=================================

Note that for the standard pthreads implementaion, many error cases
cannot be tested since these errors invoke undefined behavior,
which is not testable.

MutexDetect, TestRelock
-----------------------

Tests detection of an already locked mutex with the error-detecting
mutex variant.

MutexDetect, OwnerDead1
-----------------------

Tests correct detectuib of a dead owning thread with
the error-detecting mutex, returning OSAL_ERR_OWNER_DEAD.

MutexDetect, OwnerDead2
-----------------------

Test detection of a dead owning thread for a mutex
with its trylock command, which also should return
OSAL_OWNER_DEAD.


Tests for Priority Inheritance
******************************

Functional Tests
================


MutexFunction, InheritPar
-------------------------

Tests essentioal function of the priority-inheritance mutex variant.


MutexFunction, ProtectPar
-------------------------

Tests essential function of priority ceiling variant.


MutexFunction, TestNoPriorityInheritance
----------------------------------------

Tests function of a mutex without priority inheritance, the expected
result is priority inversion (a medium-priority thread blocks a
low-priority thread that holds a lock which a high-priority thread is
waiting for).

MutexFunction, TestPriorityInheritance
--------------------------------------

Tests correct function of a mutex with priority inheritance
protocol, which prevents priority inversion.

MutexFunc, TestPriorityCeiling
------------------------------

Tests correct function of a mutex that
prevents priority inversion using a
priority ceiling implementation.

Error Detection
===============


MutexDetect, TestPriorityError
------------------------------

Tests whether a mutex with priority ceiling detects the error case
that the requested or needed priority is higher than the configured
ceiling priority.
