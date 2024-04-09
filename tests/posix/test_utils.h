#include <time.h>
#include <stdlib.h>
#include <vector>

#ifndef TEST_UTILS_H
#define TEST_UTILS_H

/* helper function used to randomize wait times */

namespace testutils {

using std::vector;

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
