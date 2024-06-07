===================
Shared Memory Tests
===================

.. contents::
   :depth: 4

* `Explanation on Test Groups <./Overview.rst>`_

The spinlock tests are very similar to the mutex tests. Spin locks
usually have less error detection capability, with most usage errors
leading to undefined behavior, so most error cases are not tested.

  
Functional Tests
================

SharedmemoryFunction, TestMMAPWorks
-----------------------------------

Tests the osal_shm_map call with write
access.


SharedmemoryFunction, RandomWrites
----------------------------------

Has a writer process, which writes random data
to a shared memory region, and several reader processes
which read that region after finishing. Both written
and read data are compared to verify it is equal.
The reader processes have read/write access in this test case.

SharedmemoryFunction, RandomWriteRestrict
-----------------------------------------

As above, a write process which writes random
data to a memory region, and several reader processes
which re-read and hash the data from that reagion;
Hashes are compared.

In this case, the reader processes have read-only access.


Error Detection Tests
=====================

SharedmemoryDetect, TestTruncateError
-------------------------------------

This test creates a small shm file and
makes it write protected, and then tries
to open and truncate the memory region
backed by the file. The error that is
reported correcyly  is OSAL_ERR_PERMISSION_DENIED,
because the file cannot be truncated.

SharedmemoryDetect, TestExistError
----------------------------------

This test creates an existing file in /dev/shm and then tries to
create and open a shared memory region exclusive to the process, which
should file since that file already exists.


SharedmemoryError, TestInvalidName
----------------------------------

Tries to create a shared memory region
with an invalid file name, which is the
empty string. Correctly, the error
OSAL_ERR_INVALID_PARAM is reported.

SharedmemoryError, TestTooLongName
----------------------------------

Similar to above, this test case tries
to open a shared memory region with
a name that is 50,000 characters long.
This should fail.

SharedmemoryError, TestTooManyFiles
-----------------------------------

This test case uses the getrlimit/setrlimit
sys calls to set the allowed number of new
open files to zero, and tries
to open a new shared memory region. This should
fail because it is file-based, returning the
error code OSAL_ERR_SYSTEM_LIMIT_REACHED.

SharedmemoryDetect, TestPermDenied
----------------------------------

Configures a shared memory handle for reading only,
but then tries to mmap it for writing,
which should of course file.

SharedmemoryDetect, TestMmapSizeLimit
-------------------------------------

Configures a shared memory handle. Then, sets
the process resource limit for locked memory
to zero, and then tries to mmap a larger
memory region. This should fail because the
process limit has to be checked when mmap'ing.


SharedmemoryError, TestBadDescriptor
------------------------------------

Tries to map a shared memory region
using a bad file descriptor, which should
be detected and fail.

Configuration Tests
===================

These tests check for success of certain configuration
options. Their main goal is to maximize test
coverage.

SharedmemoryConfig, TestProtExec
--------------------------------

Tries to open a shared memory region with the
OSAL_SHM_MAP_ATTR__PROT_EXEC flag, which should
succeed.

SharedmemoryConfig, TestProtNone
--------------------------------

Tries creating a region with the PROT_NONE
flag.

SharedmemoryConfig, TestProtPrivate
-----------------------------------

Tries the PROT_PRIVATE flag of shm_open()









