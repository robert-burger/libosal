# Tests for the POSIX implementation of libosal

## How to run tests

````bash
make check CFLAGS="-Wall -Wextra"
````

for stricter error checking with GCC, use

````bash
make check CFLAGS="-Wall -Wextra -Werror -m64  -Wall -Wshadow \
     -Wpointer-arith -Wstrict-prototypes -Wmissing-prototypes" 
````

The results end up in the `*.log` files in the .../posix folder.

## Environment Variable Switches for running extra Tests

Because part of these tests run against non-deterministic
multi-threaded code, some test cases are not deterministic and are not
guaranteed to succeed even for a correct implementation. For example,
a loaded system might miss semaphore notifications, and some tests of
error cases will depend on system limits. Also, operations which will
have guaranteed (but usually system-dependend) maximum latency limits
on real-time systems can fail on desktop systems.

To allow for automated testing, such test cases are not run by
default, but can be activated by setting environment variables.

There are several variables which affect whether these tests are run:

1. Checking basic latency bounds.

Set CHECK_LATENCY=1 if you want to make basic checks for maximum
latencies. These are intended as sanity tests, not for performance
tuning. However adapted variants of the tests might
be useful for performace tuning.

Under Linux, maximum latencies are adjusted depending on whether the
system determines whether it runs under the FIFO real-time scheduler,
which on RMC desktops can be set using the `chrt` command.

In this case, the latency-critical tests will use an mlockall() system
call to lock memory and disable paging. 

2. Envvar Switches for non-deterministic or unstable tests

These tests are not included by default in automated runs,
since they do not necessarily run reliable on a standard system.

* **TEST_FILESIZE=1** Tests message queues for detecting a file size resource limit error

* **CHECK_TRYWAIT=1** run SemaphoreFunction::TryCount test, which checks for missed events

* **CHECK_SUSPEND=1** run suspend / resume test (might require manual SIGCONT 
  signal or `"fg"` command in bash, if the signal is received by 
  the test runner and stops it)

3. Verbose progress reports

Set VERBOSE=1 to get verbose progress reports. These might be
useful for error analysis if some tests time out or hang due 
to errors. Also, some latency results are printed.


## Coverage Analysis

Coverage analysis data can be generated during the test.

This analysis requires extra cflags when compiling.
From the project root folder, use:

````bash
make clean
make CFLAGS="--coverage -O0"
````

To generate colorful HTML output using gcovr, run

````bash
cd tests/posix
./run-gcovr.sh
````

(or include the equivalent line in the conan 
build script).

Then, open libosal/tests/posix/coverage/details.html
in your browser.

Coverage analysis reports are not included in the default
build in order to keep build dependencies smaller.

## Documentation of individual Tests

* [Overview on Test Documentation](doc/Overview.rst)

Synchronization Mechanisms
--------------------------

* [Mutexes](doc/Mutex.rst)
* [Condition Variables](doc/Condition_Variables.rst)
* [Binary Semaphores](doc/Binary_Semaphore.rst)
* [Counting Semaphores](doc/Counting_Semaphore.rst)
* [Spin Locks](doc/Spinlock.rst)

  
Task Management / Threads
-------------------------

* [Task creation and configuration](doc/Tasks.rst)


Communication Mechanisms / Inter-Process Communication
------------------------------------------------------

* [Message Queues](doc/MessageQueue.rst)
* [Shared Memory Segments](doc/SharedMemory.rst)


Timers
------

* [Timers](doc/Timer.rst)


Debugging Facilities
--------------------

* [Console IO](doc/IO.rst)
* [Tracing](doc/Trace.rst)
* [Shared Memory textual I/O](doc/SHM_IO.rst)


