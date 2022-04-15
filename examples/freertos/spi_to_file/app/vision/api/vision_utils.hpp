#pragma once
/**
 * The goal of this file is to contain small, likely inlined,
 * utility functions to reduce dependencies on the standard
 * libraries. Function bodies should be guarded to
 * check for things like platform and debug conditions.
 *
 * */
#include <limits> // numeric_limits<>::min() is_signed()
// #include <math.h>  // round()
#include <cmath>    // trunc()
#include <iostream> // TODO: remove after debugging!

/**
 * Debug printing based on known existing debug prints
 *
 * @note if you want something to print in \emph{release} builds
 * use printf() from <cstdio>
 * **/
#ifndef NDEBUG
#ifdef XCORE
#if (HAS_LIB_RTOS_SUPPORT == 1)
#include "rtos_printf.h"
// __attribute__((weak))
// extern "C" void debug_printf(const char * fmt, va_list args) {rtos_printf(fmt, args);}
#ifdef debug_printf
#undef debug_printf
#endif
#define debug_printf rtos_printf
#else
//#include "debug_print.h" //TODO
// __attribute__((weak))
// extern "C" void debug_printf(const char * fmt, va_list args){printf(fmt, fmt args);}
#include <cstdio>
#ifdef debug_printf
#undef debug_printf
#endif
#define debug_printf printf
#endif
#else
#include <cstdio>
#ifdef debug_printf
#undef debug_printf
#endif
#define debug_printf printf
// __attribute__((weak))
// extern "C" void debug_printf(const char * fmt, va_list args){printf(fmt, args);} TODO
#endif // XCORE
#else
#include <cstdio>
#ifdef debug_printf
#undef debug_printf
#endif
inline void debug_printf(const char *fmt, ...) {}
#endif

namespace vision
{

  /**
   * Round a float number to the nearest integer using bankers' rounding algorithm:
   * x.5 is rounded up or down to the nearest even integer.
   *
   */
  // bankers' rounding
  inline int bround(float x)
  {
    int out;
    if (x - std::trunc(x) == .5 || x - std::trunc(x) == -.5)
    {
      out = round(x / 2) * 2;
    }
    else
    {
      out = round(x);
    }
    return out;
  }
  /**
   * Round a float number to the nearest integer using round-to-nearest-odd integer algorithm:
   * x.5 is rounded up or down to the nearest odd integer.
   *
   */
  // round-to-nearest-odd integer rounding
  inline int obround(float x)
  {
    int out;
    if (x - std::trunc(x) == .5 || x - std::trunc(x) == -.5)
    {
      out = floor(x / 2) * 2 + 1;
    }
    else
    {
      out = round(x);
    }
    return out;
  }

  template <typename T>
  constexpr inline T clamp(int x)
  {
    constexpr int min = std::numeric_limits<T>::min();
    constexpr int max = std::numeric_limits<T>::max();
    return (x <= min ? min : (x > max ? max : x));
  }

  template <typename T>
  T abs(T x) { return x >= 0 ? x : -x; }

  // TODO rand()

  // float tanhf(float x){return x;} todo LUT

  /// Maps min sample value to zero and all other values in ascending order
  inline uint8_t sample_to_index(sample_t x)
  {
    if (std::numeric_limits<sample_t>::is_signed)
    {
      if (sizeof(sample_t) == 1)
        return (uint8_t)(0x000000FF & ((int)x - std::numeric_limits<sample_t>::min()));
      if (sizeof(sample_t) == 2)
        return (uint8_t)(0x0000FFFF & ((int)x - std::numeric_limits<sample_t>::min()));
    }
    return (uint8_t)x;
  }

  /// Maps zero to min sample value and all other values in ascending order
  inline sample_t index_to_sample(int x)
  {
    if (std::numeric_limits<sample_t>::is_signed)
    {
      return (sample_t)(x + std::numeric_limits<sample_t>::min());
    }
    return (sample_t)x;
  }

  /*Uncollated_replication, replacates values in a vector
   *    [abc]->[aabbcc] {replication by 2}
   *
   * Intended to expand the vectors associated with any kernel by n times to account for more in channels.
   */
  inline void uncollated_replication(std::vector<vision::sample_t> &vector, int factor, bool resize = true)
  {
    if (factor < 2)
      return;

    if (resize)
      vector.resize(vector.size() * factor);

    int startIndex = (vector.size() - 1) / factor;

    for (/*startIndex*/; startIndex > -1; startIndex--)
    {
      int dest = startIndex * factor + factor - 1;
      for (int i = 0; i < factor; i++)
      {
        vector[dest - i] = vector[startIndex];
      }
    }
  }

  inline void back2gray(Image &image, const int rows, const int cols)
  {
    for (int i = 0; i < image.image_data->size(); i += 3)
    {
      image.image_data->operator[](i / image.chans()) = (sample_t)(image.image_data->operator[](i));
    }

    image.resize(rows, cols, 1);
    image.set_colorspace(GRAY);
  }

} // namespace vision