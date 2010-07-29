#ifndef _INCLUDED_BIGEYE_RENDERNODES_H
#define _INCLUDED_BIGEYE_RENDERNODES_H

#include <stack>
#include "BEMath.h"
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

	class PushPopNode : public RenderNode
	{
	public:

	};

	class PushPopNodeStack
	{
	public:

		typedef std::stack<PushPopNode*> NodeStack;
		NodeStack mStack;
	};


	class PushScissorNode : public PushPopNode
	{
	public:

		struct ScissorInfo
		{
			glm::vec2 mPos;
			glm::vec2 mSize;

			ScissorInfo() {}
			ScissorInfo(const glm::vec2& inPos, const glm::vec2& inSize) : mPos(inPos), mSize(inSize) {}
		};

		PushScissorNode(PushPopNodeStack& inPushPopStack, const glm::vec2& inPos, const glm::vec2& inSize)
		:	mPushPopStack(&inPushPopStack)
		,	mScissorInfo(inPos, inSize)
		{
		}

		PushScissorNode()
		:	mPushPopStack(NULL)
		{
		}

		virtual void Render(Renderer& inRenderer)	
		{
			Set(inRenderer);
			mPushPopStack->mStack.push(this);
		}

		void Set(Renderer& inRenderer);

		PushPopNodeStack* mPushPopStack;
		ScissorInfo mScissorInfo;
	};


	class PopScissorNode : public PushPopNode
	{
	public:

		PopScissorNode(PushPopNodeStack& inPushPopStack)
		:	mPushPopStack(&inPushPopStack)
		{
		}

		PopScissorNode()
		:	mPushPopStack(NULL)
		{
		}

		virtual void Render(Renderer& inRenderer)	
		{
			mPushPopStack->mStack.pop();

			if (!mPushPopStack->mStack.empty())
			{
				PushScissorNode* prev_push_node = (PushScissorNode*) mPushPopStack->mStack.top();
				prev_push_node->Set(inRenderer);
			}
			else
			{
				DisableScissor(inRenderer);
			}
		}

		void DisableScissor(Renderer& inRenderer);

		PushPopNodeStack* mPushPopStack;
	};

}

#endif