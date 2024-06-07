==================
Counting Semaphore
==================

This lists tests for counting or "conventional" semaphores.

.. contents::
   :depth: 4

* `Explanation on Test Groups <./Overview.rst>`_

* Tests for binary semaphores are `on an extra page <./Binary_Semaphore.rst>`_

  

Functional Tests
================

The basic required properties for semaphores
are that they are an event notification mechanism.

For a counting semaphore, if there is a notification issued, it should
always be received. The receipt of that notification should always be
after sending it (and of course as soon as possible, too).



