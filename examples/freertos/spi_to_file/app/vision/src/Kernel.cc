#include "Kernel.h"

namespace vision
{

    Kernel::Kernel(const int rows_, const int cols_, const int chans_, const int output_chans_) : Image::Image(rows_, cols_, chans_),
                                                                                                  moutput_chans(output_chans_),
                                                                                                  moutput_chan_stride(rows_ * cols_ * chans_)
    {
        image_data->resize(mrows * mcols * mchans * moutput_chans); // TODO there has to be a better way to do this ctor
    }

    Kernel::~Kernel() {}

    sample_t *Kernel::get_output_chan(const int c)
    {
        return image_data->data() + mrows * mcols * mchans * c;
    }

    void Kernel::resize(const int rows_, const int cols_, const int chans_, const int output_chans_)
    {
        image_data->resize(rows_ * cols_ * chans_ * output_chans_);
        mrows = rows_;
        mcols = cols_;
        mchans = chans_;
        moutput_chans = output_chans_;
        mcol_stride = chans_;
        mrow_stride = chans_ * cols_;
        moutput_chan_stride = chans_ * cols_ * rows_;
    }

    sample_t *Kernel::get_row(const int row, const int cout)
    {
        return get_pixel(row, 0, cout);
    }

    sample_t *Kernel::get_pixel(const int row, const int col, const int output_chan)
    {
        return Image::get_pixel(row, col) + output_chan * moutput_chan_stride;
    }

    int Kernel::get_sample_ind(const int row, const int col, const int output_chan)
    {
        return Image::get_sample_ind(row, col) + output_chan * moutput_chan_stride;
    }

    int Kernel::get_sample_ind(const RowCol rc, const int output_chan)
    {
        return Image::get_sample_ind(rc) + output_chan * moutput_chan_stride;
    }

}
//