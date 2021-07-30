#include <algorithm> // rotate()
#include <cassert>   // assert()
#include <cstring>   // memmove()
#include <math.h>    // round()

#include "Image.h"
#include "vision.h"
#include "vision_utils.hpp"

namespace
{

  int debayer_stream_nearest(vision::sample_t *buffer, int row_len, const int current_row)
  {
    // nearest neighbor with tie broken by memory safety / algorithmic complexity
    if (current_row % 2 == 0)
    { // even rows start with red
      for (int i = 0; i < row_len; i++)
      {
        if (i % 6 > 3)
          buffer[i] = buffer[i + row_len]; // down
        else if (i % 6 == 2)
          buffer[i] = buffer[i + row_len - 1]; // down and left (get previous blue)
        else if (i % 6 == 3)
          buffer[i] = buffer[i + 1]; // right (get next red)
        // else no update
      }
    }
    else
    {
      for (int i = 0; i < row_len; i++)
      { // odd rows start with blue
        if (i % 6 < 2)
          buffer[i] = buffer[i + row_len]; // down
        else if (i % 6 == 3)
          buffer[i] = buffer[i + row_len - 1]; // down and left
        else if (i % 6 == 2)
          buffer[i] = buffer[i + 1]; // right
        // else no ipdate
      }
    }

    return 0;
  }

  vision::sample_t safeSum(const int addendA, const int addendB)
  {
    int sum = addendA + addendB;
    if (sum > std::numeric_limits<vision::sample_t>::max())
    {
      return ((vision::sample_t)std::numeric_limits<vision::sample_t>::max());
    }
    else if (sum < std::numeric_limits<vision::sample_t>::min())
    {
      return ((vision::sample_t)std::numeric_limits<vision::sample_t>::min());
    }
    return ((vision::sample_t)sum);
  }

} // namespace

namespace vision
{

  // print verbose info on image
  void Image::print()
  {
#ifndef NDEBUG
    int rcount = std::min(rows(), 10);
    int ccount = std::min(cols(), 10);
    debug_printf(
        "Rows: %d\nCols: %d\nChans: %d\ncol_stride: %d\nrow_stride: %d\
        \nDisplaying first %d rows and %d columns\n",
        rows(), cols(), chans(),
        col_stride(), row_stride(), rcount, ccount);

    for (int c = 0; c < chans(); c++)
    {
      debug_printf("Channel: %d\n", c);
      for (int i = 0; i < rcount; i++)
      {
        for (int j = 0; j < ccount; j++)
        {
          debug_printf("0x%02X,", sample_to_index(image_data->operator[](c + i * row_stride() + j * col_stride())));
        }
        debug_printf("\n");
      }
      debug_printf("\n");
    }
    assert(col_stride() == chans());
    assert(row_stride() == chans() * cols());
    assert(image_data->size() == rows() * cols() * chans());
    assert(image_data->size() > 0);
#endif
  }

  // debayer input image stream
  int debayer_stream(sample_t *buffer, const int buf_len, const int row_len, const int current_row, const Method method)
  {
    switch (method)
    {
    case NEAREST:
      if (buf_len < 2 * row_len)
      {
        return -2;
      }
      else
      {
        return debayer_stream_nearest(buffer, row_len, current_row);
      }
      break;
    default:
      return -1;
    }
  }

  void crop(Image &image, const Region &selection)
  {
    assert(image.is_valid(selection));

    std::rotate(image.image_data->begin(),
                image.image_data->begin() + image.get_sample_ind(selection.top_left),
                image.image_data->end());
    int len = selection.extents.col * image.col_stride();
    int skip = image.row_stride() - len; // if rows are padded crop will remove
    assert(skip >= 0);
    for (int i = 1; i < selection.extents.row; i++)
    {
      sample_t *dst = image.image_data->data() + image.get_sample_ind(i, i * len);
      sample_t *src = image.image_data->data() + image.get_sample_ind(i, i * len + skip);
      std::memmove(dst, src, len); // probably safe to use memcpy on xcore
    }
    // resize image
    image.resize(selection.extents.row, selection.extents.col, image.chans());
  }

  void vertical_pad(Image &image, const int above,const int below,const sample_t value)
  {
    if (above < 0 || below < 0)
      return;

    auto old_rows = image.rows();
    auto pad = above + below;
    image.resize(image.rows()+pad,image.cols(),image.chans());

    //Middle
    for(int i = old_rows - 1; i >= 0 ; i--){
      memcpy(image.get_row(i+above),image.get_row(i),image.row_stride());
    }

    std::vector<sample_t> tmp;
    tmp.resize(image.row_stride());
    memset(tmp.data(), value, tmp.size());
    //Bottom
    for(int i = image.rows()-1; i >= old_rows; i--){
      memcpy(image.get_row(i), tmp.data(),tmp.size());
    }
    //Top
    for(int i = above-1; i >=0; i--){
      memcpy(image.get_row(i), tmp.data(),tmp.size());
    }
  }

  void horizontal_pad(Image &image, const int pre_bytes, const int post_bytes, char value)
  {
    if (pre_bytes < 0 || post_bytes < 0)
      return;
    auto old_chans = image.chans();
    auto old_row_stride = image.row_stride();
    auto start = image.get_sample_ind(image.rows() - 1, 0);
    auto pad = pre_bytes + post_bytes;

    image.resize(image.rows(), image.cols() * image.chans(), 1);   // reshape
    image.resize(image.rows(), image.cols() + pad, image.chans()); // TODO better handle sub-pixel pads
    std::vector<sample_t> tmp;                                     // TODO in-place -- though currently this is only used with fileIO
    tmp.resize(image.row_stride());

    for (int i = image.rows() - 1; i >= 0; i--)
    {
      memset(tmp.data(), value, tmp.size());
      memcpy(tmp.data() + pre_bytes, image.image_data->data() + start, old_row_stride);
      memcpy(image.get_row(i), tmp.data(), tmp.size());
      start -= old_row_stride;
    }

    if (pre_bytes % old_chans == 0 && post_bytes % old_chans == 0)
    {
      image.resize(image.rows(), image.cols() / old_chans, old_chans);
    }
  }

  void channel_pad(Image &image, const int before, const int after)
  {
    if (before < 0 || after < 0)
      return;
    image.resize(image.rows(), image.cols(), image.chans() + before + after);
  }

  std::vector<RowCol> find_contour(Image &image, const unsigned char difference /*, Region*/)
  {
    RowCol point;
    std::vector<RowCol> points;

    //horizontal set
    for (int i = 1; i < image.rows() - 1; i++)
    {
      for (int j = 1; j < image.cols() - 2; j++)
      {
        //char has size 0-255
        int value1 = -1 * *image.get_pixel(i - 1, j - 1) + 1 * *image.get_pixel(i - 1, j + 1) - 2 * *image.get_pixel(i, j - 1) + 2 * *image.get_pixel(i, j + 1) - 1 * *image.get_pixel(i + 1, j - 1) + 1 * *image.get_pixel(i + 1, j + 1);

        int value2 = -1 * *image.get_pixel(i - 1, j) + 1 * *image.get_pixel(i - 1, j + 2) - 2 * *image.get_pixel(i, j) + 2 * *image.get_pixel(i, j + 2) - 1 * *image.get_pixel(i + 1, j) + 1 * *image.get_pixel(i + 1, j + 2);
        // ^ with some algebra a bunch of these terms can probably be grouped / go away
        if (vision::abs(value1 - value2) > difference)
        {
          point.row = i;
          point.col = j;
          points.push_back(point);
        }
      }
    }
    //Vertical Set //i = y, j = x
    for (int i = 1; i < image.rows() - 2; i++)
    {
      for (int j = 1; j < image.cols() - 1; j++)
      {
        int value1 = -1 * *image.get_pixel(i - 1, j - 1) + 1 * *image.get_pixel(i + 1, j - 1) - 2 * *image.get_pixel(i - 1, j) + 2 * *image.get_pixel(i + 1, j) - 1 * *image.get_pixel(i - 1, j + 1) + 1 * *image.get_pixel(i + 1, j + 1);

        int value2 = -1 * *image.get_pixel(i, j - 1) + 1 * *image.get_pixel(i + 2, j - 1) - 2 * *image.get_pixel(i, j) + 2 * *image.get_pixel(i + 2, j) - 1 * *image.get_pixel(i, j + 1) + 1 * *image.get_pixel(i + 2, j + 1);
        if (vision::abs(value1 - value2) > difference)
        {
          //Consider adding a check if point is already in vector
          point.row = i;
          point.col = j;
          points.push_back(point);
        }
      }
    }
    return points;
  }

  void get_bounding_box(const std::vector<RowCol> points)
  {

    if (points.empty())
    {
      //thow exception or error
      return;
    }

    unsigned rowMax = points[0].row, rowMin = points[0].row;
    unsigned colMax = points[0].col, colMin = points[0].col;
    for (const auto &i : points)
    {
      if (i.row > rowMax)
      {
        rowMax = i.row;
      }
      else if (i.row < rowMin)
      {
        rowMin = i.row;
      }

      if (i.col > colMax)
      {
        colMax = i.col;
      }
      else if (i.col < colMin)
      {
        colMin = i.col;
      }
    }

    Region box;
    box.top_left.col = colMin;
    box.top_left.row = rowMin;
    box.extents.col = 1 + colMax - colMin;
    box.extents.row = 1 + rowMax - rowMin;

    debug_printf("get_bounding_box: %d,%d|%d,%d\n", box.top_left.col, box.top_left.row,
                 box.extents.col, box.extents.col);

    return;
  }

  void to_grayscale(Image &image, const std::vector<float> *weights)
  {
    std::vector<float> w;

    if (weights->empty() || weights->size() != image.chans())
    {
      for (w.push_back(1.0f / image.chans()); w.size() < image.chans(); /**/)
        ;
    }
    else
    {
      w = *weights;
    }

    for (int i = 0; i < image.image_data->size(); i += image.chans())
    {
      // less ugly if image_data were a concrete member
      float tmp = 0.0f;
      for (int j = 0; j < image.chans(); j++)
      {
        tmp += image.image_data->operator[](i + j) * w[j];
      }

      // todo use an iterator below:
      image.image_data->operator[](i / image.chans()) = (sample_t)(round(tmp));
    }

    image.resize(image.rows(), image.cols(), 1);
  }

  void flip(Image &image, const Dimension dim)
  {
    assert(dim == ROW || dim == COL);

    if (dim == ROW)
    {
      int len = image.row_stride();
      std::vector<sample_t> temp(len);
      for (int i = 0; i < image.rows() / 2; i++)
      {
        // back to temp
        std::memcpy(temp.data(), image.get_row(image.rows() - i - 1), len);
        // front to back
        std::memcpy(image.get_row(image.rows() - i - 1), image.get_row(i), len);
        // temp to front
        std::memcpy(image.get_row(i), temp.data(), len);
      }
    }
    else
    {
      int len = image.col_stride();
      std::vector<sample_t> temp(len);
      for (int i = 0; i < image.rows(); i++)
      {
        for (int j = 0; j < image.cols() / 2; j++)
        {
          sample_t *first =
              image.image_data->data() + i * image.row_stride() + j * image.col_stride();
          sample_t *last = image.image_data->data() + (i + 1) * image.row_stride() -
                           (j + 1) * image.col_stride();
          std::memcpy(temp.data(), first, len);
          std::memcpy(first, last, len);
          std::memcpy(last, temp.data(), len);
        }
      }
    }
  }
  void rotate()
  {
  }

  void dither(Image &image, const int Nbits)
  {
    if (Nbits < 1 || Nbits > 8)
    {
      //return Error | Nbits == 0 causes an address sanitizer problem
      return;
    }

    int typeMax = 1 << (sizeof(sample_t) * 8);
    int range = typeMax / ((1 << Nbits) - 1);
    int threshold = range >> 1;

    for (int i = 0; i < image.rows(); i++)
    {
      for (int j = 0; j < image.cols(); j++)
      {
        int pixel = image.get_sample_ind(i, j);
        for (int k = 0; k < image.chans(); k++)
        {
          int index = pixel + k;
          sample_t value = image.image_data->at(index);
          int intensity = (int)value + (typeMax >> 1);
          int normalizedIntensity = intensity % range;

          sample_t error = 0;
          if (normalizedIntensity > threshold)
          {
            error = normalizedIntensity - range;
          }
          else
          {
            error = safeSum(normalizedIntensity, 0);
          }

          image.image_data->at(index) = safeSum(value, -error);

          if (j + 1 < image.cols())
          {
            int indexA = image.get_sample_ind(i, j + 1) + k;
            image.image_data->at(indexA) = safeSum(image.image_data->at(indexA), error * 7 / 16);
          }

          if (i + 1 < image.rows())
          {
            if (j - 1 > 0)
            {
              int indexB = image.get_sample_ind(i + 1, j - 1) + k;
              image.image_data->at(indexB) = safeSum(image.image_data->at(indexB), error * 3 / 16);
            }

            int indexC = image.get_sample_ind(i + 1, j) + k;
            image.image_data->at(indexC) = safeSum(image.image_data->at(indexC), error * 5 / 16);

            if (j + 1 < image.cols())
            {
              int indexD = image.get_sample_ind(i + 1, j + 1) + k;
              image.image_data->at(indexD) = safeSum(image.image_data->at(indexD), error * 1 / 16);
            }
          }
        }
      }
    }
  }

  void compute_histogram(Image &image, std::vector<unsigned> &h, const unsigned nbins)
  {
    int bin_width = (1 << (8 * sizeof(sample_t))) / nbins;

    h.resize(image.chans() * nbins);
    for (auto &e : h)
      e = 0;

    for (int k = 0; k < image.image_data->size(); k++)
    {
      int c = k % image.chans();
      int bin = sample_to_index(image.image_data->operator[](k)) / bin_width;
      h[c * nbins + bin]++;
    }
  }

  void adjust_brightness(Image &image, const sample_t mag)
  {
    constexpr int min = std::numeric_limits<sample_t>::min();
    constexpr int max = std::numeric_limits<sample_t>::max();
    for (auto &p : *image.image_data)
    {
      p = clamp(p + mag, min, max);
    }
  }

} // namespace vision
