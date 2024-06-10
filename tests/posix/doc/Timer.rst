================
Timer Operations
================

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
