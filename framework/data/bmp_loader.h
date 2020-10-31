// *************************************************************
// File:    bmp_loader.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_BMP_LOADER_H
#define AH_GDM_BMP_LOADER_H

#include <iostream>
#include <vector>

#include "abstract_image.h"

namespace gdm {

struct BmpFileHeader
{
  // https://en.wikipedia.org/wiki/BMP_file_format#Bitmap_file_header

  unsigned short type {};
  unsigned int file_size {};
  unsigned short reserved_1 {};
  unsigned short reserved_2 {};
  unsigned int data_offset {};
};

struct BmpInfoHeader
{
  // https://en.wikipedia.org/wiki/BMP_file_format#DIB_header_(bitmap_information_header)

  unsigned int header_size {};
  signed int width {};
  signed int height {};
  unsigned short planes {};
  unsigned short color_depth {};
  unsigned int compression_method {};
  unsigned int image_size {};
  signed int hor_pixels_per_meter {};
  signed int ver_pixels_per_meter {};
  unsigned int colors_in_palette {};
  unsigned int colors_important {};
};

struct BmpImage : public AbstractImage
{
  BmpImage(const char* fpath);

private:
  BmpFileHeader hdr_file_;
  BmpInfoHeader hdr_info_;

  void FixMisformattedFile(BmpFileHeader& fh, BmpInfoHeader& ih);
  void Swap();

}; // struct BmpImage

namespace helpers {

  auto ReadFileHeader(std::istream& stream) -> BmpFileHeader;
  auto ReadInfoHeader(std::istream& stream) -> BmpInfoHeader;

} // namespace helpers

} // namespace gdm::bmp

#endif // AH_GDM_BMP_LOADER_H