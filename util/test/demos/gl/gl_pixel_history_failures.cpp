/******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2019-2023 Baldur Karlsson
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

#include <stdio.h>
#include "gl_test.h"

RD_TEST(GL_Pixel_History_Failures, OpenGLGraphicsTest)
{
  static constexpr const char *Description =
      "Test various failure cases of Pixel History (depth testing, stencil, scissor, shader discard, etc.).";

  std::string common = R"EOSHADER(

#version 420 core

#define v2f v2f_block \
{                     \
    vec4 pos;           \
    vec4 col;           \
    vec4 uv;            \
}

)EOSHADER";

  std::string vertex = R"EOSHADER(

layout(location = 0) in vec3 Position;
layout(location = 1) in vec4 Color;
layout(location = 2) in vec2 UV;

out v2f vertOut;

void main()
{
    vertOut.pos = vec4(Position.xyz, 1);
    gl_Position = vertOut.pos;
    vertOut.col = Color;
    vertOut.uv = vec4(UV.xy, 0, 1);
}

)EOSHADER";

  std::string pixel = R"EOSHADER(

in v2f vertIn;

layout(location = 0, index = 0) out vec4 Color;

void main()
{
    Color = vertIn.col;
}

)EOSHADER";

  int main()
  {
    // initialise, create window, create context, etc
    if(!Init())
      return 3;

    GLuint vao = MakeVAO();
    glBindVertexArray(vao);

    GLuint vb = MakeBuffer();
    glBindBuffer(GL_ARRAY_BUFFER, vb);
    const DefaultA2V FailureOfATriangle[3] = {
        {Vec3f(-0.5f, -0.5f, 0.5f), Vec4f(0.57721f, 0.27182f, 0.1385f, 1.0f), Vec2f(0.0f, 0.0f)},
        {Vec3f(0.0f, 0.5f, 0.5f), Vec4f(0.57721f, 0.27182f, 0.1385f, 1.0f), Vec2f(0.0f, 1.0f)},
        {Vec3f(0.5f, -0.5f, 0.5f), Vec4f(0.57721f, 0.27182f, 0.1385f, 1.0f), Vec2f(1.0f, 0.0f)},
    };
    glBufferStorage(GL_ARRAY_BUFFER, sizeof(FailureOfATriangle),
        FailureOfATriangle, 0);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DefaultA2V), (void *)(0));
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(DefaultA2V), (void *)(sizeof(Vec3f)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(DefaultA2V),
                          (void *)(sizeof(Vec3f) + sizeof(Vec4f)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    GLuint program = MakeProgram(common + vertex, common + pixel);

    glDepthFunc(GL_ALWAYS);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glStencilFunc(GL_ALWAYS, 0xcc, 0xff);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glEnable(GL_STENCIL_TEST);
    glStencilMask(0xff);

    while(Running())
    {
        GLenum bufs[] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, bufs);

        glBindVertexArray(vao);

        float col[] = {0.2f, 0.2f, 0.2f, 1.0f};
        glClearBufferfv(GL_COLOR, 0, col);
        glUseProgram(program);

        glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);

        glViewport(0, 0, GLsizei(screenWidth), GLsizei(screenHeight));

        // Test 1: depth test failure
        glDepthFunc(GL_LESS);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glClearBufferfi(GL_DEPTH, 0, 0.314f, 0);
        // Test 2: stencil test failure
        // Test 3: scissor test failure
        // Test 4: shader discard failure

        glDrawArrays(GL_TRIANGLES, 0, 3);

      Present();
    }

    return 0;
  }
};

REGISTER_TEST();
