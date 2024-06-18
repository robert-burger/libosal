================
SHM IO FunctionS
================



.. contents::
   :depth: 4

* `Explanation on Test Groups <./Overview.rst>`_

These tests are not in-depth since the corresponding functions are
used for development only.
  
Functional Tests
================

SHMIOFunction, SimpleMessage
----------------------------

Tests `osal_io_shm_setup()` ,
`osal_printf()`,  and `osal_io_shm_get_message()`
by printing a test message osing osal_printf(),
retrieving the result and comparing it to the
original message.


