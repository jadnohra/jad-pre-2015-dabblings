#ifndef _INCLUDED_BIGEYE_RENDERNODES_H
#define _INCLUDED_BIGEYE_RENDERNODES_H

#include "BigeyeRenderer.h"

namespace BE
{
	
	class CompactRenderStateNode : public RenderNode
	{
	public:

		virtual void Render(Renderer& inRenderer)	
		{
			mRenderState.Apply(inRenderer);
		}

		CompactRenderState mRenderState;
	};
}

#endif