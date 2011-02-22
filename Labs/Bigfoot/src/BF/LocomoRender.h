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
	float mFootAngle;
};

class LocomoBodyRenderStyle
{
public:

	LocomoFootRenderStyle lfoot;
	LocomoFootRenderStyle rfoot;
	
	const LocomoFootRenderStyle& supportFoot(EFootFlag support) const { return (support & ELFlag) ? lfoot : rfoot; }
	const LocomoFootRenderStyle& otherFoot(EFootFlag support) const { return (support & ELFlag) ? rfoot : lfoot; }
};

void draw(const glm::mat4& viewMat, const LocomoBodyRenderStyle& style, const std::vector<LocomoState>& states)
{
	glPushAttrib(GL_TEXTURE_BIT | GL_COLOR_BUFFER_BIT);
	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
							
	for (int i = 0; i < states.size(); ++i)
	{
		const FootState& fs = states[i].supportFoot();
		const LocomoFootRenderStyle& foot_style = style.supportFoot(states[i].support);

		//if ((states[i].support & ERFlag) != 0)
		//	continue;

		glm::vec3 pos_3d(fs.pos.x, 0.0f, fs.pos.y);
		glm::mat4 model_mat = glm::rotate(glm::mat4(), -(fs.dir + foot_style.mFootAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		model_mat[3][0] = pos_3d[0];
		model_mat[3][1] = pos_3d[1];
		model_mat[3][2] = pos_3d[2];

		glm::mat4 model_view_mat = viewMat * model_mat;

		// Optimize!
		glLoadMatrixf(glm::value_ptr(model_view_mat));
		glBindTexture(GL_TEXTURE_2D, foot_style.mTex.mTexture);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);	// trilinear
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	

		float sz = 0.5f * foot_style.mFootSizeWorld;

		glBegin(GL_QUADS);			
		//glColor4f(1.0f,0.0f,0.0f, 0.5f);						
		glTexCoord2f(0.0f,1.0f); glVertex3f(sz, 0.5f * sz, -sz);
		glTexCoord2f(0.0f,0.0f); glVertex3f(sz, 0.5f * sz, sz);
		glTexCoord2f(1.0f,0.0f); glVertex3f(-sz, 0.5f * sz, sz);
		glTexCoord2f(1.0f,1.0f); glVertex3f(-sz, 0.5f * sz, -sz);
		glEnd();	
	}
		
	glPopAttrib();
}

}

#endif