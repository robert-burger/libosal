#include <time.h>
#include <stdlib.h>
#include <vector>

#ifndef LIBOSAL_POSIX_TEST_UTILS_H
#define LIBOSAL_POSIX_TEST_UTILS_H

/* helper function used to randomize wait times */

namespace testutils {

using std::vector;

/* a bit ridiculous, but this is only
   standardized in C++20 */
	
inline int popcount(ulong bits){
	int count=0;
	while (bits != 0){
		if (bits & 1ul){
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
	while (choice >= 0){
		// test rightmost bit and count down
		if (mask & 1ul) {
			choice --;
		}
		mask = mask >> 1;
		selected_bit += 1;
	}
	return selected_bit;
}
		

inline bool is_realtime()
{
  bool runs_realtime = false;
#if __linux__
  if (sched_getscheduler(0) == SCHED_FIFO) {
    runs_realtime = true;
  }
#endif
  return runs_realtime;
}

 
template<typename T>  vector<T> shuffle_vector(vector<T> ordered_numbers,
				       int seed)
  {
    vector<T> vec(ordered_numbers); // allocate a copy

    size_t vlen = vec.size();
    srand(seed);
    // shuffling elements with Fisher-Yates shuffle
    // see https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle
    for(size_t i = 0; i < (vlen - 1); i++){
      size_t new_pos = i + rand() % (vlen - i);
      std::swap(vec[i], vec[new_pos]);
    }
    return vec;
  }


inline void wait_nanoseconds(long wait_time)
{
  int ret_val = -1;
  struct timespec req_wait = {0, wait_time};
  
  errno = 0;
  while (ret_val != 0) {
    struct timespec remain_wait = {0,0};
    ret_val = nanosleep(&req_wait, &remain_wait);
    if ((ret_val == -1) and (errno == EINTR)){
      req_wait = remain_wait;
      continue;
    }
  }
}
}
#endif
