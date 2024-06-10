=====
Tasks
=====

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

The functional tests are similar to the mutex tests,
with the important difference that pthreads thread
operations and libosal mutex opearions are swapped
out against libosal task operations and pthreads
mutex operations. This avoids to test two interfaces
that are not known to work at the same time.

In addition, the priority inversion tests for
the Mutexes also verify that some tasks
are actually running in parallel.

TasksMultithreadingFunction, Parallel
-------------------------------------

This test case is similar to the `MutexFunction,
ParallelMultiThreading` tests for the osal mutex:
Several tasks are started which increment shared counters.
When they have ended, the counters need to have
a value consistent with running im multiple tasks.

TasksMultithreadingFunction, RandomizedPlusWait
-----------------------------------------------

Increments a counter in several tasks, with
additional random waiting between tasks.


TasksMultithreadingFunction, TaskCancel
---------------------------------------

Tests canceling a task at a cancellation point.

TasksMultithreadingFunction, TaskSuspend
----------------------------------------

Tests suspending and resuming a task.

This replaces an earlier version tagges
`TasksMultithreadingFunction, SuspendResume`.

Note that, depending on specifics, a suspended
task must be resumed from the terminal executing
the test since the main thread might become suspended.



Configuration Tests
===================


TasksMultithreadingConfig, TaskAttributes
-----------------------------------------

This test checks getting and setting the following
task attributes:

* CPU affinity
* scheduling policy
* scheduling priority
* other task attributes
