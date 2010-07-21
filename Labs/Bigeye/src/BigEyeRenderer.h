#ifndef _INCLUDED_BIGEYE_RENDERER_H
#define _INCLUDED_BIGEYE_RENDERER_H

#include <vector>
#include <queue>
#include <map>
#include <algorithm>
#include "Assert.h"
#include "OGL.h"

namespace BE
{
	class Renderer;

	class CompactRenderState
	{
	public:

		union
		{
			struct
			{
				unsigned enable_depth : 1;
				unsigned enable_texture : 1;
				unsigned enable_blend : 1;
				unsigned enable_scissor_test : 1;
			};

			struct
			{
				unsigned __int32 All;
			};
		};

		CompactRenderState() : All(0) {}

		inline bool operator==(const CompactRenderState& inComp) const { return All == inComp.All; }
		inline bool operator!=(const CompactRenderState& inComp) const { return All != inComp.All; }
		inline CompactRenderState& operator=(const CompactRenderState& inRef) { All = inRef.All; }

		void Apply(Renderer& renderer);
	};

	/*
	class RenderState
	{
	public:

		union
		{
			struct
			{
				unsigned enable_depth : 1;
				unsigned enable_texture : 1;
				unsigned enable_blend : 1;
				unsigned enable_scissor_test : 1;
			};

			struct
			{
				unsigned __int32 All;
			};
		};

		RenderState() : All(0) {}

		inline bool operator==(const RenderState& inComp) const { return All == inComp.All; }
		inline bool operator!=(const RenderState& inComp) const { return All != inComp.All; }
		inline RenderState& operator=(const RenderState& inRef) { All = inRef.All; }
	};

	class RenderSortHash
	{
		unsigned __int32 mKey[4];
	};

	class Renderer;

	class RenderItem
	{
	public:

		virtual ~RenderItem() {}

		virtual const RenderState& GetRenderState() = 0;
		virtual const RenderSortHash& GetRenderSortHash() = 0;

		virtual void Render(Renderer& inRenderer) {}
	};

	typedef std::vector<RenderItem*> RenderItemBatch;

	class RenderNode
	{
	public:

		virtual ~RenderNode() {}

		virtual void Render(Renderer& inRenderer) {}
	};

	class RenderTree
	{
	protected:

		struct RenderNodeInfo;

	public:

		void Clear()
		{
			mRenderNodeInfos.clear();
			mChildrenIndices.clear();
		}

		int AddNode(RenderNode& inNode, int inReservedChildCount = 0)
		{
			mRenderNodeInfos.push_back(RenderNodeInfo(inNode));

			if (inReservedChildCount > 0)
			{
				mRenderNodeInfos.back().mChildCount = inReservedChildCount;
				mRenderNodeInfos.back().mFreeChildCount = inReservedChildCount;
				mChildrenIndices.resize(mChildrenIndices.size() + inReservedChildCount);
			}
		}

		void SetAsChildNode(int inParentNode, int inChildNode)
		{
			RenderNodeInfo& parent_info = mRenderNodeInfos[inParentNode];
			RenderNodeInfo& child_info = mRenderNodeInfos[inChildNode];

			if (parent_info.mFreeChildCount == 0)
			{
				// TODO: Impl
				gAssert(false);
				return;
			}

			mChildrenIndices[parent_info.mFirstChildIndex + (parent_info.mChildCount - parent_info.mFreeChildCount)] = inChildNode;
			--parent_info.mFreeChildCount;
			child_info.mParent = inParentNode;
		}

		void Render(Renderer& inRenderer)
		{
			if (!mRenderNodeInfos.empty())
			{

				size_t rendered_node_count = 0;

				for (size_t i=(mRenderNodeInfos.size()-1); i<mRenderNodeInfos.size() && rendered_node_count<(int)mRenderNodeInfos.size(); --i)
				{
					if (mRenderNodeInfos[i].mParent == -1)
						rendered_node_count += RenderSubtree(inRenderer, mRenderNodeInfos[i]);
				}
			}
		}

	protected:


		size_t RenderSubtree(Renderer& inRenderer, RenderNodeInfo& inNode)
		{
			size_t render_count = 1;
			inNode.mNode->Render(inRenderer);

			for (int i=0; i<inNode.mChildCount-inNode.mFreeChildCount; ++i)
			{
				render_count += RenderSubtree(inRenderer, mRenderNodeInfos[mChildrenIndices[inNode.mFirstChildIndex+i]]);
			}

			return render_count;
		}

		struct RenderNodeInfo
		{
			RenderNode* mNode;
			int mFirstChildIndex;
			int mChildCount;
			int mFreeChildCount;
			int mParent;

			RenderNodeInfo()
			:	mNode(NULL)
			{
			}

			RenderNodeInfo(RenderNode& inNode)
			:	mNode(&inNode)
			,	mFirstChildIndex(-1)
			,	mChildCount(0)
			,	mFreeChildCount(0)
			,	mParent(-1)
			{
			}
		};

		typedef std::vector<RenderNodeInfo> RenderNodeInfos;
		typedef std::vector<int> ChildrenIndices;

		RenderNodeInfos mRenderNodeInfos;
		ChildrenIndices mChildrenIndices;
	};

	class Renderer
	{
	public:

		RenderTree& NewTree() { mTree.Clear(); return mTree; }

		void Render()
		{
			mTree.Render(*this);
		}

		void Render(const RenderItemBatch& itemBatch)
		{
			for (size_t i=0; i<itemBatch.size(); ++i)
			{
				SetState(itemBatch[i]->GetRenderState());
				itemBatch[i]->Render(*this);
			}
		}


		void SetState(const RenderState& inState)
		{
			if (mCurrState != inState)
			{
				mCurrState = inState;
				ApplyState(mCurrState);
			}
		}

	protected:

		void ApplyState(const RenderState& inState)
		{
			if (inState.enable_depth)
				glEnable(GL_DEPTH_TEST);
			else
				glDisable(GL_DEPTH_TEST);

			if (inState.enable_texture)
				glEnable(GL_TEXTURE_2D);
			else
				glDisable(GL_TEXTURE_2D);

			if (inState.enable_blend)
				glEnable(GL_BLEND);
			else
				glDisable(GL_BLEND);

			if (inState.enable_scissor_test)
				glEnable(GL_SCISSOR_TEST);
			else
				glDisable(GL_SCISSOR_TEST);
		}

		RenderState mCurrState;
		RenderTree mTree;
	};



	class SimpleRenderNode : public RenderNode
	{
	public:

		virtual void Render(Renderer& inRenderer) 
		{
			inRenderer.Render(mRenderItemBatch);
		}

		RenderItemBatch mRenderItemBatch;
	};
	*/

	class RenderTree;
	class Renderer;

	class RenderNodeDependency
	{
	public:

		typedef std::vector<RenderTree*> Trees;
		Trees mTrees;
	};

	class RenderNode
	{
	public:

		virtual							~RenderNode()				{}

		virtual RenderNodeDependency*	GetDependency()					{ return NULL; }
		virtual void					Render(Renderer& inRenderer)	{}
	};
	

	class RenderTree
	{
	public:

		struct TreeNode;
		class TreeNodeArrayPool;

		RenderTree(TreeNodeArrayPool& inPool, bool inAutoDestroyRenderNodes)
		:	mArrayPool(&inPool)
		,	mNodeCount(1)
		,	mAutoDestroyRenderNodes(inAutoDestroyRenderNodes)
		{
		}

		TreeNode& AddNode(RenderNode& inNode)
		{
			return mRoot.AddChild(inNode, *mArrayPool);
		}

		TreeNode& AddNode(TreeNode& inParent, RenderNode& inNode)
		{
			return inParent.AddChild(inNode, *mArrayPool);
		}

		void Render(Renderer& inRenderer)
		{
			Render(inRenderer, mRoot);
		}

		void Render(Renderer& inRenderer, TreeNode& inTreeNode)
		{
			if (inTreeNode.mChildren != NULL)
			{
				for (size_t i=0; i<inTreeNode.mChildren->mNodes.size(); ++i)
				{
					Render(inRenderer, (inTreeNode.mChildren->mNodes[i]));
				}
			}

			if (inTreeNode.mNode != NULL)
				inTreeNode.mNode->Render(inRenderer);
		}

		class DependencyContext
		{
		friend class RenderTree;
		public:

			DependencyContext(RenderTree& inTree) 
			{
				mWalkQueue.push(&inTree.mRoot);
			}

		protected:

			std::queue<TreeNode*> mWalkQueue;
		};

		RenderNodeDependency* NextDependency(DependencyContext& inContext)
		{
			if (inContext.mWalkQueue.empty())
				return NULL;

			TreeNode* node = inContext.mWalkQueue.back();
			inContext.mWalkQueue.pop();

			while (node != NULL
					&& (node->mNode == NULL || node->mNode->GetDependency() == NULL)
					)
			{
				if (node->mChildren != NULL)
				{
					for (size_t i=0; i<node->mChildren->mNodes.size(); ++i)
					{
						inContext.mWalkQueue.push(&node->mChildren->mNodes[i]);
					}
				}

				if (!inContext.mWalkQueue.empty())
				{
					node = inContext.mWalkQueue.front();
					inContext.mWalkQueue.pop();
				}
				else
				{
					node = NULL;
				}
			}

			if (node != NULL)
				return node->mNode->GetDependency();

			return NULL;
		}
		
		~RenderTree()
		{
			mRoot.Destroy(*mArrayPool, mAutoDestroyRenderNodes);
		}

	public:

		class TreeNodeArray;

		struct TreeNode
		{
			RenderNode* mNode;
			TreeNodeArray* mChildren;

			TreeNode()
			:	mNode(NULL)
			,	mChildren(NULL)
			{
			}

			TreeNode(RenderNode& inNode)
			:	mNode(&inNode)
			,	mChildren(NULL)
			{
			}

			void Destroy(TreeNodeArrayPool& inArrayPool, bool inAutoDestroyRenderNodes)
			{
				if (mChildren != NULL)
				{
					for (size_t i=0; i<mChildren->mNodes.size(); ++i)
						mChildren->mNodes[i].Destroy(inArrayPool, inAutoDestroyRenderNodes);

					inArrayPool.Put(*mChildren);
				}

				if (inAutoDestroyRenderNodes)
					delete mNode;

				mChildren = NULL;
				mNode = NULL;
			}

			~TreeNode()
			{
				// We need this for copied nodes
				//gAssert(mNode == NULL);
				//gAssert(mChildren == NULL);
			}

			TreeNode& AddChild(RenderNode& inNode, TreeNodeArrayPool& inArrayPool)
			{
				if (mChildren == NULL)
					mChildren = &inArrayPool.Get();

				return mChildren->Add(inNode);
			}
		};

		class TreeNodeArray
		{
		public:

			typedef std::vector<TreeNode> Nodes;

			TreeNode& Add(RenderNode& inNode)
			{
				mNodes.push_back(TreeNode(inNode));
				return mNodes.back();
			}

			Nodes mNodes;
		};

		class TreeNodeArrayPool
		{
		public:

			TreeNodeArray& Get()
			{
				if (mArrays.empty())
				{
					return *(new TreeNodeArray());
				}
				else
				{
					TreeNodeArray* ret = mArrays.back();
					mArrays.pop_back();
					return *ret;
				}
			}

			void Put(TreeNodeArray& inArray)
			{
				inArray.mNodes.clear();
				mArrays.push_back(&inArray);
			}

		protected:

			typedef std::vector<TreeNodeArray*> Arrays;

			Arrays mArrays;
		};

		

		TreeNodeArrayPool* mArrayPool;
		bool mAutoDestroyRenderNodes;
		TreeNode mRoot;
		int mNodeCount;
	};

	class Renderer
	{
	public:

		Renderer()
		{
			mIsValidCompactRenderState = false;
		}

		RenderTree& NewTree(bool inAutoDestroyRenderNodes)
		{
			RenderTree* new_tree = new RenderTree(mTreeNodeArrayPool, inAutoDestroyRenderNodes);
			mRenderTrees.push_back(new_tree);

			return *new_tree;
		}

		void Render()
		{
			mIsValidCompactRenderState = false;

			OrderTrees();

			for (size_t i=0; i<mOrderedRenderTrees.size(); ++i)
			{
				mOrderedRenderTrees[i]->Render(*this);
			}
		}

		bool HasCurrentCompactRenderState()
		{
			return mIsValidCompactRenderState;
		}

		CompactRenderState& GetCurrentCompactRenderState()
		{
			return mCompactRenderState;
		}

		bool SetCurrentCompactRenderState(const CompactRenderState& inCompactRenderState)
		{
			mCompactRenderState =  inCompactRenderState;
		}

		void InvalidateCurrentCompactRenderState()
		{
			mIsValidCompactRenderState = false;
		}

	protected:

		struct OrderConstraint
		{
			RenderTree* mFirst;
			RenderTree* mSecond;
			int mChainIndex;

			OrderConstraint()
			:	mFirst(NULL)
			,	mSecond(NULL)
			,	mChainIndex(-1)
			{
			}

			OrderConstraint(RenderTree* inFirst, RenderTree* inSecond)
			:	mFirst(inFirst)
			,	mSecond(inSecond)
			,	mChainIndex(-1)
			{
			}
		};

		struct RenderTreeOrdering
		{
			size_t mRenderTreeIndex;
			int mOrderIndex;

			bool operator<(const RenderTreeOrdering& inComp) const
			{
				return mOrderIndex < inComp.mOrderIndex;
			}
		};

		typedef std::vector<OrderConstraint> OrderConstraints;
		typedef std::vector<size_t> OrderConstraintChain;
		typedef std::vector<OrderConstraintChain> OrderConstraintChains;
		typedef std::vector<RenderTreeOrdering> RenderTreeOrderings;

		void OrderTrees()
		{
			mOrderedRenderTrees.clear();

			// Gather all constraints
			OrderConstraints all_constrains;
			{
				for (size_t i=0; i<mRenderTrees.size(); ++i)
				{
					RenderTree::DependencyContext context(*mRenderTrees[i]);
					RenderNodeDependency* dependency; 

					while ((dependency = mRenderTrees[i]->NextDependency(context)) != NULL)
					{
						for (size_t j=0; j<dependency->mTrees.size(); ++j)
						{
							all_constrains.push_back(OrderConstraint(dependency->mTrees[j], mRenderTrees[i]));
						}
					}
				}
			}

			// Tie into chains
			OrderConstraintChains chains;
			{
				size_t unchained_constraint_count = all_constrains.size();
				while (unchained_constraint_count != 0)
				{
					size_t first_constraint_index;

					for (size_t i=0; i<all_constrains.size(); ++i)
					{
						if (all_constrains[i].mChainIndex == -1)
						{
							first_constraint_index = i;
							break;
						}
					}

					chains.push_back(OrderConstraintChain());
					int chain_index = (int) chains.size();
					OrderConstraintChain& chain = chains.back();
					
					chain.push_back(first_constraint_index);
					all_constrains[first_constraint_index].mChainIndex = chain_index;
					--unchained_constraint_count;

					int last_run_unchained_constraint_count = -1;

					while (unchained_constraint_count != last_run_unchained_constraint_count)
					{
						last_run_unchained_constraint_count = unchained_constraint_count;

						RenderTree* extremity_left = all_constrains[chain.front()].mFirst;
						RenderTree* extremity_right = all_constrains[chain.front()].mSecond;

						for (size_t i=0; i<all_constrains.size() && unchained_constraint_count != 0; ++i)
						{
							if (all_constrains[i].mChainIndex == -1)
							{
								if (all_constrains[i].mSecond == extremity_left)
								{
									chain.insert(chain.begin(), i);
									all_constrains[i].mChainIndex = chain_index;
									--unchained_constraint_count;
									extremity_left = all_constrains[chain.front()].mFirst;

								} 
								else if (all_constrains[i].mFirst == extremity_right)
								{
									chain.push_back(i);
									all_constrains[i].mChainIndex = chain_index;
									--unchained_constraint_count;
									extremity_right = all_constrains[chain.front()].mSecond;
								}
							}
						}
					}
				}
			}

			//Check chains for conflicts
			{

				//1. for each chain check if any rendertree is repeated more than once.
				//2. for each pair of constraints in the chain (even if mor than a step away from each other, check if they contradict other chains)
			}

			//
			{
				//1. set all tree order to -1
				//2. loop thru all chains, with a counter and set it on the tree if not already set, 
				

				RenderTreeOrderings orderings;
				orderings.resize(mRenderTrees.size());

				for (size_t i=0; i<orderings.size(); ++i)
				{
					orderings[i].mOrderIndex = -1;
					orderings[i].mRenderTreeIndex = i;
				}

				typedef std::map<RenderTree*, size_t> TreeToIndexMap;
				TreeToIndexMap treeToIndexMap;

				for (size_t i=0; i<mRenderTrees.size(); ++i)
				{
					treeToIndexMap[mRenderTrees[i]] = i;
				}


				int global_ordering_counter = 0;

				for (size_t i=0; i<chains.size(); ++i)
				{
					OrderConstraintChain& chain = chains[i];

					for (size_t j=0; j<chain.size(); ++j)
					{
						size_t first_tree_index = treeToIndexMap[all_constrains[chain[j]].mFirst];
						size_t second_tree_index = treeToIndexMap[all_constrains[chain[j]].mSecond];

						if (orderings[first_tree_index].mOrderIndex == -1)
						{
							orderings[first_tree_index].mOrderIndex = global_ordering_counter++;
						}

						if (orderings[second_tree_index].mOrderIndex == -1)
						{
							orderings[second_tree_index].mOrderIndex = global_ordering_counter++;
						}
					}
				}

				std::sort(orderings.begin(), orderings.end());

				mOrderedRenderTrees.resize(orderings.size());
				for (size_t i=0; i<mOrderedRenderTrees.size(); ++i)
				{
					mOrderedRenderTrees[i] = mRenderTrees[orderings[i].mRenderTreeIndex];
				}
			}
		}

	protected:

		typedef std::vector<RenderTree*> RenderTrees;

		RenderTree::TreeNodeArrayPool mTreeNodeArrayPool;
		RenderTrees mRenderTrees;
		RenderTrees mOrderedRenderTrees;

		bool mIsValidCompactRenderState;
		CompactRenderState mCompactRenderState;
	};


	void gTestRenderer();
}

#endif
