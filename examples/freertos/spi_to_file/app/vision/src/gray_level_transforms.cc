#include <cassert> // assert()
#include <cmath>   // tanhf()  -- 4k of code, replace with 256 byte lut
#include <limits>

#include "Image.h"
#include "vision.h"
#include "vision_utils.hpp"

// TODO: decide on linkage of functions, and what the interface
// should be looking up...

/* TODO: Implement the following default functions:
        - adjust contrast (tanh?) -- float param
        - gray level linear transform -- float param
        - gray level log transform -- float param
        - gray level power transform -- float param
        - user defined function -- since we are making a lut anyway
*/

namespace
{

  vision::sample_t tbd(const vision::sample_t arg, const float slope)
  {
    static_assert(std::numeric_limits<vision::sample_t>().is_integer, "sample_t must be an integer type");
    constexpr int max = std::numeric_limits<vision::sample_t>().max();
    constexpr int min = std::numeric_limits<vision::sample_t>().min();
    float x = slope / max;
    if (min < 0)
    {
      x = tanhf(arg * x) * max;
    }
    else
    { // TODO make sure unsigned casts resolve correct index
      x = max * tanhf((arg - max / 2) * x) + max / 2;
    }

    return (vision::sample_t)vision::clamp((int)x, min, max);
  }

} // end anonymous namespace

namespace vision
{

  void adjust_contrast(Image &image, const float ratio)
  {

    std::vector<sample_t> lut;
    if (sizeof(sample_t) <= 16)
    { // prevent really large luts
      // shame on the compiler if it calls malloc more than zero times
      for (int i = 0; i <= 255; lut.emplace_back(tbd((sample_t)(i++), ratio)))
        ;

      for (auto &p : *(image.image_data))
        p = lut[sample_to_index(p)];

      // TODO: lib_nn activation function
      for (int i = 0; i <= 255; i++)
      {
        debug_printf("%d ", (unsigned)lut[i]);
      }
    }
    else
    {
      for (auto &p : *image.image_data)
        p = tbd(p, ratio);
    }
  }

  void threshold(Image &image, const unsigned char threshold, const bool binarize)
  {
    // This function is ideally for grayscale, but should (untested) work for colored.
    // Assumes no padding
    // All values < threshold -> 0
    // If binarize is true values >= threshold are maximized.
    debug_printf("threshold\n");

    if (binarize)
    {
      for (auto &pix : *image.image_data)
      {
        (pix < threshold ? pix = 0 : pix = (sample_t)-1);
      }
    }
    else
    {
      for (auto &pix : *image.image_data)
      {
        if (pix < threshold)
          pix = 0;
      }
    }
    return;
  }
} // namespace vision
