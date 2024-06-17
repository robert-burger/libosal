# Unit tests for libosal

## Timer

* uses Linux scheduler API query to
  determine whether it is running in real-time mode
* sets test margins (tolerance)
* possibly gets parameters from environment ?


### osal_timer_init() / osal_timer_expired()

#### single-threaded

- run for mus / 1 ms / 10 ms  / 100 ms times

- check that timer never expires **before** period
- if running in real-time, check that timer never expires
  **after** period plus some margin

#### multi-threaded

- do the above for 10 parallel threads


### osal_timer_gettime_nsec() / osal_sleep_until()

- use random interval (log scale), 100 values between 100 ns and 1 s

- get clock, sleep for that interval, compare clock

- check that sleep interval is never smaller than requested

- in real-time-mode, check that sleep interval is never larger
  than requested
  
  
## Mutexes

### single-threaded (sanity test)


- acquire mutex, increment counter, release mutex
- same with releasing the mutex in the meantime

### multi-threaded

- 10 threads
- all in parallel acquire / increment / release counter (same number
  of increments as decrements), compare result after join

- the same, but with a short random waiting period


## Semaphore osal_semaphore_wait()/osal_semaphore_post()

### semaphore used as binary semaphore for 1:1 signaling

- test with atomic counters und time stamps

- variant 1: two processes, one generates and save a random value,
  registers the time, emits the post(), the second waits,
  registrs the time, and stores the "sent" value
		
- result compare that all time stamps are
  registered
  
- in th first case, the comparison requires that the send time is
  always before the receive time.

- and in RT mode, the difference must be below a threshold

- in variant 2, before waiting, the waiter waits some random
  amount of time, registers start-wait time before it starts to wait 
  for the semaphore,
  
  
- in the second case, the comparison requires that the send time is
  either <= the start-wait time, or closely 
  before the receive time.

  
###  semaphore  tested as counting somaphore for many processes

similar as above, but 

- more than one waiter, respectively

- variant 1: just count how many events were received ✓

- variant 2: sending and receiving times are registered
  
- For variant 2, sending and waiting times can be
  merge-sorted before the comparison, and the difference 
  has to be below a threshold

  ==> This was put back because comparing times needs
      some link between receiver and sender, like an
      extra event number, and because it has to be
      shared, this requires additional linking.

### semaphoore_timedwait()

- variant 1 from above with a random long delay

### semaphore_trywait()

- variant 1 from above with a random long delay



  ## Condition variables

- analog to counting semaphores, with the
  counter being the protexted variable
  
