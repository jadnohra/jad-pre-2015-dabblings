#include "OGLFontRender.h"

#include "stdio.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

namespace BE
{

unsigned char temp_bitmap[512*512];
stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs

bool OGLFontInstance::Create(const char* inFilePath, float inPixelHeight)
{
	FILE* file = NULL;
	mPixelHeight = inPixelHeight;
	
	if (fopen_s(&file, inFilePath, "rb") == 0)
	{
		char* ttf_buffer = NULL;
		fseek(file, 0, SEEK_END);
		long size = ftell(file);

		if (size > 0)
		{
			unsigned char* ttf_buffer = (unsigned char*) malloc(size);
			
			fseek(file, 0, SEEK_SET);
			if (fread(ttf_buffer, 1, size, file) == size)
			{
				stbtt_BakeFontBitmap(ttf_buffer,0, inPixelHeight, temp_bitmap,512,512, 32,96, cdata); // no guarantee this fits!
				free(ttf_buffer);

				glGenTextures(1, &mTexture); //TODO free texture
				glBindTexture(GL_TEXTURE_2D, mTexture);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512,512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, temp_bitmap);
				// can free temp_bitmap at this point
			}
		}

		fclose(file);
	}

	return false;
}


static void getBakedQuad(stbtt_bakedchar *chardata, int pw, int ph, int char_index,
						 float *xpos, float *ypos, stbtt_aligned_quad *q)
{
	stbtt_bakedchar *b = chardata + char_index;
	int round_x = STBTT_ifloor(*xpos + b->xoff);
	int round_y = STBTT_ifloor(*ypos - b->yoff);
	
	q->x0 = (float)round_x;
	q->y0 = (float)round_y;
	q->x1 = (float)round_x + b->x1 - b->x0;
	q->y1 = (float)round_y - b->y1 + b->y0;
	
	q->s0 = b->x0 / (float)pw;
	q->t0 = b->y0 / (float)pw;
	q->s1 = b->x1 / (float)ph;
	q->t1 = b->y1 / (float)ph;
	
	*xpos += b->xadvance;
}

void OGLFontInstance::Render(const char* inText, float inPosX, float inPosY) const
{
	// assume orthographic projection with units = screen pixels, origin at top left
   //glBindTexture(GL_TEXTURE_2D, mTexture);
   //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   
	//inPosY += mPixelHeight;

	glBegin(GL_QUADS);
	while (*inText) {
      if (*inText >= 32 && *inText < 128) {
         stbtt_aligned_quad q;
         stbtt_GetBakedQuad(cdata, 512,512, *inText-32, &inPosX,&inPosY,&q,1);//1=opengl,0=old d3d
		 //getBakedQuad(cdata, 512,512, *inText-32, &inPosX,&inPosY,&q);
         glTexCoord2f(q.s0,q.t0); glVertex2f(q.x0,q.y0);
         glTexCoord2f(q.s1,q.t0); glVertex2f(q.x1,q.y0);
         glTexCoord2f(q.s1,q.t1); glVertex2f(q.x1,q.y1);
         glTexCoord2f(q.s0,q.t1); glVertex2f(q.x0,q.y1);
      }
      ++inText;
   }
   glEnd();
}




OGLState_FontRender::OGLState_FontRender(OGLFontInstance& inFont)
:	mFont(inFont)
{
}


void OGLState_FontRender::Set()
{
	//http://www.opengl-doc.com/Sams-OpenGL.SuperBible.Third/0672326019/ch06lev1sec1.html

	//glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA,GL_ONE);	
	//glBlendFunc(GL_DST_COLOR,GL_ZERO);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, mFont.mTexture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);	// Linear Filtering
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	// Linear Filtering
	glColor4f(0.0f,0.0f,0.0f,1.0f);			// Full Brightness, 50% Alpha ( NEW )
}


}