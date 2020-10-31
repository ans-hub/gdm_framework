// *************************************************************
// File:    tga_loader.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_TGA_LOADER_H
#define AH_GDM_TGA_LOADER_H

#include <iostream>
#include <vector>

#include "abstract_image.h"

namespace gdm {

struct TgaFileHeader
{
	// https://en.wikipedia.org/wiki/Truevision_TGA#Header

  unsigned char id_length {};
  uint8_t color_map_type {};
  uint8_t image_type {};

  unsigned short color_map_origin {};
  unsigned short color_map_length {};
  unsigned char color_map_entry_size {};

  unsigned short x_origin {};
  unsigned short y_origin {};
  unsigned short width {};
  unsigned short height {};
  unsigned char depth {};
  unsigned char image_descriptor {};

}; // struct TgaFileHeader

struct TgaHelpHeader
{
	// Helper non standartized struct to hold some useful data 
	
  unsigned char bpp {};
  unsigned int uncompressed_sz {};
  unsigned int pixels_cnt {};
	
}; // struct HelpHeader

struct TgaImage : public AbstractImage
{
  TgaImage(const char* fpath);

private:
  TgaFileHeader hdr_file_;
  TgaHelpHeader hdr_help_;

private:
  bool LoadUncompressed(std::istream& stream);
  bool LoadCompressed(std::istream& stream);
  void Swap();
  void FlipY();

  void CheckReadOutOfBounds(unsigned int pixel_idx);
  void ReadRLEHeader(std::istream& stream, unsigned char& rle_hdr);
  void ReadOnePixel(std::istream& stream, std::vector<unsigned char>& pixel);

}; // struct Image

namespace helpers {

  TgaFileHeader ReadHeader(std::istream& stream);
  TgaHelpHeader FillHelpHeader(const TgaFileHeader& fh);
	
} // namespace helpers

} // namespace gdm

#endif // AH_GDM_TGA_LOADER_H