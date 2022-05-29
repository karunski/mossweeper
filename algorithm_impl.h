#ifndef ALGORITHM_IMPL_H
#define ALGORITHM_IMPL_H

namespace std {

template <class T> const T &min(const T &a, const T &b) {
  return (b < a) ? b : a;
}

template <class T> const T &max(const T &a, const T &b) {
  return (a < b) ? b : a;
}
} // namespace std

#endif
