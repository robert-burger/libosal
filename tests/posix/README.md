# Tests for the POSIX implementation of libosal

## Environment Variables

There are two variables which affect how tests are run:

1. Checking basic latency bounds.

Set CHECK_LATENCY=1 if you want to make basic checks for maximum
latencies. These are intended as sanity tests, not for performance
tuning. However adapted variants of the tests might
be useful for performace tuning.

Under Linux, maximum latencies are adjusted depending on whether
the system determines whether it runs under the FIFO
real-time scheduler, which can be set using the chrt command.

In this case, the latency-critical tests will use an mlockall() system
call to lock memory and disable paging.


These tests are not intended to be used on CI or on a build
server, since they can fail randomly depending on system load.

2. Verbose progress reports

Set VERBOSE=1 to get verbose progress reports. These might be
useful if some tests time out or hang due to errors.


## Coverage Analysis

Coverage analysis is generated during the test.

This analysis requires extra cflags when compiling,
use, from the project root folder:

````bash
make clean
make CFLAGS="--coverage -O0"
````

To generate colorful HTML output using gcovr, run

````bash
cd tests/posix
./run-gcovr.sh
````

(or include the equivalent line in the conan build script).

Then, open libosal/tests/posix/coverage/details.html
in your browser.

## Documentation of individual Tests

* [Overview on Test Documentation](doc/Overview.rst)

* [Mutex](doc/Mutex.rst)
