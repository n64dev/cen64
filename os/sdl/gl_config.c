#include "common.h"
#include "gl_common.h"
#include "gl_config.h"
#include "gl_hints.h"

#define SET_ATTRIBUTE_IF_VALID(ATTR, VALUE_TO_TEST) \
  if ((VALUE_TO_TEST) != -1)                        \
  {                                                 \
    SDL_GL_SetAttribute((ATTR), (VALUE_TO_TEST));   \
  }

#define SET_ATTRIBUTE_STATIC_IF_VALID(ATTR, VALUE_TO_TEST) \
  if ((VALUE_TO_TEST) != -1)                               \
  {                                                        \
    SDL_GL_SetAttribute((ATTR), 1);                        \
  }

cen64_gl_config *cen64_gl_config_create(cen64_gl_display display,
                                        cen64_gl_screen screen, const cen64_gl_hints *hints, int *matching)
{
  int idx = 0;

  SET_ATTRIBUTE_STATIC_IF_VALID(SDL_GL_DOUBLEBUFFER, hints->double_buffered);
  SET_ATTRIBUTE_STATIC_IF_VALID(SDL_GL_STEREO, hints->stereoscopic);
  if (hints->rgb_color_depth != -1)
  {
    int component_depth = hints->rgb_color_depth > 0
                              ? hints->rgb_color_depth / 3
                              : 0;
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, component_depth);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, component_depth);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, component_depth);
  }
  SET_ATTRIBUTE_IF_VALID(SDL_GL_ALPHA_SIZE, hints->alpha_color_depth);

  SET_ATTRIBUTE_IF_VALID(SDL_GL_DEPTH_SIZE, hints->depth_buffer_size);

  SET_ATTRIBUTE_IF_VALID(SDL_GL_STENCIL_SIZE, hints->stencil_buffer_size);

  SET_ATTRIBUTE_IF_VALID(SDL_GL_ACCUM_RED_SIZE, hints->accum_buffer_red_bits);
  SET_ATTRIBUTE_IF_VALID(SDL_GL_ACCUM_GREEN_SIZE, hints->accum_buffer_green_bits);
  SET_ATTRIBUTE_IF_VALID(SDL_GL_ACCUM_BLUE_SIZE, hints->accum_buffer_blue_bits);
  SET_ATTRIBUTE_IF_VALID(SDL_GL_ACCUM_ALPHA_SIZE, hints->accum_buffer_alpha_bits);
}