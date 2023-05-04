# libosal
[![Build Status](https://rmc-jenkins.robotic.dlr.de/jenkins/buildStatus/icon?job=common%2libosal%2Fmaster)](https://rmc-jenkins.robotic.dlr.de/jenkins/job/common/job/libosal/job/master/)

libosal is an <ins>**o**</ins>perating <ins>**s**</ins>ystem <ins>**a**</ins>bstraction <ins>**l**</ins>ayer Library. It's purpose is to write os-independent code for easy portability between different systems and architectures.

Features:

## Timers

The timer framework of libosal provides functions for sleeping and waiting for some defined amount of time. It is also used by the other components of libosal e.g. specifying a timeout while waiting on some events.


## Mutexes

Mutual exclusion locks.

## Semaphores

## Binary Semaphores

Binary semaphores are a special case of a semaphore for signalling one event to one waiter.

## Conditions


## Spinlocks


## Tasks

Task/Thread abstraction.
