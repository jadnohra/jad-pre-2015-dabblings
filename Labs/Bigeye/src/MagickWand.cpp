#include "MagickWand.h"
#include <stdio.h>

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

#define NeedFunctionPrototypes
#define _MAGICKLIB_
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_WARNINGS
#include "ImageMagickWand/include/wand/MagickWand.h"

namespace BE
{

class MagickWandContext
{
public:

	MagickWandContext()
	{
		temp_pixels = NULL;
		temp_total_component_count = 0;

		MagickWandGenesis();
	}

	~MagickWandContext()
	{
		MagickWandTerminus();
		free(temp_pixels);
	}

	unsigned char* GetTempPixels(size_t inCount, size_t inComponentCount)
	{
		size_t total_component_count = inCount * inComponentCount;

		if (total_component_count > temp_total_component_count)
		{
			temp_pixels = (unsigned char*) realloc(temp_pixels, total_component_count * sizeof(unsigned char));
			temp_total_component_count = total_component_count;
		}

		return temp_pixels;
	}

	unsigned char* GetTempPixels() { return temp_pixels; }

	unsigned char* temp_pixels;
	size_t temp_total_component_count;
};

static MagickWandContext sMagickWandContext;

bool MagicWand::ReadImageToGLTexture(const char* inPath, GLuint& outTexture, GLsizei& outWidth, GLsizei& outHeight)
{
	bool was_read = false;

	MagickWand* wand = NewMagickWand();
	{
		if (MagickReadImage(wand, inPath))
		{
			MagickWand* test_wand = NewMagickWand();
			PixelWand* white_wand = NewPixelWand();
			PixelWand* red_wand = NewPixelWand();
			PixelWand* blue_wand = NewPixelWand();
			PixelSetColor(white_wand, "white");
			PixelSetColor(red_wand, "red");
			PixelSetColor(blue_wand, "blue");
			DrawingWand* drawing_wand = NewDrawingWand();
			MagickNewImage(test_wand, 200, 200, white_wand);
			DrawSetFillColor(drawing_wand, red_wand);
			DrawSetStrokeColor(drawing_wand, blue_wand);
			DrawRectangle(drawing_wand, 20,20,100,100);

			MagickDrawImage(test_wand, drawing_wand);

			MagickWand* copy_wand = test_wand;

			outWidth = MagickGetImageWidth(copy_wand);
			outHeight = MagickGetImageHeight(copy_wand);

			if (MagickExportImagePixels(copy_wand, 0, 0, outWidth, outHeight, "RGBA", CharPixel, sMagickWandContext.GetTempPixels(outWidth*outHeight, 4)))
			{
				glGenTextures(1, &outTexture);
				glBindTexture(GL_TEXTURE_2D, outTexture);
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, outWidth, outHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, sMagickWandContext.GetTempPixels());
				was_read = true;
			}

			ClearMagickWand(test_wand);
			ClearDrawingWand(drawing_wand);
			ClearPixelWand(white_wand);
			ClearPixelWand(red_wand);
			ClearPixelWand(blue_wand);
		}
	}
	ClearMagickWand(wand);
	
	return was_read;
}


}