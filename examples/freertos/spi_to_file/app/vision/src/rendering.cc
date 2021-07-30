#include "Image.h"
#include "vision.h"
#include "vision_utils.hpp"
#include <algorithm> //std::max, std::min
#include <array>
#include <cmath>  //std::ceil
#include <math.h> //round

namespace vision
{

  void paint(Image &image, const int row, const int col, const std::array<uint8_t, 3> color)
  {
    for (int k = 0; k < image.chans(); k++)
    {
      *(image.get_pixel(row, col) + k) = index_to_sample(color[k]);
    }
  }

  void draw_rect(Image &image, std::array<uint8_t, 3> color, const Region region, const int thickness, const bool isGray)
  {
    if (isGray == true) // if image is grayscale, convert rectangle color to grayscale
      for (int k = 0; k < image.chans(); k++)
      {
        color[k] = round(0.114f * color[0] + 0.5870f * color[1] + 0.299f * color[2]);
      }

    if (thickness == 0 or thickness == 1)
    {
      // top side
      for (int j = std::max(std::min((signed)region.top_left.col, (signed)region.extents.col), 0);
           j <= std::min(std::max((signed)region.top_left.col, (signed)region.extents.col), image.cols());
           j++)
      {
        paint(image, region.top_left.row, j, color);
      }
      // bottom side
      for (int j = std::max(std::min((signed)region.top_left.col, (signed)region.extents.col), 0);
           j <= std::min(std::max((signed)region.top_left.col, (signed)region.extents.col), image.cols());
           j++)
      {
        paint(image, region.extents.row, j, color);
      }
      // left side
      for (int i = std::max(std::min((signed)region.top_left.row, (signed)region.extents.row), 0);
           i <= std::min(std::max((signed)region.top_left.row, (signed)region.extents.row), image.rows());
           i++)
      {
        paint(image, i, region.top_left.col, color);
      }
      // right side
      for (int i = std::max(std::min((signed)region.top_left.row, (signed)region.extents.row), 0);
           i <= std::min(std::max((signed)region.top_left.row, (signed)region.extents.row), image.rows());
           i++)
      {
        paint(image, i, region.extents.col, color);
      }
    }
    else
    {
      // top side
      for (int i = std::max(std::min((signed)region.top_left.row, (signed)region.extents.row) - (signed)std::ceil((thickness + 1) / 2), 0);
           i <= std::min(std::max((signed)region.top_left.row, (signed)region.extents.row) + (signed)std::ceil((thickness + 1) / 2), image.rows());
           i++)
      {
        for (int j = std::max(std::min((signed)region.top_left.col, (signed)region.extents.col) - (signed)std::ceil((thickness + 1) / 2), 0);
             j <= std::min(std::max((signed)region.top_left.col, (signed)region.extents.col) + (signed)std::ceil((thickness + 1) / 2), image.cols());
             j++)
        {
          paint(image, i, j, color);
        }
      }

      // bottom side
      for (int i = std::max(std::min((signed)region.top_left.row, (signed)region.extents.row) - (signed)std::ceil((thickness + 1) / 2), 0);
           i <= std::min(std::max((signed)region.top_left.row, (signed)region.extents.row) + (signed)std::ceil((thickness + 1) / 2), image.rows());
           i++)
      {
        for (int j = std::max(std::min((signed)region.top_left.col, (signed)region.extents.col) - (signed)std::ceil((thickness + 1) / 2), 0);
             j <= std::min(std::max((signed)region.top_left.col, (signed)region.extents.col) + (signed)std::ceil((thickness + 1) / 2), image.cols());
             j++)
        {
          paint(image, i, j, color);
        }
      }

      // left side
      for (int i = std::max(std::min((signed)region.top_left.row, (signed)region.extents.row) - (signed)std::ceil((thickness + 1) / 2), 0);
           i <= std::min(std::max((signed)region.top_left.row, (signed)region.extents.row) + (signed)std::ceil((thickness + 1) / 2), image.rows());
           i++)
      {
        for (int j = std::max(std::min((signed)region.top_left.col, (signed)region.extents.col) - (signed)std::ceil((thickness + 1) / 2), 0);
             j <= std::min(std::max((signed)region.top_left.col, (signed)region.extents.col) + (signed)std::ceil((thickness + 1) / 2), image.cols());
             j++)
        {
          paint(image, i, j, color);
        }
      }

      // right side
      for (int i = std::max(std::min((signed)region.top_left.row, (signed)region.extents.row) - (signed)std::ceil((thickness + 1) / 2), 0);
           i <= std::min(std::max((signed)region.top_left.row, (signed)region.extents.row) + (signed)std::ceil((thickness + 1) / 2), image.rows());
           i++)
      {
        for (int j = std::max(std::min((signed)region.top_left.col, (signed)region.extents.col) - (signed)std::ceil((thickness + 1) / 2), 0);
             j <= std::min(std::max((signed)region.top_left.col, (signed)region.extents.col) + (signed)std::ceil((thickness + 1) / 2), image.cols());
             j++)
        {
          paint(image, i, j, color);
        }
      }
    }
  }

} // namespace vision
