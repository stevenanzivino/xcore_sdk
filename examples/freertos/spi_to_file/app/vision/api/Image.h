#ifndef IMAGE_H
#define IMAGE_H

#include <memory>
#include <vector>

#include "vision.h"

namespace vision
{

  /**
 * Simple container for image data with meta data and helper functions
 * 
 * @note end-user functions are declared in vision.h use methods other than print() at your own risk
 **/
  class Image
  {

  protected:
    int mchans, mcols, mrows;
    int mcol_stride, mrow_stride;
    Colorspace mcolor;

  public:
    /// Construct an empty image with these dimensions
    Image(int rows_, int cols_, int chans_ = 1, Colorspace color_ = BGR);
    ~Image();

    int chans() const { return mchans; }
    int cols() const { return mcols; }
    int rows() const { return mrows; }
    int col_stride() const { return mcol_stride; }
    int row_stride() const { return mrow_stride; }
    Colorspace color() const { return mcolor; }

    /// @note: this changes the size but doesn't crop/pad
    void resize(int rows_, int cols_, int chans_);
    /// Set colorspace
    void set_colorspace(Colorspace color_);
    Colorspace get_colorspace();
    void clear();
    sample_t *get_row(int row);

    /**
   * @brief Get the pixel at a position of [row, col]
   * 
   * @param row The row
   * @param col The col
   * @return sample_t* 
   */
    sample_t *get_pixel(int row, int col) const;
    sample_t get_sample(int row, int col, int chan) const;
    int get_sample_ind(int row, int col) const;
    int get_sample_ind(RowCol rc) const;
    bool is_valid(Region r) const;

    /** The image data is currently held as a unique pointer
   * @note This is to allow the actual buffer to be in DDR or elsewhere
   * which might be misguided -- maybe just make this a concrete vector
   * 
   * The image data is stored as rows * cols * chans where a chan is a single
   * value of sample_t for example if you have a 2*2*3 rgb image:\begin{equation}
   * [r_{00}, g_{00}, b_{00}, r_{01}, g_{01}, b_{01}, ..., r_{10}, g_{10}, b_{10}, r_{11}, g_{11}, b_{11}]
   * \end{equation}
  **/
    std::unique_ptr<std::vector<sample_t>> image_data;

    virtual void print();
  };
} // namespace vision
#endif // IMAGE_H