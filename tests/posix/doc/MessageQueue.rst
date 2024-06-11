===================
Message Queue Tests
===================

.. contents::
   :depth: 4

* `Explanation on Test Groups <./Overview.rst>`_

Functional Tests
================

MessageQueueFunction, MultiSendMultiReceive
-------------------------------------------

Tests that a message queue with multiple senders
and receivers transmits messages correctlt (matching
the sended data and its sqeuence).

MessageQueueFunction, ReadonlyWriteonly
---------------------------------------

Functional test with multiple senders / multiple receivers where the
participants are read-only and write-only.


MessageQueueFunction, TimeoutsDelayedSend
-----------------------------------------

Checks for timing of sending and receiving
emssages, especially that message are only
received after they have been sent, in the case
that the sending was done a while after starting
to receive.

MessageQueueFunction, TimeoutsDelayedRecv
-----------------------------------------

Also checks timing of sent/receive as above, but
for the case that also receiving might have
started later than sending the message.

Timings of the send/receive events are checked
for consistency.



Messaging with active Signal Handlers
=====================================

These tests are done with active signal
handlers which cause interrupt return codes
from the implementation messaging functions.

Usually, the messaging primitives *do not* return
when signals are received, since they are
not interruptible by default (contrary to
operations such as block I/O). However,
when interrupt handlers are set using sigaction
(on Linux), the low-level functions can become
interrupted and in that case, the corresponding
code paths are used.

MessageQueueFunction, TestInterruptSend
---------------------------------------

Checks for detection of a signal during osal_mq_send.

MessageQueueFunction, TestInterruptTimedSend
--------------------------------------------

Checks for detection of signal during interrupted
timed send (osal_mq_timedsend)

MessageQueueFunction, TestInterruptReceive
------------------------------------------

Checks for detection of signal during osal_mq_receive.

MessageQueueFunction, TestInterruptTimedReceive
-----------------------------------------------

Checks for detection of signal during osal_mq_timedreceive.




Error Detection
===============

MessageQueueDetect, InvalidParamsAccess
---------------------------------------

Several tests to detect message queue initialization
with invalid access configuration parameters.

MessageQueueError, InvalidMessageSize
-------------------------------------

Detects message queue setup with a message
size that is too large.


MessageQueueError, NonExistingName
----------------------------------

Detects attempt to initialize
a message queue with a non-existing name.


MessageQueueError, OverlyLongName
---------------------------------

Detects attempt to configure a message queue with
a 10000 characters long name.


MessageQueueError, ExceedingSizeLimit
-------------------------------------

Detects attempt to configure a message queue wich exceeds
the message size limit.

MessageQueueError, TestMessageNumber
------------------------------------

Detects attempt to configure a message queue which
exceeds the maximum message number.

MessageQueueError, TestFileLimit
--------------------------------

Detects attempt to configure a message queue which exceeds a
process-specific system resource limit on file descriptors (set with
rlimit).

MessageQueueError, TestFileSize
-------------------------------

Detects attemp to configure a mmap'ed message queue with
a file size that exceeds the maximum file size.

MessageQueueError, TestDataSize
-------------------------------

Detects attempt to configure a message
queue which exceeds the maximum data size.


MessageQueueError, TestInvalidDescriptor
----------------------------------------

Detects attempto to close a message queue with
an invalid descriptor.

MessageQueueError, TestMaxNumQueues
-----------------------------------

Detects to configure a number of messahe queues that
is too high for the given system configuration.

MessageQueueError, TestSendErrors
---------------------------------

Detects to use a message queue where the message
that is being tried to send is larger than
the configured maximum message size.


MessageQueueDetect, TestReceiveErrors
-------------------------------------

Detects time-outs during message receive, an invalid deadline,  a
buffer size that is too small or too large,
or an invalid file descriptor.










Tests with Signals
==================






