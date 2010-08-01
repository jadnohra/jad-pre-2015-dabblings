#include "RenderNodes.h"
#include "BEMath.h"

namespace BE
{

void PushScissorNode::Set(Renderer& inRenderer)	
{
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	glEnable(GL_SCISSOR_TEST);
	glScissor((int) horiz2d(mScissorInfo.mPos), viewport[3]-((int) vert2d(mScissorInfo.mPos)+(int) vert2d(mScissorInfo.mSize)), (int) horiz2d(mScissorInfo.mSize), (int) vert2d(mScissorInfo.mSize));
}

void PopScissorNode::DisableScissor(Renderer& inRenderer)
{
	glDisable(GL_SCISSOR_TEST);
}

}