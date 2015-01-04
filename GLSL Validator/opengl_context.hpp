#ifndef HG_OPENGL_CONTEXT_HPP
#define HG_OPENGL_CONTEXT_HPP

#if defined(__APPLE__) && defined(__MACH__)
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>
#endif

bool create_opengl_context();
bool destory_opengl_context();

#endif
