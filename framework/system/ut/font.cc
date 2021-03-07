// *************************************************************
// File:    font.cc
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "3rdparty/catch/catch.hpp"

#include "system/font.h"

namespace gdm::test {

TEST_CASE("Font loading")
{
	Font arial("fonts/arial.ttf", 14);
}

} // namespace gdm::test
