#ifndef KERNEL_H
#define KERNEL_H
#include "Image.h"

namespace vision {

class Kernel : public Image {

 protected:
  int moutput_chans;
  int moutput_chan_stride;

 public:

  int output_chans() {return moutput_chans;}
  int output_chan_stride() {return moutput_chan_stride;}

  /// Construct an empty kernel with these dimensions
  Kernel(int rows_, int cols_, int chans_ = 1, int output_chans_ = 1);

  ~Kernel();

  sample_t* get_output_chan(int index);

  void resize(int rows_, int cols_, int chans_) = delete;
  void resize(int rows_, int cols_, int chans_, int output_chans_);

  sample_t* get_row(int row) = delete;
  sample_t* get_row(int row, int cout);

  sample_t* get_pixel(int row, int col) = delete;
  sample_t* get_pixel(int row, int col, int output_chan = 0);
  
  int get_sample_ind(int row, int col) = delete;
  int get_sample_ind(int row, int col, int output_chan = 0);

  int get_sample_ind(RowCol rc) = delete;
  int get_sample_ind(RowCol rc, int output_chan = 0);

};

}  // namespace vision
#endif  // KERNEL_H
