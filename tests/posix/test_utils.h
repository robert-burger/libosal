#include <cassert>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <vector>

#ifndef LIBOSAL_POSIX_TEST_UTILS_H
#define LIBOSAL_POSIX_TEST_UTILS_H

/* helper function used to randomize wait times */

namespace testutils {

/* chain a hash value and a new value to a new hash.

   In our tests, this function has two purposes:

   1. It allows to compare the result of a transmission of
   a random byte sequence, by comparing the hashes
   of sent and received sequence. This allows for
   the recognition of bit errors, as well as of
   sequence errors.

   2. It provides a neat function that is relatively
   CPU-intensive, and can be used to keep a CPU
   busy when testing for priority inversion scenarios,
   where it matters which thread gets CPU time.
*/

inline size_t combine_hashes(size_t const oldhash, uint32_t const payload) {
  size_t new_hash = std::hash<uint32_t>{}(payload);
  return (oldhash << 4) ^ new_hash;
}

/* computes a hash value from a uint32 value */
inline size_t hash_u32(uint32_t const val) {
  size_t new_hash = std::hash<uint32_t>{}(val);
  return new_hash;
}

inline osal_timer_t set_deadline(int sec, long nsec)
/* takes a second and a nanosecond value,
   and generates a deadline value that
   many seconds / nanoseconds from now.
*/
{
  timespec now;
  clock_gettime(CLOCK_REALTIME, &now);
  osal_timer_t deadline;
  deadline.sec = now.tv_sec + sec;
  deadline.nsec = now.tv_nsec + nsec;
  while (deadline.nsec > 1000000000) {
    deadline.nsec -= 1000000000;
    deadline.sec += 1;
  }
  return deadline;
}

using std::vector;

/* a bit ridiculous, but this is only
   standardized in C++20 */

inline int popcount(ulong bits) {
  int count = 0;
  while (bits != 0) {
    if (bits & 1ul) {
      count++;
    }
    bits = bits >> 1;
  }
  return count;
}

/* this function is used to select some random
   inactive thread from a bitmask of free threads

   The input is a bit mask, and the return value
   is some random position among the bits that
   are set. This needs srand() to be called once
   before for initialization. */

inline int pick_random_from_bits(ulong mask) {
  assert(mask > 0ul);
  uint numbits = popcount(mask);

  int choice = 0;
  if (numbits > 1) {
    choice = rand() % numbits;
  }
  assert(choice < popcount(mask));

  int selected_bit = -1;
  while (choice >= 0) {
    // test rightmost bit and count down
    if (mask & 1ul) {
      choice--;
    }
    mask = mask >> 1;
    selected_bit += 1;
  }
  return selected_bit;
}

inline bool is_realtime() {
  bool runs_realtime = false;
#if __linux__
  if (sched_getscheduler(0) == SCHED_FIFO) {
    runs_realtime = true;
  }
#endif
  return runs_realtime;
}

template <typename T>
vector<T> shuffle_vector(vector<T> ordered_numbers, int seed) {
  vector<T> vec(ordered_numbers); // allocate a copy

  size_t vlen = vec.size();
  srand(seed);
  // shuffling elements with Fisher-Yates shuffle
  // see https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle
  for (size_t i = 0; i < (vlen - 1); i++) {
    size_t new_pos = i + rand() % (vlen - i);
    std::swap(vec[i], vec[new_pos]);
  }
  return vec;
}

inline void wait_nanoseconds(long wait_time) {
  int ret_val = -1;
  struct timespec req_wait = {0, wait_time};

  errno = 0;
  while (ret_val != 0) {
    struct timespec remain_wait = {0, 0};
    ret_val = nanosleep(&req_wait, &remain_wait);
    if ((ret_val == -1) and (errno == EINTR)) {
      req_wait = remain_wait;
      continue;
    }
  }
}
} // namespace testutils
#endif
