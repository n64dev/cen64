# Finds OpenGL for XQuartz on OSX. Sets the same flags as the default
# FindOpenGL.cmake module. Necessary because the default module will
# only find the system OpenGL framework, which does not work with
# XQuartz.

find_library(OPENGL_gl_LIBRARY
  NAMES GL
  PATHS /opt/X11/lib
  DOC "OpenGL lib for XQuartz on OSX"
)
find_library(OPENGL_glu_LIBRARY
  NAMES GLU
  PATHS /opt/X11/lib
  DOC "GLU lib for XQuartz on OSX"
)
find_path(OPENGL_INCLUDE_DIR
  NAMES GL/gl.h
  PATHS /opt/X11/include
  DOC "Include for OpenGL for XQuartz on OSX"
)

if(OPENGL_gl_LIBRARY)
	set(OPENGL_FOUND TRUE)
endif(OPENGL_gl_LIBRARY)

if(OPENGL_glu_LIBRARY)
	set(OPENGL_GLU_FOUND TRUE)
endif(OPENGL_glu_LIBRARY)

set(OPENGL_LIBRARIES ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY})
set(_OpenGL_REQUIRED_VARS OPENGL_gl_LIBRARY OPENGL_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenGLXQuartz REQUIRED_VARS ${_OpenGL_REQUIRED_VARS})

mark_as_advanced(
  OPENGL_INCLUDE_DIR
  OPENGL_glu_LIBRARY
  OPENGL_gl_LIBRARY
)
