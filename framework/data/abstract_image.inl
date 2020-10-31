// *************************************************************
// File:    abstract_image.inl
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

template <class T>
inline void gdm::helpers::ReadFromStream(std::istream& stream, T& t)
{
  stream.read(reinterpret_cast<char*>(&t), sizeof(T));
}
