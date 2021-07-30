#include <cassert> // assert()
#include <cmath>   // round()

#include <iostream> // todo remove

#include "Image.h"
#include "Kernel.h"
#include "vision.h"

#include "vision_utils.hpp"

namespace
{

  // throwaway functions  assumes zero bias and padding mode valid and kernels are nxn rows and cols
  long inner_product(const vision::sample_t *data, const vision::sample_t *kernel, const int len)
  {
    long accumulator = 0;
    for (int i = 0; i < len; i++)
    {
      accumulator += data[i] * kernel[i];
    }
    return accumulator;
  }

  void conv2d_portable(vision::Image &output, vision::Image &input, vision::Kernel &k, const vision::Plan plan)
  {
    float scale = 1.0f / (k.rows() * k.cols() * k.chans());

    for (int ochan = 0; ochan < k.output_chans(); ochan++)
    {
      for (int orow = 0; orow < output.rows(); orow++)
      {
        for (int ocol = 0; ocol < output.cols(); ocol++)
        {

          int inrow = plan.top_left.row + orow * plan.strides.row;
          int incol = plan.top_left.col + ocol * plan.strides.col;
          int krowstart = inrow - k.rows() / 2;
          int kcolstart = incol - k.cols() / 2;
          int krowend = krowstart + k.rows();
          int kcolend = kcolstart + k.cols();
          long acc = 0;

          if (krowstart >= 0 && kcolstart >= 0 && krowend < input.rows() && kcolend < input.cols())
          {
            for (int krow = 0; krow < k.rows(); krow++)
            {
              acc += inner_product(input.get_pixel(krowstart + krow, kcolstart), k.get_row(krow, ochan), k.row_stride());
            }
          }
          else
          {
            int pad_above = krowstart < 0 ? abs(krowstart) : 0;
            int pad_below = krowend > input.rows() ? krowend - input.rows() : 0;
            int pad_left = kcolstart < 0 ? abs(kcolstart) : 0;
            int pad_right = kcolend > input.cols() ? kcolend - input.cols() : 0;
            int krowlen = k.row_stride() - (pad_left + pad_right) * k.col_stride();

            for (int krow = pad_above; krow < k.rows() - pad_below; krow++)
            {
              acc += inner_product(input.get_pixel(krowstart + krow + pad_above, kcolstart + pad_left), k.get_pixel(krow, pad_left, ochan), krowlen);
            }
            acc += plan.pad_val * ((pad_left + pad_right) * k.col_stride() + (pad_above + pad_below) * k.row_stride());
          }

          *(output.get_pixel(orow, ocol) + ochan) = (vision::sample_t)std::round(acc * scale);
        }
      }
    }
  }

} // end anonymous namespace

namespace vision
{

  void filter(Image &output, Image &input, Kernel &kernel, const Plan plan) // conv 2d with kernels
  {
    // check plan and dimensions
    assert(output.cols() == input.cols() / plan.strides.col);
    assert(output.rows() == input.rows() / plan.strides.row);
    assert(kernel.output_chans() == output.chans());
    assert(kernel.chans() == input.chans());

    conv2d_portable(output, input, kernel, plan); // TODO call lib_nn if it's available

    return;
  }

  void gaussian_blur(Kernel &kernel, int size, double const theta, const unsigned in_chans)
  {
    kernel.clear();
    kernel.resize(size, size, 1, 1);

    // Determine the size for 'half' the vector space
    if (size % 2) // if odd
    {
      size = 1 + size / 2; // Really just a shift right
    }
    else // if even
    {
      size = size / 2;
    }

    // Fill the a new vector with values in 1D gaussian distribution
    std::vector<double> values;
    const float pi = 3.14159265358979323846;
    float fraction1 = 1 / sqrt(2 * pi * theta * theta);
    for (int i = 0; i < size; i++)
    {
      float fraction2 = exp(-(i * i) / (2 * theta * theta));
      float expression = fraction1 * fraction2;
      values.push_back(expression);
    }

    //'reflect' the elements around the center value. [abc]->[cbabc]
    for (int i = 0; i < size - 1; i++)
    {
      values.insert(values.begin() + i, values[values.size() - 1 - i]);
    }

    //Normalize values
    float sum = 0;
    for (const auto &i : values)
      sum = sum + i;

    float coeff = 1 / sum;
    for (int i = 0; i < values.size(); i++)
    {
      values[i] = values[i] * coeff;
    }

    //Create and fill 2d matrix
    std::vector<double> matrix;
    for (int i = 0; i < values.size(); i++)
    {
      for (int j = 0; j < values.size(); j++)
      {
        matrix.push_back(values[i] * values[j]);
      }
    }

    //Convert matrix of doubles into kernel's sample_t matrix via quantization.
    vision::sample_t temp;
    for (int i = 0; i < matrix.size(); i++)
    {
      temp = (sample_t)(std::min(127, (int)(matrix[i] * (128))));
      kernel.image_data->at(i) = temp;
    }

    uncollated_replication(*kernel.image_data, in_chans);
    return;
  }

  void sobel(Kernel &kernel, const unsigned in_chans)
  {
    kernel.clear();
    kernel.resize(3, 3, 1, 2);
    //vector sobel horizontal, sobel vertical;
    sample_t sobel[] = {1, 0, -1,
                        2, 0, -2,
                        1, 0, -1,
                        //output channel boundary
                        1, 2, 1,
                        0, 0, 0,
                        -1, -2, -1};
    for (int i = 0; i < 18; i++)
    {
      kernel.image_data->at(i) = sobel[i];
    }
    uncollated_replication(*kernel.image_data, in_chans);
    return;
  }

  void laplacian(Kernel &kernel, const unsigned in_chans)
  {
    kernel.clear();
    kernel.resize(3, 3, 1, 1);
    sample_t laplacian[] = {
        0,
        1,
        0,
        1,
        -4,
        1,
        0,
        1,
        0,
    };
    for (int i = 0; i < 9; i++)
    {
      kernel.image_data->at(i) = laplacian[i];
    }
    uncollated_replication(*kernel.image_data, in_chans);
    return;
  }

  void resample() // interpolate or decimate
  {
    // convolution or fft?
    return;
  }
  void magnify(const unsigned u, const unsigned v, const float factor)
  {
    assert(factor > 1.0f); // zooming out would confuse caller -- just resample
    // crop square region around u,v based on factor and nearest edge

    // resample by factor
  }

} // namespace vision
