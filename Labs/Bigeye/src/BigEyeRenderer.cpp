#include "BigEyeRenderer.h"

namespace BE
{

void CompactRenderState::Apply(Renderer& renderer)
{
	if (renderer.HasCurrentCompactRenderState()
		&& renderer.GetCurrentCompactRenderState() == *this)
		return;

	if (enable_depth)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);

	if (enable_texture)
		glEnable(GL_TEXTURE_2D);
	else
		glDisable(GL_TEXTURE_2D);

	if (enable_blend)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);

	if (enable_scissor_test)
		glEnable(GL_SCISSOR_TEST);
	else
		glDisable(GL_SCISSOR_TEST);

	renderer.SetCurrentCompactRenderState(*this);
}


class TestDependentRenderNode : public RenderNode
{
public:

	virtual RenderNodeDependency*	GetDependency()					{ return &mDependency; }

	RenderNodeDependency mDependency;
};


void gTestRenderer()
{

	Renderer renderer;

	RenderTree& tree1 = renderer.NewTree(true);
	RenderTree& tree2 = renderer.NewTree(true);
	RenderTree& tree3 = renderer.NewTree(true);

	{
		RenderNode* node1 = new RenderNode();
		RenderTree::TreeNode& tree_node1 = tree1.AddNode(*node1);
		TestDependentRenderNode* node2 = new TestDependentRenderNode();
		node2->mDependency.mTrees.push_back(&tree2);
		RenderTree::TreeNode& tree_node2 = tree1.AddNode(tree_node1, *node2);
		RenderNode* node3 = new RenderNode();
		RenderTree::TreeNode& tree_node3 = tree1.AddNode(tree_node1, *node3);
	}

	{
		RenderNode* node1 = new RenderNode();
		RenderTree::TreeNode& tree_node1 = tree2.AddNode(*node1);
	}

	{
		TestDependentRenderNode* node1 = new TestDependentRenderNode();
		node1->mDependency.mTrees.push_back(&tree2);
		RenderTree::TreeNode& tree_node1 = tree3.AddNode(*node1);
	}

	renderer.Render();
}

}