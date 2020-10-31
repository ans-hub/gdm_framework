// *************************************************************
// File:    bmp_loader.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "bmp_loader.h"

#include <ios>
#include <fstream>
#include <cassert>
#include <string>

// --public 

gdm::BmpImage::BmpImage(const char* fpath)
  : AbstractImage()
  , hdr_file_{}
  , hdr_info_{}
{
  std::ifstream stream (fpath, std::ios::in | std::ios::binary);
  if (!stream)
  {
    std::string msg = "Bmp: file not found: " + std::string(fpath);
    throw ImgExcept(msg.c_str());
  }

  hdr_file_ = helpers::ReadFileHeader(stream);
  hdr_info_ = helpers::ReadInfoHeader(stream);

  constexpr int k_bmp_type_id = 19778;

  if (hdr_file_.type != k_bmp_type_id)
  {
    std::string msg = "Bmp: invalid type value: " + std::to_string(hdr_file_.type);
    throw ImgExcept(msg.c_str());
  }
  if (hdr_info_.color_depth != 24 && hdr_info_.color_depth != 32)
  {
    std::string msg = "Bmp: invalid depth: " + std::to_string(hdr_info_.color_depth);
    throw ImgExcept(msg.c_str());
  }

  FixMisformattedFile(hdr_file_, hdr_info_);
  width_ = hdr_info_.width;
  height_ = hdr_info_.height;
  depth_ = hdr_info_.color_depth;

  data_.resize(hdr_info_.image_size);
  stream.read((char*)data_.data(), hdr_info_.image_size);
  
  Swap();
}

// --private

void gdm::BmpImage::FixMisformattedFile(BmpFileHeader& fh, BmpInfoHeader& ih)
{
  constexpr int k_color_components_cnt = 3;
  constexpr int k_bmp_header_len = 54;
 
  if (ih.image_size == 0)
    ih.image_size = ih.width * ih.height * k_color_components_cnt;
  if (fh.data_offset == 0)
    fh.data_offset = k_bmp_header_len;
}

void gdm::BmpImage::Swap()
{
  int step = hdr_info_.color_depth / 8;
  assert(step == 3 || step ==4);
  assert(data_.size() % step == 0);
  for(std::size_t i = 0; i < data_.size(); i += step)
    std::swap(data_[i], data_[i+2]);
}

// --helpers

gdm::BmpFileHeader gdm::helpers::ReadFileHeader(std::istream& stream)
{
  BmpFileHeader fh {};
  helpers::ReadFromStream(stream, fh.type);
  helpers::ReadFromStream(stream, fh.file_size);
  helpers::ReadFromStream(stream, fh.reserved_1);
  helpers::ReadFromStream(stream, fh.reserved_2);
  helpers::ReadFromStream(stream, fh.data_offset);
  return fh;
}

gdm::BmpInfoHeader gdm::helpers::ReadInfoHeader(std::istream& stream)
{
  BmpInfoHeader ih {};
  helpers::ReadFromStream(stream, ih.header_size);
  helpers::ReadFromStream(stream, ih.width);
  helpers::ReadFromStream(stream, ih.height);
  helpers::ReadFromStream(stream, ih.planes);
  helpers::ReadFromStream(stream, ih.color_depth);
  helpers::ReadFromStream(stream, ih.compression_method);
  helpers::ReadFromStream(stream, ih.image_size);
  helpers::ReadFromStream(stream, ih.hor_pixels_per_meter);
  helpers::ReadFromStream(stream, ih.ver_pixels_per_meter);
  helpers::ReadFromStream(stream, ih.colors_in_palette);
  helpers::ReadFromStream(stream, ih.colors_important);
  return ih;
}
