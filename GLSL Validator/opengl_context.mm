#include "opengl_context.hpp"

#include <cassert>

#if defined(__APPLE__) && defined(__MACH__)

#import <OpenGL/OpenGL.h>

namespace
{
    CGLContextObj Context;
}

bool create_opengl_context()
{
    CGLPixelFormatObj PixelFormat;
    CGLPixelFormatAttribute PixelFormatAttributes[] =
    {
        kCGLPFARendererID, (CGLPixelFormatAttribute)kCGLRendererGenericFloatID,
        kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute)kCGLOGLPVersion_3_2_Core,
        kCGLPFASupportsAutomaticGraphicsSwitching,
        (CGLPixelFormatAttribute)NULL
    };
    GLint VirtualScreenCount = 0;
    CGLChoosePixelFormat(PixelFormatAttributes, &PixelFormat, &VirtualScreenCount);
    assert(PixelFormat != NULL);
    auto Error = CGLCreateContext(PixelFormat, NULL, &Context);
    assert(Error == kCGLNoError);
    CGLDestroyPixelFormat(PixelFormat);
    CGLSetCurrentContext(Context);
    assert(glGetError() == GL_NO_ERROR);
    return true;
}

bool destory_opengl_context()
{
    CGLSetCurrentContext(NULL);
    CGLDestroyContext(Context);
    return true;
}

#endif
