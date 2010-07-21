#include "BigEyeRenderer.h"

namespace BE
{

class TestDependentRenderNode : public RenderNode
{
public:

	virtual RenderNodeDependency*	GetDependency()					{ return &mDependency; }

	RenderNodeDependency mDependency;
};


void gTestRenderer()
{

	Renderer renderer;

	RenderTree& tree1 = renderer.NewTree();
	RenderTree& tree2 = renderer.NewTree();
	RenderTree& tree3 = renderer.NewTree();

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