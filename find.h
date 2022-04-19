#ifndef MOS_FIND_H
#define MOS_FIND_H

namespace std {
  
template <class InputIt, class T>
constexpr InputIt find(InputIt first, InputIt last, const T &value) {
  for (; first != last; ++first) {
    if (*first == value) {
      return first;
    }
  }
  return last;
}

}
#endif
