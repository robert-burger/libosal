========================
Condition Variable Tests
========================

.. contents::
   :depth: 4

* `Explanation on Test Groups <./Overview.rst>`_

Functional Tests
================

CondvarFunction, ParallelMasked
-------------------------------

Signals multiple parallel threads with a single mutex/condvar pair and
using a broadcast notification, and counts the number of the received
signals.

CondvarFunction, ParallelSingleNotification
-------------------------------------------

Signals multiple threads with a non-broadcast
notification to one condvar for each thread.

What is checked is the number of received notifications, assuring that
no notification is lost.

CondvarFunction, ParallelWait
-----------------------------

Tests the osal_condvar_timedwait() interface, by sending
randomly delayed notifications to multiple threads,
via a single cond war. The number of wait intervals
without events is counted and compared.

