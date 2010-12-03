#if 0

#ifndef _RayTracer_BoundingVolumeHierarchy2_hpp
#define _RayTracer_BoundingVolumeHierarchy2_hpp

#include "Scene.hpp"
#include "../WE3/math/WEPlane.h"
#include "Scene.hpp"
#include "Renderer.hpp"
#include "SpatialAccelerator.hpp"
#include <limits>

namespace rayTracer {

	class BoundingVolumeHierarchy2 : public SpatialAccelerator {
	public:

		virtual void destroy();
		virtual void build(Scene& scene, int maxLeafPrimitiveCount, float planeThickness);
		virtual bool shootRay(const RayShot& rayShot, RayShotHit& result);
		virtual unsigned int debugShootRay(const RayShot& rayShot);

		#ifdef RayTracerConfig_EnablePackets
			virtual bool shootRayPacket(const Constants& cts, const PacketRayShot& rayPacket, PacketRayHit& t);
		#endif

	protected:

		struct BVHTriangle {

			// first 16 byte half cache line
			// plane:
			float n_u; //!< == normal.u / normal.k
			float n_v; //!< == normal.v / normal.k
			float n_d; //!< constant of plane equation
			int k; // projection dimension
			// second 16 byte half cache line
			// line equation for line ac
			float b_nu;
			float b_nv;
			float b_d;
			
			#ifdef RayTracerConfig_TryAlignCahe
			int pad1; // pad to next cache line
			#endif

			// third 16 byte half cache line
			// line equation for line ab
			float c_nu;
			float c_nv;
			float c_d;
			
			unsigned int primID;

			void init(const unsigned int primIndex, Primitive_Triangle& tribvhTri);
			bool intersects(const RayShot& rayShot, float& inOutT) const;
			
			#ifdef RayTracerConfig_EnablePackets
				void intersect(const Constants& cts, const PacketRayShot& rayPacket, PacketRayHit& t, PacketRayMask& rayPacketMask) const;
			#endif
		};

	protected:

		struct Node;

		typedef WETL::CountedArray<Primitive*, false, size_t, WETL::ResizeExact> LeafPrimitives;
		typedef WETL::SizeAllocT<BVHTriangle, unsigned int, true> BVHTriangles;
		typedef WETL::SizeAllocT<Node, unsigned int, true> Nodes;

		struct BuildLeaf {

			unsigned int first;
			unsigned int count;
		};

		struct BuildNode {

			#ifdef RayTracerConfig_BIHNode
				AAB gridVolume;
				int clipDim;
			#endif
			
			AAB volume;
			
			SoftPtr<BuildNode> leftChild;
			SoftPtr<BuildNode> rightChild;

			HardPtr<BuildLeaf> leaf;

			BuildNode();

			void recurseDestroy();
			void deleteSelf();

			bool findBestSplitPlane(Scene& scene, BVHTriangles& triangles, AAPlane& splitPlane, int& hintDim, float planeThikness);
			void split(Scene& scene, BVHTriangles& triangles, BVHTriangles& tempTriangles, int maxLeafPrimitiveCount, int& hintDim, float planeThickness);

			#ifdef RayTracerConfig_BIHNode
				bool findBestSplitPlaneBIH(Scene& scene, BVHTriangles& triangles, AAPlane& splitPlane, int& hintDim, float planeThikness);
				void splitBIH(Scene& scene, BVHTriangles& triangles, BVHTriangles& tempTriangles, int maxLeafPrimitiveCount, int& hintDim, float planeThickness);
			#endif

			void convertTo(Node& node, bool leftRight);
			unsigned int recurseFlatten(Nodes& nodes, unsigned int& runningIndex, bool leftRight = true);
			unsigned int recurseFlatten2(Nodes& nodes, unsigned int& runningIndex, unsigned int thisIndex = 0, bool leftRight = true);

			void recurseCount(unsigned int& minDepth, unsigned int& maxDepth, unsigned int& leafCount, unsigned int& nodeCount);
		};

		#ifdef RayTracerConfig_BIHNode
			struct BIHStack1;
			typedef BIHStack1 Stack1; 
		#else
			struct Stack1;
		#endif
		
		struct PacketStack1;
		struct PacketStack2;
		//typedef PacketStack1 PacketStack;
		typedef PacketStack2 PacketStack;

		struct Node {

			#ifdef RayTracerConfig_BIHNode

				int dim;
				float clip[2];

			#else

				AAB volume;

			#endif
						
			unsigned int first;

			union {
			
				struct {
					unsigned int leftChild;
					unsigned int rightChild;
				};

				struct {
					unsigned int count;
				};
			};

			#ifdef RayTracerConfig_BIHNode
			#else

				#ifdef RayTracerConfig_TryAlignCahe
					unsigned int _pad1;
				#endif

			#endif

			Node();

			inline bool hasLeaf() const { return first != -1; }
			inline bool hasLeftChild() const { return leftChild != -1; }
			inline bool hasRightChild() const { return rightChild != -1; }
			inline Node& getLeftChild(Nodes& nodes) const { return nodes[leftChild]; }
			inline Node& getRightChild(Nodes& nodes) const { return nodes[rightChild]; }

			static inline bool intersect(const BVHTriangle& tri, const RayShot& rayShot, RayShotHit& result) {

				if (tri.intersects(rayShot, result.inters.t)) {

					result.primID = tri.primID;
					return true;
				}

				return false;
			}

			#ifdef RayTracerConfig_BIHNode
				static void shootRay3(BoundingVolumeHierarchy2& hier, Node& startNode, const AAB& startVolume, Stack1& stack, const RayShot& rayShot, RayShotHit& result, float minT);
			#else
				static void shootRay3(BoundingVolumeHierarchy2& hier, Node& startNode, Stack1& stack, const RayShot& rayShot, RayShotHit& result, float minT);
				static unsigned int debugShootRay3(BoundingVolumeHierarchy2& hier, Node& startNode, Stack1& stack, const RayShot& rayShot);
			#endif

			#ifdef RayTracerConfig_EnablePackets
				static void shootRayPacketIterative(BoundingVolumeHierarchy2& hier, Node& startNode, PacketStack& stack, const Constants& cts, const PacketRayShot& rayPacket, PacketRayHit& t, PacketRayMask& rayPacketState, const Packet4& inTmin);
				bool intersects(const Constants& cts, const PacketRayShot& rayPacket, PacketRayMask& rayPacketMask, Packet4& tmin, const bool forceFullTest) const;

				static inline void intersect(const BVHTriangle& tri, const Constants& cts, const PacketRayShot& rayPacket, PacketRayHit& t, PacketRayMask& rayPacketMask) {

					tri.intersect(cts, rayPacket, t, rayPacketMask);
				}

			#endif

			#ifdef RayTracerConfig_BIHNode
				bool intersects2(const AAB& volume, const Ray& ray, float& outTMin) const;
			#else
				bool intersects2(const Ray& ray, float& outTMin) const;
			#endif

			
		};

		#ifdef RayTracerConfig_BIHNode

			struct BIHStack1 {

				struct Element {

					AAB volume;
					SoftPtr<Node> node;
				};

				void create(unsigned int size) {

					mElements.resize(size);
					mCount = 0;
				}

				void grow(bool warn) {

					unsigned int newSize = mElements.size + mElements.size + 2;

					if (warn) {

						printf("Warning: Traversal Stack Grown from %u to %u\n", mElements.size, newSize);
					}

					mElements.resize(newSize);
				}

				inline void empty() {

					mCount = 0;
				}

				inline Node* popNode(AAB& volume) {

					if (mCount) {

						Element& el = mElements[--mCount];

						volume = el.volume;

						return el.node;
					}

					return NULL;
				}

				inline void pushNode(Node* pNode, const AAB& volume) {

					if (mElements.size == mCount) {

						grow(true);
					}

					Element& el = mElements[mCount++];
					el.volume = volume;
					el.node = pNode;
				}

				inline void pushNode(Node* pNode, const AAB& volume, int minMax, int clipDim, float clipVal) {

					if (mElements.size == mCount) {

						grow(true);
					}

					Element& el = mElements[mCount++];
					el.volume = volume;
					el.volume.parameters[minMax][clipDim] = clipVal;
					el.node = pNode;
				}

				typedef WETL::SizeAllocT<Element, unsigned int> Elements;
				Elements mElements;
				unsigned int mCount;
			};

		#else

			struct Stack1 {

				void create(unsigned int size) {

					mNodes.resize(size);
					mCount = 0;
				}

				void grow(bool warn) {

					unsigned int newSize = mNodes.size + mNodes.size + 2;

					if (warn) {

						printf("Warning: Traversal Stack Grown from %u to %u\n", mNodes.size, newSize);
					}

					mNodes.resize(newSize);
				}

				inline void empty() {

					mCount = 0;
				}

				inline Node* popNode() {

					return mCount ? mNodes[--mCount] : NULL;
				}

				inline void pushNode(Node* pNode) {

					if (mNodes.size != mCount) {

						mNodes[mCount++] = pNode;
						
					} else {
				
						grow(true);
						
						mNodes[mCount++] = pNode;
					}
				}

				typedef WETL::SizeAllocT<Node*, unsigned int> Nodes;
				Nodes mNodes;
				unsigned int mCount;
			};

		#endif

		

		#ifdef RayTracerConfig_EnablePackets

		__declspec(align(16))
		struct PacketStack1 {

			struct Element {

				union {

					struct {
						Packet4 packetMask;
						SoftPtr<Node> node;
					};
				};
			};

			void create(unsigned int size) {

				mElements.resize(size, 16);
				mCount = 0;
			}

			~PacketStack1() {

				mElements.destroy(true);
			}

			void grow(bool warn) {

				unsigned int newSize = mElements.size + mElements.size + 2;

				if (warn) {

					printf("Warning: Traversal Stack Grown from %u to %u\n", mElements.size, newSize);
				}

				mElements.resize(newSize);
			}

			inline void empty() {

				mCount = 0;
			}

			inline bool hasElements() { return mCount > 0; }

			inline Node* popNode(PacketRayMask& mask) {

				if (mCount) {

					Element& el = mElements[--mCount];
					mask.active.el4 = el.packetMask.el4;

					return el.node;
				}

				return NULL;
			}

			 inline void pushNode(Node* pNode, PacketRayMask& mask) {

				if (mElements.size <= mCount) {

					grow(true);
				}

				Element& el = mElements[mCount++];
				el.node = pNode;
				el.packetMask.el4 = mask.active.el4;
			}

			typedef WETL::SizeAllocT<Element, unsigned int> Elements;
			Elements mElements;
			unsigned int mCount;
		};

		struct PacketStack2 {

			struct Element {

				SoftPtr<Node> node;
				int packetMask;
			};

			void create(unsigned int size) {

				mElements.resize(size);
				mCount = 0;
			}

			void grow(bool warn) {

				unsigned int newSize = mElements.size + mElements.size + 2;

				if (warn) {

					printf("Warning: Traversal Stack Grown from %u to %u\n", mElements.size, newSize);
				}

				mElements.resize(newSize);
			}

			inline void empty() {

				mCount = 0;
			}

			 inline Node* popNode(PacketRayMask& mask) {

				if (mCount) {

					Element& el = mElements[--mCount];
					mask.setActiveMask(el.packetMask);
					
					return el.node;
				}

				return NULL;
			}

			 inline void pushNode(Node* pNode, PacketRayMask& mask) {

				if (mElements.size != mCount) {

					Element& el = mElements[mCount++];
					el.node = pNode;
					el.packetMask = mask.extractMask();
					
				} else {
			
					grow(true);
					
					Element& el = mElements[mCount++];
					el.node = pNode;
					el.packetMask = mask.extractMask();
				}
			}

			typedef WETL::SizeAllocT<Element, unsigned int> Elements;
			Elements mElements;
			unsigned int mCount;
		};

		#endif

		SoftPtr<Scene> mScene;
		SoftPtr<Node> mRootNode;
		#ifdef RayTracerConfig_BIHNode
			AAB mRootVolume;
		#endif

		Nodes mNodes;
		BVHTriangles mTriangles;
		
		unsigned int mMinDepth;
		unsigned int mMaxDepth;
		unsigned int mLeafCount;
		unsigned int mNodeCount;

		Stack1 mTraversalStack;

		#ifdef RayTracerConfig_EnablePackets
			PacketStack mPacketTraversalStack;
		#endif

		#ifdef RayTracerConfig_BIHNode
			BIHStack1 mTraversalStackBIH;
		#endif
	};

	typedef BoundingVolumeHierarchy2 BVH;
}

#endif

#endif