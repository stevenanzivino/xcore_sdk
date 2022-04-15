#include <cassert>  /* assert */
#include <cstring>  /* std::memmove */
#include <iostream> /* std::cout */
#include <limits>
#include <algorithm> /* std::min */

#include "Image.h"
#include "vision.h"
#include "vision_utils.hpp" /* debug_printf */

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
          debug_printf("%d,", (int)sample_to_index(image_data->operator[](c + i * row_stride() + j * col_stride())));
        }
        debug_printf("\n");
      }
      debug_printf("\n");
    }

    // sanity checks
#ifndef NDEBUG
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
    // sanity checks
#ifndef NDEBUG
    assert(image.is_valid(selection));
#endif

    int len = selection.extents.col * image.col_stride();

    for (int i = 0; i < selection.extents.row; i++)
    {
      std::memmove(image.image_data->data() + i * len,
                   image.get_pixel(selection.top_left.row + i, selection.top_left.col),
                   len);
    }

    // resize image
    image.resize(selection.extents.row, selection.extents.col, image.chans());

    // sanity checks
#ifndef NDEBUG
    int new_image_size = image.rows() * image.cols() * image.chans();
    std::cout << "new_image_size: " << new_image_size << std::endl
              << std::endl;
    assert(new_image_size >= MIN_IMG_SIZE);
    assert(image.rows() >= MIN_IMG_HEIGHT);
    assert(image.cols() >= MIN_IMG_WIDTH);
#endif
  }

  void vertical_pad(Image &image, std::array<uint8_t, 4> color, const int before, const int after)
  {
    // sanity check
    if (before < 0 || after < 0)
      return;

    // params.
    std::vector<sample_t> tmp; // tmp row vector used for padding
    auto old_rows = image.rows();
    auto old_chans = image.chans();

    // resize
    image.resize(image.rows() + before + after, image.cols(), image.chans());
    tmp.resize(image.row_stride());

    // if image is grayscale, convert 1st channel of color to grayscale and use it for padding
    if (old_chans == 1)
    {
      color[0] = round(0.114f * color[0] + 0.5870f * color[1] + 0.299f * color[2]);
    }

    // fill tmp vector with same color pixels
    for (int j = 0; j < image.cols(); j++)
    {
      for (int k = 0; k < image.chans(); k++)
      {
        tmp.data()[j * image.chans() + k] = index_to_sample(color[k]);
      }
    }

    // [bottom]: overwite bottom zero rows with rows of value (moving upwards)
    for (int i = image.rows() - 1; i >= image.rows() - after; i--)
    {
      memcpy(image.get_row(i), tmp.data(), tmp.size());
    }

    // [middle]: shift old image down (moving upwards)
    for (int i = old_rows - 1; i >= 0; i--)
    {
      memcpy(image.get_row(i + before), image.get_row(i), tmp.size());
    }

    // [top]: overwrite old image data with rows of value (moving upwards)
    for (int i = before - 1; i >= 0; i--)
    {
      memcpy(image.get_row(i), tmp.data(), tmp.size());
    }
  }

  void horizontal_pad(Image &image, std::array<uint8_t, 4> color, const int before, const int after)
  {
    // sanity check
    if (before < 0 || after < 0)
      return;

    // params.
    std::vector<sample_t> tmp; // tmp row vector used for padding
    auto old_row_stride = image.row_stride();
    auto old_start = image.get_sample_ind(image.rows() - 1, 0);
    auto old_chans = image.chans();

    // resize
    image.resize(image.rows(), image.cols() + before + after, image.chans());
    tmp.resize(image.row_stride());

    // if image is grayscale, convert 1st channel of color to grayscale and use it for padding
    if (old_chans == 1)
    {
      color[0] = round(0.114f * color[0] + 0.5870f * color[1] + 0.299f * color[2]);
    }

    // overwrite rows (moving upwards)
    for (int i = image.rows() - 1; i >= 0; i--)
    {
      for (int j = 0; j < image.cols(); j++)
      {
        for (int k = 0; k < image.chans(); k++)
        {
          tmp.data()[j * image.chans() + k] = index_to_sample(color[k]);
        }
      }

      // update middle tmp data with old row values
      memcpy(tmp.data() + before * image.chans(), image.image_data->data() + old_start, old_row_stride); // populate new tmp row with before + old_row_stride + after pixels of value

      // update row in image data
      memcpy(image.get_row(i), tmp.data(), tmp.size());

      // update old start point
      old_start -= old_row_stride;
    }
  }

  void horizontal_byte_pad(Image &image, const int pre_bytes, const int post_bytes, uint8_t value)
  {
    // sanity check
    if (pre_bytes < 0 || post_bytes < 0)
      return;

    // params.
    std::vector<sample_t> tmp; // tmp row vector used for padding
    auto old_chans = image.chans();
    auto old_row_stride = image.row_stride();
    auto start = image.get_sample_ind(image.rows() - 1, 0);
    auto pad = pre_bytes + post_bytes;

    // resize
    image.resize(image.rows(), image.cols() * image.chans(), 1);
    image.resize(image.rows(), image.cols() + pad, image.chans());
    tmp.resize(image.row_stride());

    // overwrite rows (moving upwards)
    for (int i = image.rows() - 1; i >= 0; i--)
    {
      memset(tmp.data(), value, tmp.size());
      memcpy(tmp.data() + pre_bytes, image.image_data->data() + start, old_row_stride);
      memcpy(image.get_row(i), tmp.data(), tmp.size());
      start -= old_row_stride;
    }

    // correct img dims after resize and byte padding!
    image.set_dims(image.rows(), image.cols() / old_chans, old_chans);

    if (pre_bytes / old_chans >= 1 && post_bytes % old_chans >= 1) // if pre-/post-bytes is >= n * chans => resize image and increase number of columns
    {
      image.resize(image.rows(), image.cols() / old_chans, old_chans);
    }
  }

  void horizontal_byte_crop(Image &image, const int pre_bytes, const int post_bytes)
  {
    int pad = pre_bytes + post_bytes;
    int old_chans = image.chans();
    int old_cols = image.cols();

    // resize image
    image.resize(image.rows(), image.cols() * image.chans() + pad, 1);

    // overwrite rows (moving upwards)
    for (int i = 0; i < image.rows(); i++)
    {
      memmove(image.image_data->data() + (image.row_stride() - pad) * i, image.image_data->data() + image.row_stride() * i + pre_bytes, image.row_stride() - pad);
    }

    // resize image
    image.resize(image.rows(), old_cols, old_chans);
  }

  void channel_byte_pad(Image &image, const int before, const int after, const uint8_t value)
  {
    // sanity check
    if (before < 0 || after < 0)
      return;

    // params.
    std::vector<sample_t> temp; // tmp row vector used for padding
    int head = image.image_data->size() - 1;

    // resize
    image.resize(image.rows(), image.cols(), image.chans() + before + after);
    temp.resize(image.row_stride());

    memset(temp.data(), index_to_sample(value), temp.size());

    for (int i = image.rows() - 1; i >= 0; i--)
    {
      for (int j = image.cols() - 1; j >= 0; j--)
      {
        // for (int c = before; c < image.chans() - after; c++)
        for (int c = image.chans() - after - 1; c >= before; c--)
        {
          temp[j * image.col_stride() + c] = image.image_data->at(head--);
        }
      }
      memcpy(image.get_row(i), temp.data(), temp.size());
    }
  }

  void find_contour(Image &image, const uint8_t threshold, std::vector<RowCol> &points)
  {
    RowCol point;
    int value;

    // horizontal set
    for (int i = 0; i < image.rows(); i++)
    {
      for (int j = 0; j < image.cols(); j++)
      {
        // check points on the left of the image
        if (j == 0)
        {
          value = *image.get_pixel(i, j);
          if (vision::abs(value) > threshold)
          {
            point.row = i;
            point.col = j;
            points.push_back(point);
          }
        }
        else if (j == image.cols() - 1)
        {
          // check points on the right of the image
          value = *image.get_pixel(i, j);
          if (vision::abs(value) > threshold)
          {
            point.row = i;
            point.col = j;
            points.push_back(point);
          }
        }
        else
        {
          // make horizontal comparison
          value = *image.get_pixel(i, j) - *image.get_pixel(i, j - 1);
          if (vision::abs(value) > threshold)
          {
            point.row = i;
            point.col = j - 1;
            points.push_back(point);
          }
        }
      }
    }

    // vertical set
    for (int i = 0; i < image.rows(); i++)
    {
      for (int j = 0; j < image.cols(); j++)
      {
        // check points on the top of the image
        if (i == 0)
        {
          value = *image.get_pixel(i, j);
          if (vision::abs(value) > threshold)
          {
            point.row = i;
            point.col = j;
            points.push_back(point);
          }
        }
        // check points on the bottom of the image.
        else if (i == image.rows() - 1)
        {
          value = *image.get_pixel(i, j);
          if (vision::abs(value) > threshold)
          {
            point.row = i;
            point.col = j;
            points.push_back(point);
          }
        }
        else
        {
          // make vertical comparison
          value = *image.get_pixel(i, j) - *image.get_pixel(i - 1, j);
          if (vision::abs(value) > threshold)
          {
            point.row = i - 1;
            point.col = j;
            points.push_back(point);
          }
        }
      }
    }
  }

  void get_bounding_box(Region &box, const std::vector<RowCol> points)
  {

    if (points.empty())
    {
      // throw exception or error
      return;
    }

    // calculate box limits
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

    // update box properties
    box.top_left.col = colMin;
    box.top_left.row = rowMin;
    box.extents.col = 1 + colMax - colMin;
    box.extents.row = 1 + rowMax - rowMin;

    // pretty print to screen
    debug_printf("get_bounding_box: row #%d, col #%d | height #%d, width #%d\n", box.top_left.row, box.top_left.col,
                 box.extents.row, box.extents.col);
  }

  void bgr2gray(Image &image)
  {
    // sanity checks
#ifndef NDEBUG
    assert(image.get_colorspace() == BGR);
#endif

    std::vector<float> weights{0.114f, 0.5870f, 0.299f};
    if (image.get_colorspace() == BGR)
    {
      for (int i = 0; i < image.image_data->size(); i += image.chans())
      {
        float tmp = 0.0f;
        for (int j = 0; j < image.chans(); j++)
        {
          tmp += image.image_data->operator[](i + j) * weights[j];
        }

        image.image_data->operator[](i / image.chans()) = (sample_t)(bround(tmp));
      }

      image.resize(image.rows(), image.cols(), 1);
      image.set_colorspace(GRAY);
    }
  }

  void bgr2bgra(Image &image, const uint8_t value)
  {
    // sanity checks
#ifndef NDEBUG
    assert(image.get_colorspace() == BGR);
#endif

    if (image.get_colorspace() == BGR)
    {
      channel_byte_pad(image, 0, 1, value);
      image.set_colorspace(BGRA);
    }
  }

  void bgr2yuv(Image &image)
  {
    // [NVIDIA implementation]: https://docs.nvidia.com/cuda/npp/group__rgbtoyuv.html

    // sanity checks
#ifndef NDEBUG
    assert(image.get_colorspace() == BGR);
#endif

    std::vector<float> weights{0.114f, 0.5870f, 0.299f};
    float y, u, v;

    if (image.get_colorspace() == BGR)
    {
      for (int i = 0; i < image.image_data->size() - image.chans() + 1; i += image.chans())
      {
        y = 0.0f;

        for (int j = 0; j < image.chans(); j++)
        {
          y += sample_to_index(image.image_data->operator[](i + j)) * weights[j];
        }
        u = (sample_to_index(image.image_data->operator[](i)) - y) * 0.492f;
        v = (sample_to_index(image.image_data->operator[](i + 2)) - y) * 0.877f;

        image.image_data->operator[](i) = index_to_sample(bround(y));
        image.image_data->operator[](i + 1) = index_to_sample((int)clamp<uint8_t>(bround(u) + 128));
        image.image_data->operator[](i + 2) = index_to_sample((int)clamp<uint8_t>(bround(v) + 128));
      }
      image.set_colorspace(YUV);
    }
  }

  void yuv2bgr(Image &image)
  {
    // [NVIDIA implementation]: https://docs.nvidia.com/cuda/npp/group__yuvtorgb.html

    // sanity checks
#ifndef NDEBUG
    assert(image.get_colorspace() == YUV);
#endif

    float b, g, r;

    if (image.get_colorspace() == YUV)
    {
      for (int i = 0; i < image.image_data->size() - image.chans() + 1; i += image.chans())
      {
        b = sample_to_index(image.image_data->operator[](i)) + 2.032f * (sample_to_index(image.image_data->operator[](i + 1)) - 128.0f);
        g = sample_to_index(image.image_data->operator[](i)) - 0.394f * (sample_to_index(image.image_data->operator[](i + 1)) - 128.0f) - 0.581f * (sample_to_index(image.image_data->operator[](i + 2)) - 128.0f);
        r = sample_to_index(image.image_data->operator[](i)) + 1.140f * (sample_to_index(image.image_data->operator[](i + 2)) - 128.0f);

        image.image_data->operator[](i) = index_to_sample((int)clamp<uint8_t>(bround(b)));
        image.image_data->operator[](i + 1) = index_to_sample((int)clamp<uint8_t>(bround(g)));
        image.image_data->operator[](i + 2) = index_to_sample((int)clamp<uint8_t>(bround(r)));
      }
      image.set_colorspace(BGR);
    }
  }

  void flip(Image &image, const Dimension dim)
  {
    // sanity checks
#ifndef NDEBUG
    assert(dim == ROW || dim == COL);
#endif

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
      // return Error | Nbits == 0 causes an address sanitizer problem
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

  /*TODO for streaming version take 1 row at a time, chan count and a flag to clear first or not
  also make the histogram vector an array*/
  void hist(const Image &image, std::vector<int> &hist_vec, const int ch, const int nbins)
  {
    uint8_t pixel_value;
    int hist_bin, bin_width;

    // sanity checks
#ifndef NDEBUG
    assert(nbins > 0);
#endif

    // calculate histogram bin width
    bin_width = (int)ceil((float)(std::numeric_limits<sample_t>::max() - std::numeric_limits<sample_t>::min() + 1) / nbins);

    // resize histogram to fit nbins
    hist_vec.resize(nbins);

    // initialize histogram values with 0
    for (auto &e : hist_vec)
    {
      e = 0;
    }

    for (int k = ch; k < image.image_data->size(); k = k + image.chans())
    {
      // signed -> unsigned pixel value
      pixel_value = sample_to_index(image.image_data->at(k));

      // find histogram bin to update
      hist_bin = (int)((float)pixel_value / bin_width);

      // update histogram
      hist_vec[hist_bin]++;
    }
  }

  void adjust_brightness(Image &image, const sample_t mag)
  {
    for (auto &p : *image.image_data)
    {
      p = clamp<sample_t>(p + mag);
    }
  }

  void region2array(const Region &box, std::array<int, 4> &region_array)
  {
    region_array[0] = box.top_left.row;
    region_array[1] = box.top_left.col;
    region_array[2] = box.extents.row;
    region_array[3] = box.extents.col;
  }

} // namespace vision
