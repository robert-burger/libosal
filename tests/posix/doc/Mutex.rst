===========
Mutex Tests
===========

.. contents::
   :depth: 4

Functional Tests
================

:MutexFunction, SingleThreadedNoRelease:
   Tests construction, locking using osal_mutex_lock
   and unlcking once in a single thread.

:MutexFunction, SingleThreadedWithRelease:
   Tests multiple locks / unlocks in a single thread


:MutexFunction, ParallelMultiThreading:
   Tests prevention of data race conditions in multilke threads.

:MutexFunction, MultithreadingPlusRandomizedWait:
   Tests prevention of race conditions with random waits in
   each thread.

:MutexDetect, TryLock:
   Tests osal_mutex_trylock of a mutex that is already locked,
   verifying correct detection of the error.
   
					 
