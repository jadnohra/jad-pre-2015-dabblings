#ifndef _INCLUDED_BIGEYE_BIGEYERENDERTREEBUILDER_H
#define _INCLUDED_BIGEYE_BIGEYERENDERTREEBUILDER_H

#include <stack>
#include "BigeyeRenderer.h"
#include "BigeyeRenderNodes.h"

namespace BE 
{

	class RenderTreeBuilder
	{
	public:

		RenderTreeBuilder()
		:	mRenderer(NULL)
		{
		}

		void Reset(Renderer& inRenderer)
		{
			mRenderer = &inRenderer;

			for (size_t i=0; i<mTreeBuildStates.size(); ++i)
			{
				delete mTreeBuildStates[i];
			}

			mTreeBuildStates.clear();
		}

		void BranchUp(int inTreeIndex, RenderNode& inNode)
		{
			TreeBuildState* tree_state = mTreeBuildStates[inTreeIndex];
			RenderTree::TreeNode* tree_node;

			if (!tree_state->mTreeNodeStack.empty())
				tree_node = &tree_state->mTree->AddNode(*tree_state->mTreeNodeStack.top(), inNode);
			else
				tree_node = &tree_state->mTree->AddNode(inNode);

			tree_state->mTreeNodeStack.push(tree_node);
			
			return;
		}

		void BranchDown(int inTreeIndex)
		{
			TreeBuildState* tree_state = mTreeBuildStates[inTreeIndex];
			
			if (!tree_state->mTreeNodeStack.empty())
				tree_state->mTreeNodeStack.pop();
		}

		int BranchUpNewTree(RenderNode* inNode, bool inSetDependency, bool inAutoDestroyRenderNodes)
		{
			TreeBuildState* new_tree_state = new TreeBuildState();
			new_tree_state->mTree = &mRenderer->NewTree(inAutoDestroyRenderNodes);

			if (inNode)
			{
				new_tree_state->mTreeNodeStack.push(&new_tree_state->mTree->AddNode(*inNode));

				if (inSetDependency)
				{
					RenderNodeDependency* dependency = inNode->GetDependency();
					gAssert(dependency != NULL);

					dependency->mTrees.push_back(new_tree_state->mTree);
				}
			}

			mTreeBuildStates.push_back(new_tree_state);

			return ((int) mTreeBuildStates.size() - 1);
		}

		PushPopNodeStack& GetScissorPushPopNodeStack(int inTreeIndex)
		{
			return mTreeBuildStates[inTreeIndex]->mScissorStack;
		}

	private:

		class TreeBuildState
		{
		public:

			typedef std::stack<RenderTree::TreeNode*> TreeNodeStack;

			RenderTree*			mTree;
			TreeNodeStack		mTreeNodeStack;
			PushPopNodeStack	mScissorStack;
		};

		typedef std::vector<TreeBuildState*> TreeBuildStates;

		Renderer* mRenderer;
		TreeBuildStates mTreeBuildStates;
	};
}

#endif