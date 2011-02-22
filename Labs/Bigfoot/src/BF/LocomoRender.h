#ifndef _INCLUDED_BIGFOOT_LOCOMO_RENDER_H
#define _INCLUDED_BIGFOOT_LOCOMO_RENDER_H

#include "Locomo.h"
#include "Rendering.h"
#include "BFMath.h"

namespace BF
{

class LocomoFootRenderStyle
{
public:

	BE::OGLTexture mTex;
	GLsizei mTexWidth;
	GLsizei mTexHeight;
	float mFootSizeWorld;
};

class LocomoBodyRenderStyle
{
public:

	LocomoFootRenderStyle lfoot;
	LocomoFootRenderStyle rfoot;
};

void draw(const glm::mat4& viewMat, const LocomoBodyRenderStyle& style, const std::vector<LocomoState>& states)
{
	glPushAttrib(GL_TEXTURE_BIT | GL_COLOR_BUFFER_BIT);
	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, style.lfoot.mTex.mTexture);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);	// trilinear
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
	
						
	for (int i = 0; i < states.size(); ++i)
	{
		const FootState& fs = states[i].supportFoot();

		glm::vec3 pos_3d(fs.pos.x, 0.0f, fs.pos.y);
		glm::mat4 model_mat = glm::rotate(glm::mat4(), 90.0f + fs.dir, glm::vec3(0.0f, 1.0f, 0.0f));
		model_mat[3][0] = pos_3d[0];
		model_mat[3][1] = pos_3d[1];
		model_mat[3][2] = pos_3d[2];

		glm::mat4 model_view_mat = viewMat * model_mat;
		
		glLoadMatrixf(glm::value_ptr(model_view_mat));

		glBegin(GL_QUADS);			
		//glColor4f(1.0f,0.0f,0.0f, 0.5f);						
		glTexCoord2f(0.0f,0.0f); glVertex3f(-0.5f * style.lfoot.mFootSizeWorld, 0.0f, -0.5f * style.lfoot.mFootSizeWorld);
		glTexCoord2f(-1.0f,0.0f); glVertex3f(-0.5f * style.lfoot.mFootSizeWorld, 0.0f, 0.5f * style.lfoot.mFootSizeWorld);
		glTexCoord2f(-1.0f,1.0f); glVertex3f(0.5f * style.lfoot.mFootSizeWorld, 0.0f, 0.5f * style.lfoot.mFootSizeWorld);
		glTexCoord2f(0.0f,1.0f); glVertex3f(0.5f * style.lfoot.mFootSizeWorld, 0.0f, -0.5f * style.lfoot.mFootSizeWorld);
		glEnd();	
	}
		
	glPopAttrib();
}

}

#endif