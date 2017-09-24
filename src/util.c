#include "util.h"

#include "math.h"

unsigned int util_digits(double d) {
  return floor(log10(d));
}
