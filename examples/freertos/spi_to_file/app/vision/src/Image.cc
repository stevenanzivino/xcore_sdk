#include "Image.h"

namespace vision
{

  Image::Image(int rows_, int cols_, int chans_, Colorspace color_)
      : mchans(chans_),
        mcols(cols_),
        mrows(rows_),
        mcol_stride(chans_),
        mrow_stride(cols_ * chans_),
        mcolor(color_),
        image_data(new std::vector<sample_t>(rows_ * cols_ * chans_)) {}

  Image::~Image() {}

  // TODO: this has non-obvious behavior since it doesn't preserve the upper left corner in most cases...
  // but crop contains all of the needed functionality
  void Image::resize(int rows_, int cols_, int chans_)
  {
    if (rows_ * cols_ * chans_ != image_data->size())
    {
      image_data->resize(rows_ * cols_ * chans_);
    }
    mrows = rows_;
    mcols = cols_;
    mchans = chans_;
    mcol_stride = chans_;
    mrow_stride = chans_ * cols_;
  }

  // This function sets the colorspace of the image
  void Image::set_colorspace(Colorspace color_)
  {
    mcolor = color_;
  }

  // This function sets the colorspace of the image
  Colorspace Image::get_colorspace()
  {
    return mcolor;
  }

  void Image::clear()
  {
    // compiler will match memset, no dependency on <string> this way
    for (int i = 0; i < image_data->size(); i++)
      *(image_data->data() + i) = 0;
  }

  sample_t *Image::get_row(int row)
  {
    return get_pixel(row, 0);
  }

  sample_t *Image::get_pixel(int row, int col) const
  {
    int tmp = get_sample_ind(row, col);
    return image_data->data() + tmp;
  }

  sample_t Image::get_sample(int row, int col, int chan) const
  {
    int tmp = get_sample_ind(row, col) + chan;
    return image_data->at(tmp);
  }

  int Image::get_sample_ind(int row, int col) const
  {
    return row * mrow_stride + col * mcol_stride;
  }
  int Image::get_sample_ind(RowCol rc) const
  {
    return rc.row * mrow_stride + rc.col * mcol_stride;
  }

  bool Image::is_valid(Region r) const
  {
    // region's values being unsigned GREATLY simplifies this
    if (r.top_left.col + r.extents.col > mcols ||
        r.top_left.row + r.extents.row > mrows ||
        r.extents.col <= 0 ||
        r.extents.row <= 0)
      return false;

    return true;
  }

} // namespace vision
