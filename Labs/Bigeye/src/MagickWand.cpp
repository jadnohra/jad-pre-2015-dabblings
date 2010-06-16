#include "MagickWand.h"
#include <stdio.h>
#include <vector>

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

//useful links:
// http://members.shaw.ca/el.supremo/MagickWand/
// http://valokuva.org/?paged=2


template<>
struct PtrDeletePolicy_delete<MagickWand> {

    static inline MagickWand* getDefault() { return NULL; }
    static inline void doDelete(MagickWand* ptr) { ClearMagickWand(ptr); }
};


template<>
struct PtrDeletePolicy_delete<PixelWand> {

    static inline PixelWand* getDefault() { return NULL; }
    static inline void doDelete(PixelWand* ptr) { ClearPixelWand(ptr); }
};

template<>
struct PtrDeletePolicy_delete<DrawingWand> {

    static inline DrawingWand* getDefault() { return NULL; }
    static inline void doDelete(DrawingWand* ptr) { ClearDrawingWand(ptr); }
};

template<>
struct PtrDeletePolicy_delete<PixelIterator> {

    static inline PixelIterator* getDefault() { return NULL; }
    static inline void doDelete(PixelIterator* ptr) { ClearPixelIterator(ptr); }
};

class MagickWandImpl
{
public:

	struct AutoGenesis
	{
		AutoGenesis()
		{
			MagickWandGenesis();
		}
	};

	struct GradientCurve
	{
		float mExp;
		float mFrom;
		float mTo;
		float mInvLength;

		GradientCurve()
		:	mExp(1.0f)
		,	mFrom(0.0f)
		,	mTo(1.0f)
		,	mInvLength(1.0f)
		{
		}

		GradientCurve(float inFrom, float inTo, float inExp)
		{
			mExp = inExp;
			mFrom = inFrom;
			mTo = inTo;
			mInvLength = 1.0f / (mTo-mFrom);
		}

		float AdjustAndMap(float inValue) const
		{
			return (Map((inValue - mFrom) * mInvLength));
		}

		float Map(float inValue) const
		{
			return powf(inValue, mExp);
		}
	};

	class GradientCurves
	{
	public:

		int StartMap() const
		{
			return 0;
		}

		float Map(int& ioIndex, float inValue) const
		{
			if (ioIndex >= (int) mCurves.size())
				return inValue;

			if (mCurves[ioIndex].mTo >= inValue)
				return mCurves[ioIndex].AdjustAndMap(inValue);

			++ioIndex;
			return Map(ioIndex, inValue);
		}

		typedef std::vector<GradientCurve> Curves;
		Curves mCurves;
	};
	

	unsigned char* temp_pixels;
	size_t temp_total_component_count;

	AutoGenesis auto_genesis;

	auto_scoped_ptr<PixelWand> white;
	auto_scoped_ptr<PixelWand> red;
	auto_scoped_ptr<PixelWand> blue;
	auto_scoped_ptr<PixelWand> black;
	auto_scoped_ptr<PixelWand> black_mask;
	auto_scoped_ptr<PixelWand> blue1;
	auto_scoped_ptr<PixelWand> transparent;

	typedef std::vector<DrawingWand*> FontWands;
	FontWands mFontWands;

	MagickWandImpl()
	:	temp_pixels(NULL)
	,	temp_total_component_count(0)
	,	auto_genesis()
	,	white(NewPixelWand())
	,	red(NewPixelWand())
	,	blue(NewPixelWand())
	,	black(NewPixelWand())
	,	black_mask(NewPixelWand())
	,	blue1(NewPixelWand())
	,	transparent(NewPixelWand())
	{
		PixelSetColor(white, "white");
		PixelSetColor(red, "red");
		PixelSetColor(blue, "blue");
		PixelSetColor(black, "black");
		PixelSetColor(black_mask, "black");
		PixelSetAlpha(black_mask, 0.0);
		PixelSetColor(blue1, "#4096EE");
		PixelSetColor(transparent, "transparent");
	}

	~MagickWandImpl()
	{
		for (size_t i=0; i<mFontWands.size(); ++i)
			ClearDrawingWand(mFontWands[i]);

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


	bool ToGLTexture(DrawingWand* draw, GLuint inTexture, GLsizei inWidth, GLsizei inHeight)
	{
		auto_scoped_ptr<MagickWand> wand(NewMagickWand());
		MagickNewImage(wand, inWidth, inHeight, transparent);
		MagickDrawImage(wand, draw);

		return ToGLTexture(wand, inTexture, inWidth, inHeight);
	}


	bool ToGLTexture(MagickWand* wand, GLuint inTexture, GLsizei& outWidth, GLsizei& outHeight)
	{
		if (wand)
		{
			outWidth = MagickGetImageWidth(wand);
			outHeight = MagickGetImageHeight(wand);

			if (MagickExportImagePixels(wand, 0, 0, outWidth, outHeight, "RGBA", CharPixel, GetTempPixels(outWidth*outHeight, 4)))
			{
				glBindTexture(GL_TEXTURE_2D, inTexture);
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, outWidth, outHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, GetTempPixels());
				return true;
			}
		}

		return false;
	}

	void GetRoundingWandSpace(int inRounding, bool inStroke, int& outAdditionalHorizSpace, int& outAdditionalVertSpace)
	{
		outAdditionalHorizSpace = inRounding;
		outAdditionalVertSpace = inRounding;

		if (inStroke)
		{
			outAdditionalHorizSpace += 1.0f;
			outAdditionalVertSpace += 1.0f;
		}
	}

	// modifies source wand
	void RoundWand(MagickWand* ioSourceWand, int inRounding, bool inStroke)
	{
		//return;

		auto_scoped_ptr<DrawingWand> round_drawing_wand(NewDrawingWand());
		MagickWand* round_wand = NewMagickWand();
		MagickNewImage(round_wand, MagickGetImageWidth(ioSourceWand), MagickGetImageHeight(ioSourceWand), transparent);
		
		DrawSetFillColor(round_drawing_wand, white);
		if (inStroke)
		{
			DrawSetStrokeWidth(round_drawing_wand, 1.0f);
			DrawSetStrokeColor(round_drawing_wand, black);
		}
		//DrawRoundRectangle(round_drawing_wand, inRounding/2, inRounding/2, MagickGetImageWidth(round_wand)-inRounding/2,MagickGetImageHeight(round_wand)-inRounding/2, inRounding, inRounding);
		DrawRoundRectangle(round_drawing_wand, 0.0, 0.0, MagickGetImageWidth(round_wand)-1,MagickGetImageHeight(round_wand)-1, inRounding, inRounding);
		//DrawRoundRectangle(round_drawing_wand, 1.0, 1.0, MagickGetImageWidth(round_wand)-2,MagickGetImageHeight(round_wand)-2, inRounding, inRounding);
		//DrawRoundRectangle(round_drawing_wand, 0.0, 0.0, MagickGetImageWidth(round_wand),MagickGetImageHeight(round_wand), inRounding, inRounding);
		//DrawRoundRectangle(round_drawing_wand, 0.0, 0.0, MagickGetImageWidth(round_wand)-inRounding/2,MagickGetImageHeight(round_wand)-inRounding/2, inRounding, inRounding);

		MagickDrawImage(round_wand, round_drawing_wand);
		MagickCompositeImage(ioSourceWand, round_wand, DstInCompositeOp, 0, 0);
		//MagickCompositeImage(ioSourceWand, round_wand, ReplaceCompositeOp, 0, 0);
		
		if (inStroke)
		{
			auto_scoped_ptr<MagickWand> stroke_round_wand(NewMagickWand());
			MagickNewImage(stroke_round_wand, MagickGetImageWidth(ioSourceWand)+2, MagickGetImageHeight(ioSourceWand)+2, transparent);
			DrawSetFillColor(round_drawing_wand, transparent);
			DrawSetStrokeColor(round_drawing_wand, black);
			DrawRoundRectangle(round_drawing_wand, 0.0, 0.0, MagickGetImageWidth(round_wand)-1,MagickGetImageHeight(round_wand)-1, inRounding, inRounding);
			MagickDrawImage(stroke_round_wand, round_drawing_wand);
			
			MagickCompositeImage(ioSourceWand, stroke_round_wand, MultiplyCompositeOp, 0, 0);
			//MagickCompositeImage(ioSourceWand, stroke_round_wand, ReplaceCompositeOp, 0, 0);
		}
	}

	MagickWand* DrawToWand(DrawingWand* draw, GLsizei inWidth, GLsizei inHeight, PixelWand* inColor)
	{
		MagickWand* wand(NewMagickWand());
		MagickNewImage(wand, inWidth, inHeight, inColor);
		MagickDrawImage(wand, draw);

		return wand;
	}

	// Returns result
	MagickWand* ShadowDrawWand(DrawingWand* draw, GLsizei inWidth, GLsizei inHeight, int inOffsetX=1, int inOffsetY=1, float inOpacity=40.0f, float inSigma=1.0f)
	{
		auto_scoped_ptr<MagickWand> wand(DrawToWand(draw, inWidth, inHeight, transparent));
		return ShadowWand(wand, inOffsetX, inOffsetY, inOpacity, inSigma);
	}

	// Returns result
	MagickWand* ShadowWand(MagickWand* inSourceWand, int inOffsetX=1, int inOffsetY=1, float inOpacity=40.0f, float inSigma=1.0f)
	{
		auto_scoped_ptr<MagickWand> shadow_wand(CloneMagickWand(inSourceWand));

		//$shadow->setImageBackgroundColor( new ImagickPixel('black') );
		MagickSetImageBackgroundColor(shadow_wand, black);

		//$shadow->shadowImage( 40, 1, 1, 1 );
		MagickShadowImage(shadow_wand, inOpacity, inSigma, 0, 0);

		//$shadow->compositeImage( $im, Imagick::COMPOSITE_OVER, 0, 0 );
		//MagickCompositeImage(shadow_wand, inSourceWand, OverCompositeOp, inOffsetX, inOffsetY);

		MagickWand* composite_wand = NewMagickWand();
		int min_x = std::min(inOffsetX, 0);
		int max_x = std::max(MagickGetImageWidth(inSourceWand), inOffsetX+MagickGetImageWidth(shadow_wand));
		int min_y = std::min(inOffsetY, 0);
		int max_y = std::max(MagickGetImageHeight(inSourceWand), inOffsetY+MagickGetImageHeight(shadow_wand));

		MagickNewImage(composite_wand, max_x-min_x, max_y-min_y, transparent);
		MagickCompositeImage(composite_wand, shadow_wand, OverCompositeOp, inOffsetX < 0 ? 0 : inOffsetX, inOffsetY < 0 ? 0 : inOffsetY);
		MagickCompositeImage(composite_wand, inSourceWand, OverCompositeOp, inOffsetX < 0 ? -inOffsetX : 0, inOffsetY < 0 ? -inOffsetY : 0);
		
		return composite_wand;
	}

	MagickWand* GradientFillWand(size_t inWidth, size_t inHeight, size_t startOffsetX, size_t startOfsetY, 
								glm::vec4* inColorStack, const GradientCurves* inVerticalCurves = NULL)
	{
		MagickWand* wand = NewMagickWand();
		MagickNewImage(wand, inWidth, inHeight, transparent);

		GradientFillWand(wand, startOffsetX, startOfsetY, inColorStack, inVerticalCurves);
		return wand;
	}

	void GradientFillWand(MagickWand* ioSourceWand, size_t startOffsetX, size_t startOfsetY, glm::vec4* inColorStack, const GradientCurves* inVerticalCurves = NULL)
	{
		auto_scoped_ptr<PixelIterator> iterator(NewPixelIterator(ioSourceWand));

		size_t width = MagickGetImageWidth(ioSourceWand);
		size_t height = MagickGetImageHeight(ioSourceWand);

		char hex[128];
		
		size_t x,y;

		float y_step = 1.0f / (float) (height-startOfsetY);
		float x_step = 1.0f / (float) (width-startOffsetX);
		float yf = 0.0f;
		float xf = 0.0f;

		glm::vec4 row_colors[2];

		int stack_index = inVerticalCurves ? inVerticalCurves->StartMap() : 0;

		for(y=0;y<startOfsetY;y++) {
			// Get the next row of the image as an array of PixelWands
			PixelWand** pixels=PixelGetNextIteratorRow(iterator,&x);

			for(x=startOffsetX;x<width;x++, xf+=x_step) {

				PixelSetColor(pixels[x],"black");
				PixelSetAlpha(pixels[x],0.0f);
			}
			PixelSyncIterator(iterator);
		}

		for(y=startOfsetY;y<height;y++, yf+=y_step) {
			// Get the next row of the image as an array of PixelWands
			PixelWand** pixels=PixelGetNextIteratorRow(iterator,&x);
			// Set the row of wands to a simple gray scale gradient
			
			float mapped_yf = inVerticalCurves ? inVerticalCurves->Map(stack_index, yf) : yf;

			row_colors[0] = inColorStack[stack_index*2+2] * mapped_yf + inColorStack[stack_index*2+0] * (1.0f-mapped_yf);
			row_colors[1] = inColorStack[stack_index*2+3] * mapped_yf + inColorStack[stack_index*2+1] * (1.0f-mapped_yf);

			float temp_dbg = row_colors[0].x * 255.0f;

			xf = 0.0f;
			for(x=startOffsetX;x<width;x++, xf+=x_step) {

				glm::vec4 color = row_colors[1]*xf + row_colors[0]* (1.0f-xf);
				
				sprintf(hex,"rgb(%d,%d,%d)", (int) (color.r * 255.0f),(int) (color.g * 255.0f),(int) (color.b * 255.0f));

				PixelSetColor(pixels[x],hex);
				PixelSetAlpha(pixels[x],color.a);
			}
			// Sync writes the pixels back to the m_wand
			PixelSyncIterator(iterator);
		}
	}

	MagicWand::FontID LoadFont(const char* inPath)
	{
		DrawingWand* new_font = NewDrawingWand();
		DrawSetFont(new_font, "inPath");
		mFontWands.push_back(new_font);

		return ((int) mFontWands.size() - 1);
	}

};


bool MagicWand::MakeTestButtonTexture(GLuint inTexture, GLsizei& outWidth, GLsizei& outHeight)
{
	bool was_read = false;

	auto_scoped_ptr<MagickWand> test_wand(NewMagickWand());
	auto_scoped_ptr<PixelWand> white_wand(NewPixelWand());
	auto_scoped_ptr<PixelWand> red_wand(NewPixelWand());
	auto_scoped_ptr<PixelWand> blue_wand(NewPixelWand());
	auto_scoped_ptr<PixelWand> black_wand(NewPixelWand());
	auto_scoped_ptr<PixelWand> col1_wand(NewPixelWand());
	auto_scoped_ptr<PixelWand> transparent_wand(NewPixelWand());
	PixelSetColor(white_wand, "white");
	PixelSetColor(red_wand, "red");
	PixelSetColor(blue_wand, "blue");
	//PixelSetColor(blue_wand, "black");
	PixelSetColor(black_wand, "black");
	PixelSetColor(col1_wand, "#4096EE");
	//PixelSetColor(col1_wand, "black");
	PixelSetColor(transparent_wand, "transparent");
	auto_scoped_ptr<DrawingWand> drawing_wand(NewDrawingWand());
	
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
	//
		auto_scoped_ptr<DrawingWand> round_drawing_wand(NewDrawingWand());
		MagickWand* round_wand = NewMagickWand();
		MagickNewImage(round_wand, MagickGetImageWidth(test_wand),MagickGetImageHeight(test_wand), transparent_wand);
		
		DrawSetFillColor(round_drawing_wand, white_wand);
		DrawRoundRectangle(round_drawing_wand, 2,2, MagickGetImageWidth(round_wand)-2,MagickGetImageHeight(round_wand)-2, 4, 4);

		MagickDrawImage(round_wand, round_drawing_wand);
		MagickCompositeImage(test_wand, round_wand, CopyOpacityCompositeOp, 0, 0);
	//

	//$shadow = $im->clone();
	auto_scoped_ptr<MagickWand> shadow_wand(CloneMagickWand(test_wand));

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

	if (MagickExportImagePixels(copy_wand, 0, 0, outWidth, outHeight, "RGBA", CharPixel, mImpl->GetTempPixels(outWidth*outHeight, 4)))
	{
		glBindTexture(GL_TEXTURE_2D, inTexture);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, outWidth, outHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, mImpl->GetTempPixels());
		was_read = true;
	}

	return was_read;
}


bool MagicWand::MakeFrameTexture(GLuint inTexture, GLsizei& outWidth, GLsizei& outHeight)
{
	glm::vec4 colors[4] = { glm::vec4(82.0f/255.0f, 82.0f/255.0f, 82.0f/255.0f, 1.0f), 
							glm::vec4(82.0f/255.0f, 82.0f/255.0f, 82.0f/255.0f, 1.0f), 
							glm::vec4(37.0f/255.0f, 37.0f/255.0f, 37.0f/255.0f, 1.0f), 
							glm::vec4(37.0f/255.0f, 37.0f/255.0f, 37.0f/255.0f, 1.0f) };

	auto_scoped_ptr<MagickWand> gradient_wand(mImpl->GradientFillWand(outWidth, outHeight, 0, 0, colors));
	mImpl->RoundWand(gradient_wand, 8, false);

	int offset[2];
	offset[0] = -4;
	offset[1] = 4;

	auto_scoped_ptr<MagickWand> shadowed_wand(mImpl->ShadowWand(gradient_wand, offset[0], offset[1], 40.0f, 1.5f));

	return mImpl->ToGLTexture(shadowed_wand, inTexture, outWidth, outHeight);
}


bool MagicWand::MakeSliderFrameTexture(GLuint inTexture, GLsizei inLength, GLsizei& outWidth, GLsizei& outHeight)
{
	glm::vec3 maincolor(1.0f, 1.0f, 1.0f);
	//glm::vec3 maincolor(0.0f, 0.0f, 0.0f);

	glm::vec4 gradient_colors_stack[4] = { glm::vec4(maincolor.r, maincolor.g, maincolor.b, 0.0f), 
											glm::vec4(maincolor.r, maincolor.g, maincolor.b, 1.0f), 
											glm::vec4(maincolor.r, maincolor.g, maincolor.b, 1.0f), 
											glm::vec4(maincolor.r, maincolor.g, maincolor.b, 0.0f) };

	outWidth = inLength;
	outHeight = 1;

	auto_scoped_ptr<MagickWand> gradient_wand(mImpl->GradientFillWand(outWidth, outHeight, 0, 0, gradient_colors_stack));
	//return mImpl->ToGLTexture(gradient_wand, inTexture, outWidth, outHeight);

	{
		auto_scoped_ptr<DrawingWand> draw(NewDrawingWand());
		
		DrawSetFillColor(draw, mImpl->white);
		DrawSetStrokeColor(draw, mImpl->white);
		DrawSetStrokeWidth(draw, 0.0f);

		PointInfo points[3];
		points[0].x = 0.0f;
		points[0].y = 0.5f;
		points[1].x = inLength;
		points[1].y = 0.0;
		points[2].x = inLength;
		points[2].y = 1.0f;

		DrawPolygon(draw, 3, points);

		auto_scoped_ptr<MagickWand> poly_wand(mImpl->DrawToWand(draw, outWidth, outHeight, mImpl->transparent));

		MagickCompositeImage(gradient_wand, poly_wand, DstInCompositeOp, 0, 0);
	}

	return mImpl->ToGLTexture(gradient_wand, inTexture, outWidth, outHeight);
}

bool MagicWand::MakeSliderMarkerTexture(GLuint inTexture, GLsizei& outWidth, GLsizei& outHeight)
{
	auto_scoped_ptr<DrawingWand> draw(NewDrawingWand());
	auto_scoped_ptr<PixelWand> fill(NewPixelWand());
	auto_scoped_ptr<PixelWand> stroke(NewPixelWand());
	
	//http://www.colorsontheweb.com/colorwizard.asp

	PixelSetColor(stroke, "#000000");
	PixelSetAlpha(stroke, 0.4f);
	//PixelSetColor(fill, "#F47100");
	//PixelSetColor(fill, "#F0A000");
	//PixelSetColor(fill, "#0077F0");
	//PixelSetColor(fill, "#F37103");
	PixelSetColor(fill, "#F27100");
	PixelSetAlpha(fill, 1.0f);

	outWidth = 25;
	outHeight = 12;

	DrawSetStrokeWidth(draw, 1.5f);
	DrawSetFillColor(draw, fill);
	DrawSetStrokeColor(draw, stroke);
	DrawRoundRectangle(draw, 2, 2 ,outWidth-2, outHeight-2, 3, 3);
	
	//auto_scoped_ptr<MagickWand> test_wand(mImpl->DrawToWand(draw, outWidth, outHeight, mImpl->red));
	//MagickWriteImage(test_wand, "test.bmp");

	//auto_scoped_ptr<MagickWand> shadowed_wand(mImpl->ShadowWand(draw, outWidth, outHeight));
	//return mImpl->ToGLTexture(shadowed_wand, inTexture, outWidth, outHeight);

	return mImpl->ToGLTexture(draw, inTexture, outWidth, outHeight);

	//auto_scoped_ptr<MagickWand> shadowed_wand(mImpl->ShadowWand(draw, inWidth, inHeight));
	//return mImpl->ToGLTexture(shadowed_wand, inTexture, inWidth, inHeight);
}

bool MagicWand::ReadImageToGLTexture(const char* inPath, GLuint inTexture, GLsizei& outWidth, GLsizei& outHeight)
{
	bool was_read = false;

	auto_scoped_ptr<MagickWand> wand(NewMagickWand());
	{
		if (MagickReadImage(wand, inPath))
		{
			was_read = mImpl->ToGLTexture(wand, inTexture, outWidth, outHeight);
		}
	}
	
	return was_read;
}


MagicWand::FontID MagicWand::LoadFont(const char* inPath)
{
	return mImpl->LoadFont(inPath);
}


bool MagicWand::MakeButtonTexture(GLuint inTexture, const char* inText, FontID inFontID, float inPointSize, int inAdditionalHorizSpace, int inAdditionalVertSpace, GLsizei& outWidth, GLsizei& outHeight)
{
	DrawingWand* font = mImpl->mFontWands[inFontID];
	DrawSetFontSize(font, inPointSize);
	
	auto_scoped_ptr<MagickWand> empty_wand(NewMagickWand());
	MagickReadImage(empty_wand,"xc:");
	
	double* font_metrics = MagickQueryFontMetrics(empty_wand, font, inText);
	
	float text_width = (float) font_metrics[4];
	float text_height = (float) font_metrics[5];
	
	float radius = 4.0f;
	float additional_size_base = (radius * (1.0f - (1.0f / sqrtf(2.0f))));
	float additional_size[2];
	additional_size[0] = 2.0f * additional_size_base + (float) inAdditionalHorizSpace;
	additional_size[1] = 2.0f * additional_size_base + /*font_metrics[8]*/ + (float) inAdditionalVertSpace;

	float rect_width = text_width+additional_size[0];
	float rect_height = text_height+additional_size[1];

	glm::vec4 gradient_colors_stack[] = {	glm::vec4(85.0f/255.0f, 85/255.0f, 85/255.0f, 1.0f), 
											glm::vec4(85/255.0f, 85/255.0f, 85/255.0f, 1.0f),
											glm::vec4(37.0f/255.0f, 37.0f/255.0f, 37/255.0f, 1.0f), 
											glm::vec4(37.0f/255.0f, 37.0f/255.0f, 37/255.0f, 1.0f),
											glm::vec4(30.0f/255.0f, 30.0f/255.0f, 30/255.0f, 1.0f), 
											glm::vec4(30.0f/255.0f, 30.0f/255.0f, 30/255.0f, 1.0f)};

	MagickWandImpl::GradientCurves gradient_curves;
	gradient_curves.mCurves.resize(2);
	float specular_fraction = 8.0f * (1.0f / rect_height);
	gradient_curves.mCurves[0] = MagickWandImpl::GradientCurve(0.0f, specular_fraction, 0.4f);
	gradient_curves.mCurves[1] = MagickWandImpl::GradientCurve(specular_fraction, 1.0f, 1.0f);

	auto_scoped_ptr<MagickWand> gradient_wand(mImpl->GradientFillWand((size_t) (rect_width), (size_t) (rect_height), 0, 1, gradient_colors_stack, &gradient_curves));
	mImpl->RoundWand(gradient_wand, radius, true);

	DrawSetFillAlpha(font, 1.0);
	DrawSetFontSize(font, inPointSize);
	DrawSetFillColor(font, mImpl->white);
	MagickAnnotateImage(gradient_wand, font, 0.5f*(rect_width-text_width), (font_metrics[8]+ font_metrics[5]) + 0.5f*(rect_height-text_height), 0.0, inText);

	return mImpl->ToGLTexture(gradient_wand, inTexture, outWidth, outHeight);
}


bool MagicWand::MakeTestTexture(GLuint inTexture, int inWidth, int inHeight, GLsizei& outWidth, GLsizei& outHeight)
{
	auto_scoped_ptr<MagickWand> wand(NewMagickWand());
	auto_scoped_ptr<DrawingWand> draw(NewDrawingWand());

	int rounding = 8;

	//mImpl->GetRoundingWandSpace(rounding, true, additional_size[0], additional_size[1]);
	MagickNewImage(wand, inWidth/* + additional_size[0]*/, inHeight/* + additional_size[1]*/, mImpl->white);
	DrawSetFillColor(draw, mImpl->red);
	//DrawSetStrokeWidth(draw, 0.0);
	DrawSetStrokeColor(draw, mImpl->blue);
	DrawRoundRectangle(draw, 0, 0, MagickGetImageWidth(wand)-1,MagickGetImageHeight(wand)-1, rounding, rounding);
	MagickDrawImage(wand, draw);

	return mImpl->ToGLTexture(wand, inTexture, outWidth, outHeight);
}


MagicWand::MagicWand()
{
	mImpl = new MagickWandImpl();
}

MagicWand::~MagicWand()
{
	delete mImpl;
}

}