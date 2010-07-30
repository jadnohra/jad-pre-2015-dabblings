#include "MagickWand.h"
#include <stdio.h>
#include <vector>
#include "float.h"

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
    static inline void doDelete(MagickWand* ptr) { if (ptr) ClearMagickWand(ptr); }
};


template<>
struct PtrDeletePolicy_delete<PixelWand> {

    static inline PixelWand* getDefault() { return NULL; }
    static inline void doDelete(PixelWand* ptr) { if (ptr) ClearPixelWand(ptr); }
};

template<>
struct PtrDeletePolicy_delete<DrawingWand> {

    static inline DrawingWand* getDefault() { return NULL; }
    static inline void doDelete(DrawingWand* ptr) { if (ptr) ClearDrawingWand(ptr); }
};

template<>
struct PtrDeletePolicy_delete<PixelIterator> {

    static inline PixelIterator* getDefault() { return NULL; }
    static inline void doDelete(PixelIterator* ptr) { if (ptr) ClearPixelIterator(ptr); }
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
	auto_scoped_ptr<PixelWand> orange1;
	auto_scoped_ptr<PixelWand> yellow;
	auto_scoped_ptr<PixelWand> yellow1;
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
	,	orange1(NewPixelWand())
	,	yellow(NewPixelWand())
	,	yellow1(NewPixelWand())
	,	transparent(NewPixelWand())
	{
		PixelSetColor(white, "white");
		PixelSetColor(red, "red");
		PixelSetColor(blue, "blue");
		PixelSetColor(black, "black");
		PixelSetColor(black_mask, "black");
		PixelSetAlpha(black_mask, 0.0);
		PixelSetColor(blue1, "#4096EE");
		PixelSetColor(orange1, "#F06D00");
		PixelSetColor(yellow, "#FFFF00");
		PixelSetColor(yellow1, "#F9AB16");
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
	MagickWand* ShadowDrawWand(DrawingWand* draw, GLsizei inWidth, GLsizei inHeight, 
								int& outSourceOffsetX, int& outSourceOffsetY,
								float inShadowSizeRelOffsetX=0.0f, float inShadowSizeRelOffsetY=0.0f, 
								int inOffsetX=0, int inOffsetY=0, 
								float inOpacity=40.0f, float inSigma=1.0f)
	{
		auto_scoped_ptr<MagickWand> wand(DrawToWand(draw, inWidth, inHeight, transparent));
		return ShadowWand(wand, outSourceOffsetX, outSourceOffsetY, inShadowSizeRelOffsetX, inShadowSizeRelOffsetY, inOffsetX, inOffsetY, inOpacity, inSigma);
	}

	// Returns result
	MagickWand* ShadowWand(MagickWand* inSourceWand,
								int& outSourceOffsetX, int& outSourceOffsetY,
								float inShadowSizeRelOffsetX=0.0f, float inShadowSizeRelOffsetY=0.0f, 
								int inOffsetX=0, int inOffsetY=0, 
								float inOpacity=40.0f, float inSigma=1.0f)
	{
		auto_scoped_ptr<MagickWand> shadow_wand(CloneMagickWand(inSourceWand));

		//$shadow->setImageBackgroundColor( new ImagickPixel('black') );
		MagickSetImageBackgroundColor(shadow_wand, black);

		//$shadow->shadowImage( 40, 1, 1, 1 );
		MagickShadowImage(shadow_wand, inOpacity, inSigma, 0, 0);

		//$shadow->compositeImage( $im, Imagick::COMPOSITE_OVER, 0, 0 );
		//MagickCompositeImage(shadow_wand, inSourceWand, OverCompositeOp, inOffsetX, inOffsetY);

		int orig_size_x = MagickGetImageWidth(inSourceWand);
		int orig_size_y = MagickGetImageHeight(inSourceWand);
		int shadow_size_x = MagickGetImageWidth(shadow_wand);
		int shadow_size_y = MagickGetImageHeight(shadow_wand);
		int shadow_size_diff_x = shadow_size_x - orig_size_x;
		int shadow_size_diff_y = shadow_size_y - orig_size_y;
		int source_center_offest_x = inOffsetX + (inShadowSizeRelOffsetX * 0.5f * shadow_size_diff_x);
		int source_center_offest_y = inOffsetY + (inShadowSizeRelOffsetY * 0.5f * shadow_size_diff_y);
		int shadow_center_left_corner_x = -shadow_size_x/2;
		int shadow_center_left_corner_y = -shadow_size_y/2;
		int shadow_center_right_corner_x = shadow_size_x/2;
		int shadow_center_right_corner_y = shadow_size_y/2;
		int source_center_left_corner_x = -(orig_size_x/2)+source_center_offest_x;
		int source_center_left_corner_y = -(orig_size_y/2)+source_center_offest_y;
		int source_center_right_corner_x = (orig_size_x/2)-source_center_offest_x;
		int source_center_right_corner_y = (orig_size_y/2)-source_center_offest_y;
		int composite_center_left_corner_x = std::min(shadow_center_left_corner_x, source_center_left_corner_x);
		int composite_center_left_corner_y = std::min(shadow_center_left_corner_y, source_center_left_corner_y);
		int composite_center_right_corner_x = std::max(shadow_center_right_corner_x, source_center_right_corner_x);
		int composite_center_right_corner_y = std::max(shadow_center_right_corner_y, source_center_right_corner_y);

		MagickWand* composite_wand = NewMagickWand();
		MagickNewImage(composite_wand, composite_center_right_corner_x-composite_center_left_corner_x, 
										composite_center_right_corner_y-composite_center_left_corner_y, transparent/*red*/);
		
		int shadow_wand_on_composite_offset_x = std::max(shadow_center_left_corner_x - source_center_left_corner_x, 0);
		int shadow_wand_on_composite_offset_y = std::max(shadow_center_left_corner_y - source_center_left_corner_y, 0);
		MagickCompositeImage(composite_wand, shadow_wand, OverCompositeOp, shadow_wand_on_composite_offset_x, shadow_wand_on_composite_offset_y);

		int source_wand_on_composite_offset_x = std::max(source_center_left_corner_x - shadow_center_left_corner_x, 0);
		int source_wand_on_composite_offset_y = std::max(source_center_left_corner_y - shadow_center_left_corner_y, 0);
		MagickCompositeImage(composite_wand, inSourceWand, OverCompositeOp, source_wand_on_composite_offset_x, source_wand_on_composite_offset_y);
		
		outSourceOffsetX = source_wand_on_composite_offset_x;
		outSourceOffsetY = source_wand_on_composite_offset_y;

		return composite_wand;
	}



	MagickWand* FillWand(size_t inWidth, size_t inHeight, const PixelWand& inColor)
	{
		MagickWand* wand = NewMagickWand();
		MagickNewImage(wand, inWidth, inHeight, &inColor);
	
		return wand;
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

	PixelWand* MakeColor(float inR, float  inG, float  inB)
	{
		return MakeColor255((int) (inR*255.0f), (int) (inG*255.0f), (int) (inB*255.0f));
	}

	PixelWand* MakeColor(float  inR, float  inG, float  inB, float  inA)
	{
		return MakeColor255((int) (inR*255.0f), (int) (inG*255.0f), (int) (inB*255.0f), (int) (inA*255.0f));
	}

	PixelWand* MakeColor255(int inR, int inG, int inB)
	{
		PixelWand* color = NewPixelWand();

		char hex[128];
		sprintf(hex,"rgb(%d,%d,%d)", inR, inG, inB);
		PixelSetColor(color, hex);

		return color;
	}


	PixelWand* MakeColor255(int inR, int inG, int inB, int inA)
	{
		PixelWand* color = MakeColor255(inR, inG, inB);
		PixelSetAlpha(color, (float) inA/255.0f);

		return color;
	}

	MagicWand::FontID LoadFont(const char* inPath)
	{
		DrawingWand* new_font = NewDrawingWand();
		DrawSetFont(new_font, "inPath");
		mFontWands.push_back(new_font);

		return ((int) mFontWands.size() - 1);
	}

};


MagicWand::TextInfo::TextInfo()
:	mFontID(0)
,	mPointSize(12.0f)
,	mBold(false)
{
}

MagicWand::TextInfo::TextInfo(const char* inText, FontID inFontID, float inPointSize, bool inBold, const glm::vec2& inMinEmptySpace)
:	mText(inText)
,	mFontID(inFontID)
,	mPointSize(inPointSize)
,	mBold(inBold)
,	mMinEmptySpace(inMinEmptySpace)
{
}

MagicWand::SizeConstraints::SizeConstraints()
{
	mMaxSize.x = FLT_MAX;
	mMaxSize.y = FLT_MAX;
}

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


bool MagicWand::MakeFrameTexture(FrameType inType, bool inUseGradient, GLuint inTexture, GLsizei& outWidth, GLsizei& outHeight, glm::vec2& outInternalPos, glm::vec2& outInternalSize)
{
	int source_end_offset[2] = { 0, 0 };
	int round_radius = 8;
	bool ret = false;

	GLsizei inWidth = outWidth;
	GLsizei inHeight = outHeight;

	int stroke_size = 0;

	if (inType == FRAME_NORMAL)
	{

		static glm::vec4 colors[4] = { 
				glm::vec4(82.0f/255.0f, 82.0f/255.0f, 82.0f/255.0f, 1.0f), 
				glm::vec4(82.0f/255.0f, 82.0f/255.0f, 82.0f/255.0f, 1.0f), 
				glm::vec4(37.0f/255.0f, 37.0f/255.0f, 37.0f/255.0f, 1.0f), 
				glm::vec4(37.0f/255.0f, 37.0f/255.0f, 37.0f/255.0f, 1.0f) };

		auto_scoped_ptr<MagickWand> background_wand;

		if (inUseGradient)
		{
			background_wand.reset(mImpl->GradientFillWand(outWidth, outHeight, 0, 0, colors));
		}
		else
		{
			background_wand.reset(mImpl->FillWand(outWidth, outHeight, *mImpl->black));
		}

		stroke_size = 1;
		mImpl->RoundWand(background_wand, 8, true);

		int offset_in_shadow[2];
		offset_in_shadow[0] = -2;
		offset_in_shadow[1] = -4;

		auto_scoped_ptr<MagickWand> shadowed_wand(mImpl->ShadowWand(background_wand, source_end_offset[0], source_end_offset[1], 0.0f, 0.0f, offset_in_shadow[0], offset_in_shadow[1], 40.0f, 1.5f));

		ret = mImpl->ToGLTexture(shadowed_wand, inTexture, outWidth, outHeight);
	}
	else if (inType == FRAME_NORMAL_CUT_UPPER)
	{
		static glm::vec4 colors[4] = { 
			glm::vec4(39.0f/255.0f, 39.0f/255.0f, 39.0f/255.0f, 1.0f), 
			glm::vec4(39.0f/255.0f, 39.0f/255.0f, 39.0f/255.0f, 1.0f), 
			glm::vec4(39.0f/255.0f, 39.0f/255.0f, 39.0f/255.0f, 1.0f), 
			glm::vec4(39.0f/255.0f, 39.0f/255.0f, 39.0f/255.0f, 1.0f) };

		auto_scoped_ptr<MagickWand> background_wand;

		if (inUseGradient && false)
		{
			background_wand.reset(mImpl->GradientFillWand(outWidth, outHeight, 0, 0, colors));
		}
		else
		{
			auto_scoped_ptr<PixelWand> color(mImpl->MakeColor255(39, 39, 39));
			background_wand.reset(mImpl->FillWand(outWidth, outHeight, /* *mImpl->white */ *color));
		}

		//mImpl->RoundWand(background_wand, 8, true);
		//auto_scoped_ptr<MagickWand> cut_wand(NewMagickWand());
		//MagickNewImage(cut_wand, MagickGetImageWidth(background_wand), MagickGetImageHeight(background_wand)-8, mImpl->transparent);
		//MagickCompositeImage(cut_wand, background_wand, OverCompositeOp, 0, -8);

		int offset_in_shadow[2];
		offset_in_shadow[0] = -2;
		offset_in_shadow[1] = -4;

		auto_scoped_ptr<MagickWand> shadowed_wand(mImpl->ShadowWand(/*cut_wand*/ background_wand, source_end_offset[0], source_end_offset[1], 0.0f, 0.0f, offset_in_shadow[0], offset_in_shadow[1], 40.0f, 1.5f));
		
		//return mImpl->ToGLTexture(cut_wand, inTexture, outWidth, outHeight);
		//return mImpl->ToGLTexture(gradient_wand, inTexture, outWidth, outHeight);
		ret = mImpl->ToGLTexture(shadowed_wand, inTexture, outWidth, outHeight);
	}

	outInternalPos.x = source_end_offset[0] + stroke_size;
	outInternalPos.y = source_end_offset[1] + stroke_size;
	outInternalSize.x = (float) inWidth - (2*stroke_size);
	outInternalSize.y = (float) inHeight - (2*stroke_size);
	
	return ret;
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


bool MagicWand::MakeSliderFrameTexture(GLuint inTexture, GLsizei inLength, const TextInfo& inTextInfo, const SizeConstraints& inSizeConstraints, GLsizei& outWidth, GLsizei& outHeight)
{
	DrawingWand* font = mImpl->mFontWands[inTextInfo.mFontID];
	DrawSetFillAlpha(font, 1.0);
	DrawSetFontSize(font, inTextInfo.mPointSize);
	DrawSetFillColor(font, mImpl->white);
	DrawSetFontWeight(font, inTextInfo.mBold ? 700 : 400);
	
	auto_scoped_ptr<MagickWand> empty_wand(NewMagickWand());
	MagickReadImage(empty_wand,"xc:");
	
	double* font_metrics = MagickQueryFontMetrics(empty_wand, font, inTextInfo.mText.c_str());
	
	float text_width = (float) font_metrics[4];
	float text_height = (float) font_metrics[5];
	
	float radius = 4.0f;
	float additional_size_base = (radius * (1.0f - (1.0f / sqrtf(2.0f))));
	float additional_size[2];
	additional_size[0] = 2.0f * additional_size_base + inTextInfo.mMinEmptySpace.x;
	additional_size[1] = 2.0f * additional_size_base + /*font_metrics[8]*/ + inTextInfo.mMinEmptySpace.y;

	float rect_width = text_width+additional_size[0];
	float rect_height = text_height+additional_size[1];

	if (inLength > text_width + additional_size[0])
	{
		additional_size[0] = 0.0f;
		rect_width = inLength;
	}

	rect_width = std::max(rect_width, horiz2d(inSizeConstraints.mMinSize));
	rect_height = std::max(rect_height, vert2d(inSizeConstraints.mMinSize));

	rect_width = std::min(rect_width, horiz2d(inSizeConstraints.mMaxSize));
	rect_height = std::min(rect_height, vert2d(inSizeConstraints.mMaxSize));

	static glm::vec4 gradient_colors_stack[] = {	
					glm::vec4(0.0f/255.0f, 0.0f/255.0f, 0.0f/255.0f, 1.0f), 
					glm::vec4(0.0f/255.0f, 0.0f/255.0f, 0.0f/255.0f, 1.0f), 
					glm::vec4(30.0f/255.0f, 30.0f/255.0f, 30.0f/255.0f, 1.0f), 
					glm::vec4(30.0f/255.0f, 30.0f/255.0f, 30.0f/255.0f, 1.0f),
					glm::vec4(40.0f/255.0f, 40.0f/255.0f, 40.0f/255.0f, 1.0f),
					glm::vec4(40.0f/255.0f, 40.0f/255.0f, 40.0f/255.0f, 1.0f)};

	glm::vec4* used_gradient_colors_stack = gradient_colors_stack;

	MagickWandImpl::GradientCurves gradient_curves;
	gradient_curves.mCurves.resize(2);
	float specular_fraction = 8.0f * (1.0f / rect_height);
	gradient_curves.mCurves[0] = MagickWandImpl::GradientCurve(0.0f, specular_fraction, 0.4f);
	gradient_curves.mCurves[1] = MagickWandImpl::GradientCurve(specular_fraction, 1.0f, 1.0f);


	auto_scoped_ptr<MagickWand> gradient_wand(mImpl->GradientFillWand((size_t) (rect_width), (size_t) (rect_height), 0, 0, used_gradient_colors_stack, &gradient_curves));
	mImpl->RoundWand(gradient_wand, radius, false);

	MagickAnnotateImage(gradient_wand, font, inTextInfo.mMinEmptySpace.x, text_height, 0.0, inTextInfo.mText.c_str());

	return mImpl->ToGLTexture(gradient_wand, inTexture, outWidth, outHeight);
}


bool MagicWand::MakeVerticalSliderFrameTexture(GLuint inTexture, GLsizei inWidth, GLsizei inHeight, GLsizei& outWidth, GLsizei& outHeight)
{
	float rect_width = (float) inWidth;
	float rect_height = (float) inHeight;
	float radius = 4.0f;

	static glm::vec4 gradient_colors_stack[] = {	
					glm::vec4(0.0f/255.0f, 0.0f/255.0f, 0.0f/255.0f, 1.0f), 
					glm::vec4(0.0f/255.0f, 0.0f/255.0f, 0.0f/255.0f, 1.0f), 
					glm::vec4(30.0f/255.0f, 30.0f/255.0f, 30.0f/255.0f, 1.0f), 
					glm::vec4(30.0f/255.0f, 30.0f/255.0f, 30.0f/255.0f, 1.0f),
					glm::vec4(40.0f/255.0f, 40.0f/255.0f, 40.0f/255.0f, 1.0f),
					glm::vec4(40.0f/255.0f, 40.0f/255.0f, 40.0f/255.0f, 1.0f)};

	glm::vec4* used_gradient_colors_stack = gradient_colors_stack;

	MagickWandImpl::GradientCurves gradient_curves;
	gradient_curves.mCurves.resize(2);
	float specular_fraction = 0.9f; //8.0f * (1.0f / rect_height);
	gradient_curves.mCurves[0] = MagickWandImpl::GradientCurve(0.0f, specular_fraction, 0.4f);
	gradient_curves.mCurves[1] = MagickWandImpl::GradientCurve(specular_fraction, 1.0f, 1.0f);


	auto_scoped_ptr<MagickWand> gradient_wand(mImpl->GradientFillWand((size_t) (rect_width), (size_t) (rect_height), 0, 0, used_gradient_colors_stack, &gradient_curves));
	mImpl->RoundWand(gradient_wand, radius, false);

	return mImpl->ToGLTexture(gradient_wand, inTexture, outWidth, outHeight);
}

bool MagicWand::MakeSliderMarkerTexture(GLuint inTexture, WidgetState inWidgetState, GLsizei inFrameHeight, GLsizei& outWidth, GLsizei& outHeight)
{
	auto_scoped_ptr<DrawingWand> draw(NewDrawingWand());
	auto_scoped_ptr<PixelWand> fill(NewPixelWand());
	auto_scoped_ptr<PixelWand> stroke(NewPixelWand());
	
	PixelSetColor(stroke, "#000000");
	PixelSetAlpha(stroke, 0.4f);
	if (inWidgetState == WIDGET_NORMAL) PixelSetColor(fill, "#FFFFFF");
	if (inWidgetState == WIDGET_PRESSED) PixelSetColor(fill, "#F06D00");
	if (inWidgetState == WIDGET_HIGHLIGHTED) PixelSetColor(fill, "#F9AB16");
	PixelSetAlpha(fill, inWidgetState == WIDGET_PRESSED ? 1.0f : 0.7f);

	outHeight = inFrameHeight - 2;
	outWidth = (3*outHeight)/4;

	DrawSetStrokeWidth(draw, 1.0f);
	DrawSetFillColor(draw, fill);
	DrawSetStrokeColor(draw, stroke);
	DrawRoundRectangle(draw, 0, 0,outWidth-1, outHeight-1, 3, 3);
	
	return mImpl->ToGLTexture(draw, inTexture, outWidth, outHeight);
}


bool MagicWand::MakeVerticalSliderMarkerTexture(GLuint inTexture, WidgetState inWidgetState, GLsizei inFrameWidth, GLsizei& outWidth, GLsizei& outHeight)
{
	auto_scoped_ptr<DrawingWand> draw(NewDrawingWand());
	auto_scoped_ptr<PixelWand> fill(NewPixelWand());
	auto_scoped_ptr<PixelWand> stroke(NewPixelWand());
	
	PixelSetColor(stroke, "#000000");
	PixelSetAlpha(stroke, 0.4f);
	if (inWidgetState == WIDGET_NORMAL) PixelSetColor(fill, "#FFFFFF");
	if (inWidgetState == WIDGET_PRESSED) PixelSetColor(fill, "#F06D00");
	if (inWidgetState == WIDGET_HIGHLIGHTED) PixelSetColor(fill, "#F9AB16");
	PixelSetAlpha(fill, inWidgetState == WIDGET_PRESSED ? 1.0f : 0.7f);

	outWidth = inFrameWidth - 2;
	outHeight = (3*outWidth)/4;

	DrawSetStrokeWidth(draw, 1.0f);
	DrawSetFillColor(draw, fill);
	DrawSetStrokeColor(draw, stroke);
	DrawRoundRectangle(draw, 0, 0,outWidth-1, outHeight-1, 3, 3);
	
	return mImpl->ToGLTexture(draw, inTexture, outWidth, outHeight);
}


bool MagicWand::MakeButtonTexture(GLuint inTexture, WidgetState inWidgetState, const TextInfo& inTextInfo, const SizeConstraints& inSizeConstraints, GLsizei& outWidth, GLsizei& outHeight)
{
	DrawingWand* font = mImpl->mFontWands[inTextInfo.mFontID];
	DrawSetFillAlpha(font, 1.0);
	DrawSetFontSize(font, inTextInfo.mPointSize);
	if (inWidgetState == WIDGET_PRESSED) DrawSetFillColor(font, mImpl->black);
	if (inWidgetState == WIDGET_NORMAL) DrawSetFillColor(font, mImpl->white);
	if (inWidgetState == WIDGET_HIGHLIGHTED) DrawSetFillColor(font, mImpl->yellow1);
	DrawSetFontWeight(font, inTextInfo.mBold ? 700 : 400);
	
	auto_scoped_ptr<MagickWand> empty_wand(NewMagickWand());
	MagickReadImage(empty_wand,"xc:");
	
	double* font_metrics = MagickQueryFontMetrics(empty_wand, font, inTextInfo.mText.c_str());
	
	float text_width = (float) font_metrics[4];
	float text_height = (float) font_metrics[5];
	
	float radius = 4.0f;
	float additional_size_base = (radius * (1.0f - (1.0f / sqrtf(2.0f))));
	float additional_size[2];
	additional_size[0] = 2.0f * additional_size_base + inTextInfo.mMinEmptySpace.x;
	additional_size[1] = 2.0f * additional_size_base + /*font_metrics[8]*/ + inTextInfo.mMinEmptySpace.y;

	float rect_width = text_width+additional_size[0];
	float rect_height = text_height+additional_size[1];

	rect_width = std::max(rect_width, horiz2d(inSizeConstraints.mMinSize));
	rect_height = std::max(rect_height, vert2d(inSizeConstraints.mMinSize));

	rect_width = std::min(rect_width, horiz2d(inSizeConstraints.mMaxSize));
	rect_height = std::min(rect_height, vert2d(inSizeConstraints.mMaxSize));

	static glm::vec4 gradient_colors_stack[] = {	
					glm::vec4(85.0f/255.0f, 85/255.0f, 85/255.0f, 1.0f), 
					glm::vec4(85/255.0f, 85/255.0f, 85/255.0f, 1.0f),
					glm::vec4(37.0f/255.0f, 37.0f/255.0f, 37.0f/255.0f, 1.0f), 
					glm::vec4(37.0f/255.0f, 37.0f/255.0f, 37.0f/255.0f, 1.0f),
					glm::vec4(30.0f/255.0f, 30.0f/255.0f, 30.0f/255.0f, 1.0f), 
					glm::vec4(30.0f/255.0f, 30.0f/255.0f, 30.0f/255.0f, 1.0f)};

	static glm::vec4 pressed_gradient_colors_stack[] = {	
					glm::vec4(110.0f/255.0f, 48/255.0f, 0.0/255.0f, 1.0f), 
					glm::vec4(110.0f/255.0f, 48/255.0f, 0.0/255.0f, 1.0f),
					glm::vec4(234.0f/255.0f, 107.0f/255.0f, 0.0f/255.0f, 1.0f), 
					glm::vec4(234.0f/255.0f, 107.0f/255.0f, 0.0f/255.0f, 1.0f), 
					glm::vec4(243.0f/255.0f, 111.0f/255.0f, 0.0f/255.0f, 1.0f), 
					glm::vec4(243.0f/255.0f, 111.0f/255.0f, 0.0f/255.0f, 1.0f)};

	glm::vec4* used_gradient_colors_stack = inWidgetState == WIDGET_PRESSED ? pressed_gradient_colors_stack : gradient_colors_stack;

	MagickWandImpl::GradientCurves gradient_curves;
	gradient_curves.mCurves.resize(2);
	float specular_fraction = 8.0f * (1.0f / rect_height);
	gradient_curves.mCurves[0] = MagickWandImpl::GradientCurve(0.0f, specular_fraction, 0.4f);
	gradient_curves.mCurves[1] = MagickWandImpl::GradientCurve(specular_fraction, 1.0f, 1.0f);


	auto_scoped_ptr<MagickWand> gradient_wand(mImpl->GradientFillWand((size_t) (rect_width), (size_t) (rect_height), 0, 1, used_gradient_colors_stack, &gradient_curves));
	mImpl->RoundWand(gradient_wand, radius, true);

	//(font_metrics[8]+ font_metrics[5]) + 0.5f*(rect_height-text_height)
	MagickAnnotateImage(gradient_wand, font, 0.5f*(rect_width-text_width), text_height, 0.0, inTextInfo.mText.c_str());

	return mImpl->ToGLTexture(gradient_wand, inTexture, outWidth, outHeight);
}


bool MagicWand::MakeTextTexture(GLuint inTexture, const TextInfo& inTextInfo, const SizeConstraints& inSizeConstraints, GLsizei& outWidth, GLsizei& outHeight)
{
	DrawingWand* font = mImpl->mFontWands[inTextInfo.mFontID];
	DrawSetFillAlpha(font, 1.0);
	DrawSetFontSize(font, inTextInfo.mPointSize);
	DrawSetFillColor(font, mImpl->white);
	DrawSetFontWeight(font, inTextInfo.mBold ? 700 : 400);
	
	auto_scoped_ptr<MagickWand> empty_wand(NewMagickWand());
	MagickReadImage(empty_wand,"xc:");
	
	double* font_metrics = MagickQueryFontMetrics(empty_wand, font, inTextInfo.mText.c_str());
	
	float text_width = (float) font_metrics[4];
	float text_height = (float) font_metrics[5];
	
	float radius = 4.0f;
	float additional_size_base = (radius * (1.0f - (1.0f / sqrtf(2.0f))));
	float additional_size[2];
	additional_size[0] = 2.0f * additional_size_base + inTextInfo.mMinEmptySpace.x;
	additional_size[1] = 2.0f * additional_size_base + /*font_metrics[8]*/ + inTextInfo.mMinEmptySpace.y;

	float rect_width = text_width+additional_size[0];
	float rect_height = text_height+additional_size[1];

	rect_width = std::max(rect_width, horiz2d(inSizeConstraints.mMinSize));
	rect_height = std::max(rect_height, vert2d(inSizeConstraints.mMinSize));

	rect_width = std::min(rect_width, horiz2d(inSizeConstraints.mMaxSize));
	rect_height = std::min(rect_height, vert2d(inSizeConstraints.mMaxSize));

	auto_scoped_ptr<MagickWand> wand(NewMagickWand());
	MagickNewImage(wand, (size_t) (rect_width), (size_t) (rect_height), mImpl->transparent);

	MagickAnnotateImage(wand, font, 0.5f*(rect_width-text_width), text_height, 0.0, inTextInfo.mText.c_str());

	return mImpl->ToGLTexture(wand, inTexture, outWidth, outHeight);
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