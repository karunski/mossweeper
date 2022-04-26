#include "rand.h"

namespace {

unsigned rand_state = 1;

}

void srand(unsigned seed) {
  rand_state = seed;
}

int rand() {

  rand_state ^= rand_state << 7;
  rand_state ^= rand_state >> 9;
  rand_state ^= rand_state << 8;
  return static_cast<int>(rand_state);
}
