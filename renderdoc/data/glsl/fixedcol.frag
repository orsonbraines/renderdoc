/******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2020-2021 Baldur Karlsson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ******************************************************************************/

#include "glsl_globals.h"

#if defined(OPENGL_ES) && __VERSION__ == 100

// GLES shading language 1.0 must use gl_FragColor
#define color_out gl_FragColor

#else

// otherwise we use a proper output
IO_LOCATION(0) out vec4 color_out;

#endif

#ifndef VULKAN    // Vulkan uses SPIR-V patching

// if we're compiling for GL SPIR-V, give the uniform an explicit location
#ifdef GL_SPIRV
layout(location = 99)
#endif

    uniform vec4 RENDERDOC_Fixed_Color;

#endif

void main(void)
{
#ifdef VULKAN
  // used to have a shader-replacement pixel shader
  // that outputs a fixed colour, without needing a
  // slot in a descriptor set. We re-write the SPIR-V
  // on the fly to replace these constants
  color_out = vec4(1.1f, 2.2f, 3.3f, 4.4f);
#else
  color_out = RENDERDOC_Fixed_Color;
#endif
}
