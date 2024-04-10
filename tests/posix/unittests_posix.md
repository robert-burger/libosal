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


## Semaphore osal_semaphore_wait()/osal_semephore_post()

### binary semaphore

- test with atomic counters und time stamps

- variant 1: two processes, one emits the post() n times and registers the time,
  the second waits n times and registers the time
		
- result compare that all time stamps are
  registered, and in RT mode, the difference is below a threshold

- in variant 2, before waiting, the waiter waits some random
  amount of time, registers also before it starts to wait again,
  and the comparison requires that the send time is
  either <= the start-wait time, or closely 
  before the receive time.

  
### counting semaphore

similar as above, but 

- more than one waiter and sender, respectively

- variant 1: sending and receiving times are registered

- Variant 2: the sender thread holds a mutex to introduce a random delay,
  which the receiver waits for, and the time registered by the senders 
  is the time after releasing the mutex
  
- sending and waiting times are merge-sorted before
  the comparison, and the difference has to be
  below a threhold



## Condition variables

- analog to counting semaphores, with the
  counter being the protexted variable
  
