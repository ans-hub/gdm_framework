// *************************************************************
// File:    buffer_factory.h
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_BUFFER_FACT_H
#define AH_GDM_BUFFER_FACT_H

#include "data_factory.h"

#include "render/uniform_buffer.h"

namespace gdm {

using BufferHandle = Handle;

struct BufferFactory : public DataFactory<gfx::UniformBuffer*>
{

}; // struct BufferFactory

} // namespace gdm

#endif // AH_GDM_BUFFER_FACT_H
