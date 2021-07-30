#ifndef VISION_H
#define VISION_H

#include <memory>
#include <vector>
#include <array>

/**
 * Goals of the c++ library
 * - Correct, safe, and portable
 * - Small code size and decently fast (hard float assumed)
 * - Can be used to develop ASM/VPU lib
 *
 * Stretch Goals
 * -Make "Image" and functions using it compatible with MLIR
 * -Even the low performance functions here would benefit from
 *  looking at a whole graph and grouping channel merge/splits
 * **/

namespace vision
{

  /// @note Storage order hard coded to
  /// { CHAN,COL,ROW,OUTCHAN }
  enum Dimension
  {
    CHAN,
    COL,
    ROW,
    OUTCHAN
  };

  /// Useful method names for a variety of algorithms
  enum Method
  {
    NEAREST,
    LINEAR,
    CUBIC,
    BILINEAR
  };

  /// Coordinates
  struct RowCol
  {
    unsigned row;
    unsigned col;
  };

  /// Box defined by top left corner and side lengths
  struct Region
  {
    RowCol top_left;
    RowCol extents;
  };

  /// Fundamental value for a singleton in an Image
  typedef int8_t sample_t;

  /// Instructions for convolutional functions
  struct Plan
  {
    RowCol top_left;
    RowCol strides;
    sample_t pad_val;
  };

  class Image; // forward declaration

  /**
 * Reads a bitmap image.
 **/
  void read_bitmap(const std::string fileName, Image &image, const int debug_mode);
  /**
 * Writes a bitmap image.
 **/
  void write_bitmap(const std::string filename, Image &image, const int debug_mode);

  /** 
 * Crop an image to a Region
 * 
 * @note The pixels outside of Region will be permanently deleted
 **/
  void crop(Image &image, const Region &selection);

  /**
 * Add padding rows above / below an image
 **/
  void vertical_pad(Image &image, const int above, const int below, const vision::sample_t value);

  /**
 * Add padding bytes on the left / right of an image
 **/
  void horizontal_pad(Image &image, int pre_bytes, int post_bytes, char value);

  /**
 * Add padding channels to an entire image
 **/
  void channel_pad(Image &image, int before, int after, vision::sample_t value);

  /**
 * Find contours returns a vector of RowCol pairs indicating each pixel which may be part of a contour.
 * Argument: Image is a reference to an image from which the contours will be found.
 *         Difference is the magnitude of how hard the edge has to be to be considered a contour point.
 * 
 * TBD: Enable functionality for only a region within the image to be analyzed.
 * 
 * Return: a vector of RowCol pairs indicating each pixel which may be part of a contour.
 * 
 * Further Improvements:
 * Do not add a point found in the vertical check if it was already found in the horizontal check.
 * 
 **/
  std::vector<RowCol> find_contour(Image &image, const unsigned char difference /*, Region*/); // todo return type vs reference?

  /**
 * Prints the dimentions of a Region which encapsulates all points passed in.
 * 
 * Argument: points, a vector of RowCol pairs to be considered.
 * 
 * Return: This function currently does not return anything, but prints what it is prepared to return.
 * 
 **/
  void get_bounding_box(const std::vector<RowCol> points); // todo, by reference?

  /**
 * Debayers a caller managed buffer in place.
 * 
 * @return number of bytes written out
 *         If return value is negative:
 *         |-1 | unsupported method |
 *         |-2 | buffer too small   |
 * 
 * Currently supported methods and restrictions:
 * 
 * | NEAREST | buf_len >= 2 * row_len | 
 * 
 * Unless otherwise stated all algorithms will require a buffer
 * that is at least 1 row longer than the current row being processed.
 * 
 * @note images are assumed densely packed in the following order:
 * |RGRGRGRGRGRGRG...|
 * |GBGBGBGBGBGBGB...|
 * |.................|
 * */
  int debayer_stream(sample_t *buffer, int buf_len, int row_len, int current_row, Method method);

  /**
 * Converts an n-channel image to a 1-channel image
 *
 * If there is no weight vector, or a channel mismatch
 * then equal weights will be used.
 *
 * Typical usage would to to convert from a 3-channel
 * BGR image to a 1 channel grayscale image with the
 * a different weight for red green and blue:
 *
 *  std::vector<float> bgr_weights = {0.114f, 0.5870f, 0.299f};
 *  to_grayscale(my_image, bgr_weights);
 **/
  void to_grayscale(Image &image, const std::vector<float> *weights = nullptr);

  /**
 * Flips image about dim.
 *
 * if dim is COL then the order of columns
 * in the image will be swapped (horizontal flip)
 * 
 * if dim is ROW then the order of rows
 * in the image will be swapped (vertical flip)
 *
 * Channel flipping is not supported.
 **/
  void flip(Image &image, Dimension dim);

  /**
 * Rotate an image by (degrees radians?) 
 * TODO
 **/
  void rotate(); // hard and terrible

  /**Dither is an attempt to retain visual fidelity with reduced bit depth.
 * Reducing the bit depth will reduce the number of colors present in an image.
 * 
 * This algorithm currently assumes no padding sections.
 * Add dithering to the lowest N bits or an image 
 * 
 * TODO: Add Error condition if dither size is < 0 or > 8.
 *      Clear out remaining comments / prints
 *      Find a better way to verify accuracy. Gimp appears unreliable
 *      Clean code for comprehension
 **/
  void dither(Image &image, int Nbits);

  /**
 * This function is idealy for grayscale, but should (untested) work for multi-channel images
 * This function assumes no padding.
 * 
 * Arguments:
 *        Image: A reference to the image being affected.
 *        Threshold: The cutoff for the thresholding. 
 *        Binarize: (conditional) Should values above the threshold be set to the maximum?
 * 
 * Behavior:
 *        Acts upon each byte contained in the image and performs thresholding.
 *        All values < threshold -> 0
 *        If binarize is true, values >- threshold are maximized
 * 
 * Outputs:
 *        By the completion of this function the refernced image is thresholded.
 * 
 **/
  void threshold(Image &image, const unsigned char threshold, const bool binarize = false);

  /**
 * Computes a historgram for each channel
 *
 * TODO decide storage order
 **/
  void compute_histogram(Image &image, std::vector<unsigned> &h, unsigned nbins);

  /** 
 * Add mag to every pixel
 * + numbers brighten
 * - numbers darken
 * 
 * Saturates at max and min range of sample_t
 * **/
  void adjust_brightness(Image &image, sample_t mag);

  /**
 * Increase the contrast ratio of image by ratio
 * 
 * Ratio values $ \in{} (0,1) $ will decrease contrast
**/
  void adjust_contrast(Image &image, float ratio);

  // fft

  void fft(); // 2D fft specifics TBD
  void ifft();

  // convolutions

  class Kernel; // forward declaration

  //TODO: make a check-plan function that looks at plan,kernel,image to see if it's OK
  // this functiion should just return true outside of debug mode...

  /**
 * General 2D filtering function
 **/
  void filter(Image &output, Image &input, Kernel &kernel, Plan plan);

  // erode?
  // prewitt?
  // robinson?

  /**
 * Receives a reference to a kernel, the size of the output, the deviation, and desired input channels.
 * 
 * This function creates a 1D gaussian vector then performs matrix multiplication to get a 2D version.
 * 
 * Implemented to represent: https://en.wikipedia.org/wiki/Gaussian_blur
 *  Has been verified on size 7.
 * 
 * May need to replicate values depending on the value of in_chans. 
 * Since uncollated_replication resizes based on in_chans, we assume in_chans to be 1 unless otherwise specified.
 * 
 * Note: This function will slightly dim the image due to the rounding loss in the quantization step.
 * 
 * TODO:
 * Attempt to move as much of this math into the compilation process as possible. Minimize runtime cost.
 * 
 **/
  void gaussian_blur(Kernel &kernel, int size, double const theta, const unsigned in_chan = 1);

  /**
 * Receives a reference to a kernel and the desired input channels.
 * Overwrite all data to remake the kernel.
 * 
 * May need to replicate values depending on the value of in_chans. 
 * Since uncollated_replication resizes based on in_chans, we assume in_chans to be 1 unless otherwise specified.
 * 
 * Implemented to represent this: https://en.wikipedia.org/wiki/Sobel_operator#Technical_details
 *      Though that page also offers a 3 layer deep version.
 **/
  void sobel(Kernel &kernel, const unsigned in_chan = 1);

  /**
 * Receives a reference to a kernel and the desired input channels.
 * Overwrites all data to remake the kernel.
 * 
 * May need to replicate values depending on the value of in_chans. 
 * Since uncollated_replication resizes based on in_chans, we assume in_chans to be 1 unless otherwise specified.
 * 
 * Implemented to represent this: https://en.wikipedia.org/wiki/Discrete_Laplace_operator#Image_processing
 **/
  void laplacian(Kernel &kernel, const unsigned in_chan = 1);

  /**
 * 
**/
  void resample(); // interpolate or decimate

  /**
 * Resample and crop and image to zoom in / out at a specified point
 * 
 * @note this function will not change the image storage size
**/
  void magnify(unsigned u, unsigned v, float factor);

  // Drawing functions

  /**
 * Adds a colored rectangle to an image around region 
 **/
  void draw_rect(Image &image, std::array<uint8_t, 3> color, const Region region, const int thickness, const bool isGray);

} // namespace vision

#endif // VISION_H