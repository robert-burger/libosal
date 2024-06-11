==============================
Overview on Test Documentation
==============================

Complexity of Tests
===================

In comparison to typical unittests, a
part of the tests are relatively complex,
since they test complex mechanisms,
and they deal with errors which might
be triggered in non-deterministic ways.

However, they do not represent stress tests.
Temporarily changing parameters can
change some test cases into stress tests.

Some tests check for basic latency and
timing conditions. However, they are not
meant to provide extensive latency checks
for a tuned system (some test cases might
be modified to cover this).

Table of Tests
==============



Synchronization Mechanisms
--------------------------

* `Mutexes <Mutex.rst>`_
* `Condition Variables <Condition_Variables.rst>`_
* `Binary Semaphores <Binary_Semaphore.rst?>`_
* `Counting Semaphores <Counting_Semaphore.rst>`_
* `Spin Locks <Spinlock.rst>`_

  
Task Management / Threads
-------------------------

* `Task creation and configuration <Tasks.rst>`_


Communication Mechanisms / Inter-Process Communication
------------------------------------------------------

* `Message Queues <MessageQueue.rst>`_
* `Shared Memory Segments <SharedMemory.rst>`_


Timers
------

* `Timers <Timer.rst>`_


Debugging Facilities
--------------------

* `Console IO <IO.rst>`_
* `Tracing <Trace.rst>`_
* `Shared Memory textual I/O <SHM_IO.rst>`_


Grouping / Classification of  Tests
===================================

The tests are grouped per subsystem / facility.

Tests for each subsystem are tagged
by their group and test name.
The group name consists of the subsystem
name plus a type suffix.

Suffixes are:

:Function: Test of the core functionality of
	   a substem. For example, a mutex function
	   test might verify whether the mutex
	   provides mutual exclusion, by trying
	   to provoke race conditions which#
	   the mutex should avoid.

:Config:   Test of a subsystem configuration,
	   which activates, for example, behavior variants.

:Param:    Tests which check for processing of
	   a specific parameter, which invokes
	   an alternate code path.

:Reject:   Tests which trigger a rejection of parameters, for example
	   due to system limits. An example would be a message queue
	   where the number of messages to be configured is higher
	   than the system limit.  These tests will invoke assertions
	   that operations have failed.

:Detect:   Tests which detect an error case. An example would be an
	   operation which requires a file descriptor for a message
	   queue which receives an invalid file descriptor as input.

	   Note that some error case cannot be detected
	   since they  provoke undefined behavior.
	   This is a weakness of the language since
	   C and C++ assume in many places that no
	   invalid operations are performed.
	   

