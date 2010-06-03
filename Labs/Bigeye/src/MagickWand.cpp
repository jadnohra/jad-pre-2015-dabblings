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
			PixelWand* black_wand = NewPixelWand();
			PixelWand* col1_wand = NewPixelWand();
			PixelWand* transparent_wand = NewPixelWand();
			PixelSetColor(white_wand, "white");
			PixelSetColor(red_wand, "red");
			PixelSetColor(blue_wand, "blue");
			PixelSetColor(black_wand, "black");
			PixelSetColor(col1_wand, "#4096EE");
			PixelSetColor(transparent_wand, "transparent");
			DrawingWand* drawing_wand = NewDrawingWand();
			
			//$im->newImage( 200, 200, "white", "png" );
			MagickNewImage(test_wand, 200, 200, white_wand);

			//$draw->setFillColor( "#4096EE" );
			DrawSetFillColor(drawing_wand, col1_wand);

			//??
			//DrawSetStrokeColor(drawing_wand, blue_wand);
			
			//$draw->rectangle( 0, 0, 170, 40 );
			DrawRectangle(drawing_wand, 0,0,170,40);

			//$draw->setFillColor( "white" );
			DrawSetFillColor(drawing_wand, white_wand);

			//$draw->setFillAlpha( 0.5 );
			DrawSetFillAlpha(drawing_wand, 0.5);

			//$draw->setFont( "./Vera.ttf" );
			DrawSetFont(drawing_wand, "media/DroidSans.ttf");

			//$draw->setFillAlpha( 0.17 );
			DrawSetFillAlpha(drawing_wand, 0.17);

			//$draw->bezier( array(
            //   array( "x" => 0 , "y" => 0 ),
            //    array( "x" => 85, "y" => 24 ),
            //    array( "x" => 170, "y" => 0 ),
            //   ) );
			PointInfo bezier_points[3];
			bezier_points[0].x = 0; bezier_points[0].y = 0;
			bezier_points[1].x = 85; bezier_points[1].y = 24;
			bezier_points[2].x = 170; bezier_points[2].y = 0;
			DrawBezier(drawing_wand, 3, bezier_points);

			/* Render all pending operations on the image */
			MagickDrawImage(test_wand, drawing_wand);

			//$draw->setFillAlpha( 1 );
			DrawSetFillAlpha(drawing_wand, 1.0);

			//$draw->setFontSize( 25 );
			DrawSetFontSize(drawing_wand, 25.0);

			//$draw->setFillColor( "white" );
			DrawSetFillColor(drawing_wand, white_wand);

			//$im->annotateImage( $draw, 38, 28, 0, "Submit" );
			MagickAnnotateImage(test_wand, drawing_wand, 32, 28, 0, "Bigeye ! ;)");

			//$im->trimImage( 0 );
			MagickTrimImage(test_wand, 0);

			//$im->roundCorners( 4, 4 );
			// MagickRoundCorners( ??
			//
				DrawingWand* round_drawing_wand = NewDrawingWand();
				MagickWand* round_wand = NewMagickWand();
				MagickNewImage(round_wand, MagickGetImageWidth(test_wand),MagickGetImageHeight(test_wand), transparent_wand);
				
				DrawSetFillColor(round_drawing_wand, white_wand);
				DrawRoundRectangle(round_drawing_wand, 2,2, MagickGetImageWidth(round_wand)-2,MagickGetImageHeight(round_wand)-2, 4, 4);

				MagickDrawImage(round_wand, round_drawing_wand);
				MagickCompositeImage(test_wand, round_wand, CopyOpacityCompositeOp, 0, 0);
			//

			//$shadow = $im->clone();
			MagickWand* shadow_wand = CloneMagickWand(test_wand);

			//$shadow->setImageBackgroundColor( new ImagickPixel('black') );
			MagickSetImageBackgroundColor(shadow_wand, black_wand);

			//$shadow->shadowImage( 40, 1, 1, 1 );
			MagickShadowImage(shadow_wand, 40, 1, 1, 1);

			//$shadow->compositeImage( $im, Imagick::COMPOSITE_OVER, 0, 0 );
			MagickCompositeImage(shadow_wand, test_wand, OverCompositeOp, 0, 0);

			//MagickDrawImage(test_wand, drawing_wand);

			MagickWand* copy_wand = shadow_wand;

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

			ClearMagickWand(shadow_wand);
			ClearMagickWand(test_wand);
			ClearDrawingWand(drawing_wand);
			ClearPixelWand(white_wand);
			ClearPixelWand(red_wand);
			ClearPixelWand(blue_wand);
			ClearPixelWand(col1_wand);
			ClearPixelWand(black_wand);
		}
	}
	ClearMagickWand(wand);
	
	return was_read;
}


}