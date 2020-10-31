// *************************************************************
// File:    tga_loader.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "tga_loader.h"

#include <ios>
#include <fstream>
#include <cassert>
#include <string>

namespace gdm {

//-- public

TgaImage::TgaImage(const char* fpath)
  : AbstractImage()
  , hdr_file_{}
  , hdr_help_{}
{
  std::ifstream stream (fpath, std::ios::in | std::ios::binary);
  if (!stream)
  {
    std::string msg = "Tga: open file error: " + std::string(fpath);
    throw ImgExcept(msg.c_str());
  }

  hdr_file_ = helpers::ReadHeader(stream);
  hdr_help_ = helpers::FillHelpHeader(hdr_file_);

  if ((hdr_file_.width <= 0) || (hdr_file_.height <= 0))
  {
    std::string msg = "Tga: zero width or height: " +  std::to_string(hdr_file_.width) + std::to_string(hdr_file_.height);
    throw ImgExcept(msg.c_str());
  }
  if ((hdr_file_.depth != 24) && (hdr_file_.depth != 32))	
  {
    std::string msg = "Tga: depth is differ from 24 or 32: " + std::to_string(hdr_file_.depth);
    throw ImgExcept(msg.c_str());
  }
  if (hdr_file_.image_type == 2)
    LoadUncompressed(stream);
  else if (hdr_file_.image_type == 10)
    LoadCompressed(stream);
  else
  {
    std::string msg = "Tga: unsupported tga pixel format: " + std::to_string(hdr_file_.image_type) + 
                      ", supported direct color (24 && 32), not mapper colors";
    throw ImgExcept(msg.c_str());
  }
  assert(data_.size() == hdr_help_.uncompressed_sz);
  Swap();

  width_ = hdr_file_.width;
  height_ = hdr_file_.height;
  depth_ = hdr_file_.depth;
}

// --private

TgaFileHeader helpers::ReadHeader(std::istream& stream)
{
  TgaFileHeader fh {};
  helpers::ReadFromStream(stream, fh.id_length);
  helpers::ReadFromStream(stream, fh.color_map_type);
  helpers::ReadFromStream(stream, fh.image_type);
  helpers::ReadFromStream(stream, fh.color_map_origin);
  helpers::ReadFromStream(stream, fh.color_map_length);
  helpers::ReadFromStream(stream, fh.color_map_entry_size);
  helpers::ReadFromStream(stream, fh.x_origin);
  helpers::ReadFromStream(stream, fh.y_origin);
  helpers::ReadFromStream(stream, fh.width);
  helpers::ReadFromStream(stream, fh.height);
  helpers::ReadFromStream(stream, fh.depth);
  helpers::ReadFromStream(stream, fh.image_descriptor);
  return fh;
}

TgaHelpHeader helpers::FillHelpHeader(const TgaFileHeader& fh)
{
  TgaHelpHeader hh {};
  hh.bpp = fh.depth / 8;
  hh.uncompressed_sz = hh.bpp * fh.width * fh.height;
  hh.pixels_cnt = fh.width * fh.height;
  return hh;
}

bool TgaImage::LoadUncompressed(std::istream& stream)
{
  assert(hdr_help_.uncompressed_sz != 0);
  data_.resize(hdr_help_.uncompressed_sz);
  stream.read((char*)data_.data(), data_.size());
  return true;
}

bool TgaImage::LoadCompressed(std::istream& stream)
{
  unsigned int pixls_been_read = 0;
  std::vector<unsigned char> curr_pix (hdr_help_.bpp);
  data_.reserve(hdr_help_.uncompressed_sz);

  assert(hdr_help_.uncompressed_sz != 0);

  unsigned char rle_hdr {};
  do
	{
    ReadRLEHeader(stream, rle_hdr);
	  bool rle = (rle_hdr >> 7) & 1U;  // does have ms bit
    if (rle == false) // < 128
		{
		  rle_hdr++;
      pixls_been_read += rle_hdr;
      if (pixls_been_read > hdr_help_.pixels_cnt)
			  return false;
		  for (int i = 0; i < rle_hdr; i++)
			{
    	  stream.read((char*)curr_pix.data(), curr_pix.size());
        data_.insert(data_.end(), curr_pix.begin(), curr_pix.end());
			}
		}
	  else
		{
      rle_hdr -= 127; // clear ms bit
      pixls_been_read += rle_hdr;
      if (pixls_been_read > hdr_help_.pixels_cnt)
			  return false;
  	  stream.read((char*)curr_pix.data(), curr_pix.size());
		  for(int i = 0; i < rle_hdr; i++)
        data_.insert(data_.end(), curr_pix.begin(), curr_pix.end());
		}
  } while(pixls_been_read < hdr_help_.pixels_cnt);

  return true;
}

void TgaImage::Swap()
{
  for (std::size_t i = 0; i < data_.size(); i += hdr_help_.bpp)
    std::swap(data_[i], data_[i+2]);
}

// Todo: make correct algorithm, add option to ctor

void TgaImage::FlipY()
{
  // for (std::size_t i = 0; i < data_.size(); i += hdr_help_.bpp)
  //   for (std::size_t k = 0; k < hdr_help_.bpp / sizeof(unsigned char); ++k)
      // std::swap(data_[i+k], data_[hdr_help_.pixels_cnt-i+k]);
}

void TgaImage::ReadRLEHeader(std::istream& stream, unsigned char& rle_hdr)
{
  if (stream.read((char*)&rle_hdr, 1))
    return;
  {
    std::string msg = "Tga: RLE header is missed: " + std::to_string(rle_hdr);
    throw ImgExcept(msg.c_str());
  }
}

void TgaImage::ReadOnePixel(std::istream& stream, std::vector<unsigned char>& pixel)
{
  if (stream.read((char*)pixel.data(), pixel.size()))
    return;
  else
  {
    std::string msg = "Tga: can't read image data";
    throw ImgExcept(msg.c_str());
  }
}

void TgaImage::CheckReadOutOfBounds(unsigned int curr_pixel)
{
  if (curr_pixel <= hdr_help_.pixels_cnt)
    return;
}

} // namespace tga
