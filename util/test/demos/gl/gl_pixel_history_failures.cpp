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

    GLuint ssFbo = MakeFBO();
    GLuint msFbo = MakeFBO();
    glBindFramebuffer(GL_FRAMEBUFFER, ssFbo);
    GLuint ssColour = MakeTexture();
    glBindTexture(GL_TEXTURE_2D, ssColour);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, screenWidth, screenHeight);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssColour, 0);
    GLuint ssDepthStencil = MakeTexture();
    glBindTexture(GL_TEXTURE_2D, ssDepthStencil);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
                           ssDepthStencil, 0);


    glBindFramebuffer(GL_FRAMEBUFFER, msFbo);
    GLuint multisampledTexture = MakeTexture();
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, multisampledTexture);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 2, GL_RGBA32F, screenWidth, screenHeight, GL_TRUE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D_MULTISAMPLE, multisampledTexture, 0);

    GLuint program = MakeProgram(common + vertex, common + pixel);

    int err = glGetError();
    printf("laksdjfsdjfljasd before loop btw %d\n", err);

    while(Running())
    {
        glBindFramebuffer(GL_FRAMEBUFFER, ssFbo);
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
        glClearDepth(0.31415926);
        glClear(GL_DEPTH_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Test 2: stencil test failure
        glDisable(GL_DEPTH_TEST);
        glStencilFunc(GL_EQUAL, 0x1, 0xff);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glEnable(GL_STENCIL_TEST);
        glStencilMask(0xff);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Test 3: scissor test failure
        glDisable(GL_STENCIL_TEST);
        glEnable(GL_SCISSOR_TEST);
        glScissor(1, 1, 1, 1);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Test 4: face culling failure
        glDisable(GL_SCISSOR_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Test 5: sample mask failure
        glBindFramebuffer(GL_FRAMEBUFFER, msFbo);
        glDisable(GL_CULL_FACE);
        glEnable(GL_SAMPLE_MASK);
        glSampleMaski(0, 2);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glDisable(GL_SAMPLE_MASK);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
      Present();
    }

    return 0;
  }
};

REGISTER_TEST();
