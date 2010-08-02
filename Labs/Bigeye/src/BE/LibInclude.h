#ifndef _INCLUDED_BIGEYE_LIBINCLUDE_H
#define _INCLUDED_BIGEYE_LIBINCLUDE_H


//////////////////////////////////////////
// OGL.h
/////////////////////////////////////////

// TODO!!!  (http://glew.sourceforge.net/install.html)
// otherwise glew32.dll is needed
//#define GLEW_STATIC 
#include "GL/glew.h"
#ifdef GLEW_STATIC 
	#pragma comment (lib, "glew32s.lib")	
#else
	#pragma comment (lib, "glew32.lib")	
#endif

#ifdef WIN32
	#include "Platform.h"
	#define FREEGLUT_STATIC
	#include "freeglut/GL/freeglut.h"
	#include "freeglut/GL/freeglut_ext.h"
#endif


//////////////////////////////////////////
// MagickWand.h
/////////////////////////////////////////

#ifdef _DEBUG
	#pragma comment (lib, "CORE_DB_bzlib_.lib")	
	#pragma comment (lib, "CORE_DB_coders_.lib")
	#pragma comment (lib, "CORE_DB_filters_.lib")
	#pragma comment (lib, "CORE_DB_jbig_.lib")
	#pragma comment (lib, "CORE_DB_jp2_.lib")
	#pragma comment (lib, "CORE_DB_jpeg_.lib")
	#pragma comment (lib, "CORE_RL_lcms_.lib")
	#pragma comment (lib, "CORE_DB_libxml_.lib")
	#pragma comment (lib, "CORE_DB_magick_.lib")
	#pragma comment (lib, "CORE_DB_png_.lib")
	#pragma comment (lib, "CORE_DB_tiff_.lib")
	#pragma comment (lib, "CORE_DB_ttf_.lib")
	#pragma comment (lib, "CORE_DB_wand_.lib")
	#pragma comment (lib, "CORE_DB_wmf_.lib")
	#pragma comment (lib, "CORE_DB_xlib_.lib")
	#pragma comment (lib, "CORE_DB_zlib_.lib")
	#pragma comment (lib, "ws2_32.lib")
#else
	#pragma comment (lib, "CORE_RL_bzlib_.lib")	
	#pragma comment (lib, "CORE_RL_coders_.lib")
	#pragma comment (lib, "CORE_RL_filters_.lib")
	#pragma comment (lib, "CORE_RL_jbig_.lib")
	#pragma comment (lib, "CORE_RL_jp2_.lib")
	#pragma comment (lib, "CORE_RL_jpeg_.lib")
	#pragma comment (lib, "CORE_RL_lcms_.lib")
	#pragma comment (lib, "CORE_RL_libxml_.lib")
	#pragma comment (lib, "CORE_RL_magick_.lib")
	#pragma comment (lib, "CORE_RL_png_.lib")
	#pragma comment (lib, "CORE_RL_tiff_.lib")
	#pragma comment (lib, "CORE_RL_ttf_.lib")
	#pragma comment (lib, "CORE_RL_wand_.lib")
	#pragma comment (lib, "CORE_RL_wmf_.lib")
	#pragma comment (lib, "CORE_RL_xlib_.lib")
	#pragma comment (lib, "CORE_RL_zlib_.lib")
	#pragma comment (lib, "ws2_32.lib")
#endif


#endif 