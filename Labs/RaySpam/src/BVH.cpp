#if 0

#include "BoundingVolumeHierarchy2.hpp"

namespace rayTracer {

BoundingVolumeHierarchy2::BuildNode::BuildNode() {

	volume.empty();
}

void BoundingVolumeHierarchy2::BuildNode::recurseDestroy() {

	if (leftChild.isValid())
		leftChild->recurseDestroy();

	if (rightChild.isValid())
		rightChild->recurseDestroy();
	
	deleteSelf();
}

void BoundingVolumeHierarchy2::BuildNode::deleteSelf() {

	delete this;
}

void BoundingVolumeHierarchy2::BuildNode::convertTo(Node& node, bool leftRight) {

	#ifdef RayTracerConfig_BIHNode
		node.dim = clipDim;
	#else
		node.volume = volume;
	#endif

	if (leaf.isValid()) {

		node.first = leaf->first;
		node.count = leaf->count;

	} else {

		node.first = -1;
	}
}

unsigned int BoundingVolumeHierarchy2::BuildNode::recurseFlatten(Nodes& nodes, unsigned int& runningIndex, bool leftRight) {

	unsigned int currIndex = runningIndex;

	Node& selfNode = nodes[runningIndex++];
	convertTo(selfNode, leftRight);

	unsigned int leftIndex = -1;
	unsigned int rightIndex = -1;


	if (leftChild.isValid()) 
		leftIndex = leftChild->recurseFlatten(nodes, runningIndex, true);

	if (rightChild.isValid()) 
		rightIndex = rightChild->recurseFlatten(nodes, runningIndex, false);

	#ifdef RayTracerConfig_BIHNode
	if (leftChild.isValid()) {

		selfNode.clip[0] = leftChild->volume.max[clipDim];
	}
	if (rightChild.isValid()) {

		selfNode.clip[1] = rightChild->volume.min[clipDim];
	}
	#endif

	if (!leaf.isValid()) {

		selfNode.leftChild = leftIndex;
		selfNode.rightChild = rightIndex;
	}

	return currIndex;
}

unsigned int BoundingVolumeHierarchy2::BuildNode::recurseFlatten2(Nodes& nodes, unsigned int& runningIndex, unsigned int thisIndex, bool leftRight) {

	Node& selfNode = nodes[thisIndex];
	convertTo(selfNode, leftRight);

	unsigned int leftIndex = -1;
	unsigned int rightIndex = -1;

	if (leftChild.isValid()) 
		leftIndex = (runningIndex += 1);

	if (rightChild.isValid()) 
		rightIndex = (runningIndex += 1);

	if (leftChild.isValid()) 
		leftIndex = leftChild->recurseFlatten2(nodes, runningIndex, leftIndex, true);

	if (rightChild.isValid()) 
		rightIndex = rightChild->recurseFlatten2(nodes, runningIndex, rightIndex, false);

	#ifdef RayTracerConfig_BIHNode
	if (leftChild.isValid()) {

		selfNode.clip[0] = leftChild->volume.max[clipDim];
	}
	if (rightChild.isValid()) {

		selfNode.clip[1] = rightChild->volume.min[clipDim];
	}
	#endif

	if (!leaf.isValid()) {

		selfNode.leftChild = leftIndex;
		selfNode.rightChild = rightIndex;
	}

	return thisIndex;
}

void BoundingVolumeHierarchy2::BuildNode::recurseCount(unsigned int& minDepth, unsigned int& maxDepth, unsigned int& leafCount, unsigned int& nodeCount) {

	++nodeCount;
	++maxDepth;

	if (leaf.isValid()) {

		++leafCount;
	}

	unsigned int maxLeftDepth = maxDepth;
	unsigned int maxRightDepth = maxDepth;
	unsigned int minLeftDepth = maxDepth;
	unsigned int minRightDepth = maxDepth;

	if (leftChild.isValid())
		leftChild->recurseCount(minLeftDepth, maxLeftDepth, leafCount, nodeCount);

	if (rightChild.isValid())
		rightChild->recurseCount(minRightDepth, maxRightDepth, leafCount, nodeCount);

	maxDepth = std::max(maxLeftDepth, maxRightDepth);
	minDepth = std::min(minLeftDepth, minRightDepth);
}

BoundingVolumeHierarchy2::Node::Node() {

	#ifdef RayTracerConfig_BIHNode
	#else
		volume.empty();
	#endif
}

void BoundingVolumeHierarchy2::destroy() {
}


bool BoundingVolumeHierarchy2::BuildNode::findBestSplitPlane(Scene& scene, BVHTriangles& triangles, AAPlane& splitPlane, int& hintDim, float planeThickness) {
	
	int chosenSplitDim = -1;
	float chosenSplitRatio = 2.0f;

	bool hasSolution = false;
	int chosenSplitLeftCount = 0;
	int chosenSplitRightCount = 0;
	int chosenSplitCommonCount = leaf->count + 1;
	AAPlane chosenSplitPlane;
	
	for (int splitDim = 0; splitDim < 3; ++splitDim)  {

		Vector3 center;
		Vector3 normal;

		volume.center(center);
		normal.zero();
		normal.el[splitDim] = 1.0f;

		splitPlane.init(center, normal, true);

		int leftCount = 0;
		int rightCount = 0;
		int commonCount = 0;

		for (int pi = 0; pi < leaf->count; ++pi) {

			Primitive& prim = dref(scene.primitives()[triangles[leaf->first + pi].primID]);

			switch (prim.classify(splitPlane, planeThickness)) {

				case PP_Back: ++leftCount; break;
				case PP_Front: ++rightCount; break;
				case PP_BackAndFront: ++commonCount; break;
				default: break;
			}
		}

		if (leftCount && rightCount && commonCount < chosenSplitCommonCount) {

			hasSolution = true;
			chosenSplitDim = splitDim;
			//chosenSplitRatio = splitRatio;

			chosenSplitPlane = splitPlane;

			chosenSplitCommonCount = commonCount;
			chosenSplitLeftCount = leftCount;
			chosenSplitRightCount = rightCount;
		}
	}

	hintDim = (chosenSplitDim + 1) % 3;
	splitPlane = chosenSplitPlane;

	return hasSolution;
}

void BoundingVolumeHierarchy2::BuildNode::split(Scene& scene, BVHTriangles& triangles, BVHTriangles& tempTriangles, int maxLeafPrimitiveCount, int& hintDim, float planeThickness) {

	if (leaf.isNull() || leaf->count <= maxLeafPrimitiveCount)
		return;

	AAPlane splitPlane;

	if (!findBestSplitPlane(scene, triangles, splitPlane, hintDim, planeThickness))
		return;

	memcpy(tempTriangles.el, triangles.el + leaf->first, leaf->count * sizeof(BVHTriangle));

	unsigned int leftIndex = leaf->first;
	unsigned int leftCount = 0;
	AAB leftVolume;
	leftVolume.empty();

	for (unsigned int i = 0; i < leaf->count; ++i) {

		Primitive_Triangle& prim = dref((Primitive_Triangle*) scene.primitives()[tempTriangles[i].primID]);

		if (prim.chooseClassify(splitPlane, planeThickness) == PP_Back) {

			prim.appendToVolume(leftVolume);
			++leftCount;
			triangles[leftIndex++] = tempTriangles[i];
		}
	}

	unsigned int rightIndex = leftIndex;
	unsigned int rightCount = 0;
	AAB rightVolume;
	rightVolume.empty();

	for (unsigned int i = 0; i < leaf->count; ++i) {

		Primitive_Triangle& prim = dref((Primitive_Triangle*) scene.primitives()[tempTriangles[i].primID]);

		if (prim.chooseClassify(splitPlane, planeThickness) == PP_Front) {

			prim.appendToVolume(rightVolume);
			++rightCount;
			triangles[rightIndex++] = tempTriangles[i];
		}
	}

	assert(rightIndex == leaf->first + leaf->count);

	#ifdef RayTracerConfig_BIHNode
		this->clipDim = splitPlane.normalDim;
	#endif

	if (leftCount) {

		leftChild = new BuildNode();
		leftChild->leaf = new BuildLeaf();
		leftChild->volume = leftVolume;

		leftChild->leaf->first = leaf->first;
		leftChild->leaf->count = leftCount;
	}

	if (rightCount) {

		rightChild = new BuildNode();
		rightChild->leaf = new BuildLeaf();
		rightChild->volume = rightVolume;

		rightChild->leaf->first = leaf->first + leftCount;
		rightChild->leaf->count = rightCount;
	}

	if (leftChild.isValid()) {

		leftChild->split(scene, triangles, tempTriangles, maxLeafPrimitiveCount, hintDim, planeThickness);
	}

	if (rightChild.isValid()) {

		rightChild->split(scene, triangles, tempTriangles, maxLeafPrimitiveCount, hintDim, planeThickness);
	}

	if (leftChild.isValid() || rightChild.isValid()) {

		leaf.destroy();
	}
}

#ifdef RayTracerConfig_BIHNode
bool BoundingVolumeHierarchy2::BuildNode::findBestSplitPlaneBIH(Scene& scene, BVHTriangles& triangles, AAPlane& splitPlane, int& hintDim, float planeThickness) {
	
	float maxSize;

	int splitDim = gridVolume.getMaxSizeIndex(maxSize);

	Vector3 center;
	Vector3 normal;

	gridVolume.center(center);
	normal.zero();
	normal.el[splitDim] = 1.0f;

	splitPlane.init(center, normal, true);

	return true;
}

void BoundingVolumeHierarchy2::BuildNode::splitBIH(Scene& scene, BVHTriangles& triangles, BVHTriangles& tempTriangles, int maxLeafPrimitiveCount, int& hintDim, float planeThickness) {

	if (leaf.isNull() || leaf->count <= maxLeafPrimitiveCount)
		return;

	AAPlane splitPlane;

	if (!findBestSplitPlaneBIH(scene, triangles, splitPlane, hintDim, planeThickness))
		return;

	memcpy(tempTriangles.el, triangles.el + leaf->first, leaf->count * sizeof(BVHTriangle));

	unsigned int leftIndex = leaf->first;
	unsigned int leftCount = 0;
	AAB leftVolume;
	leftVolume.empty();

	for (unsigned int i = 0; i < leaf->count; ++i) {

		Primitive_Triangle& prim = dref((Primitive_Triangle*) scene.primitives()[tempTriangles[i].primID]);

		if (prim.chooseClassify(splitPlane, planeThickness) == PP_Back) {

			prim.appendToVolume(leftVolume);
			++leftCount;
			triangles[leftIndex++] = tempTriangles[i];
		}
	}

	unsigned int rightIndex = leftIndex;
	unsigned int rightCount = 0;
	AAB rightVolume;
	rightVolume.empty();

	for (unsigned int i = 0; i < leaf->count; ++i) {

		Primitive_Triangle& prim = dref((Primitive_Triangle*) scene.primitives()[tempTriangles[i].primID]);

		if (prim.chooseClassify(splitPlane, planeThickness) == PP_Front) {

			prim.appendToVolume(rightVolume);
			++rightCount;
			triangles[rightIndex++] = tempTriangles[i];
		}
	}

	assert(rightIndex == leaf->first + leaf->count);

	if (leftCount) {

		leftChild = new BuildNode();
		leftChild->leaf = new BuildLeaf();
		leftChild->gridVolume = gridVolume;
		leftChild->volume = leftVolume;

		leftChild->gridVolume.max[splitPlane.normalDim] = splitPlane.position.el[splitPlane.normalDim] + planeThickness;
		

		leftChild->leaf->first = leaf->first;
		leftChild->leaf->count = leftCount;
	}

	if (rightCount) {

		rightChild = new BuildNode();
		rightChild->leaf = new BuildLeaf();
		rightChild->volume = rightVolume;

		leftChild->gridVolume.min[splitPlane.normalDim] = splitPlane.position.el[splitPlane.normalDim] - planeThickness;

		rightChild->leaf->first = leaf->first + leftCount;
		rightChild->leaf->count = rightCount;
	}

	if (leftChild.isValid()) {

		leftChild->split(scene, triangles, tempTriangles, maxLeafPrimitiveCount, hintDim, planeThickness);
	}

	if (rightChild.isValid()) {

		rightChild->split(scene, triangles, tempTriangles, maxLeafPrimitiveCount, hintDim, planeThickness);
	}

	if (leftChild.isValid() || rightChild.isValid()) {

		leaf.destroy();
	}
}
#endif

void BoundingVolumeHierarchy2::build(Scene& scene, int maxLeafPrimitiveCount, float planeThickness) {

	destroy();
	mScene = &scene;

	BVHTriangles buildTriangles;
	AAB sceneVolume;
	sceneVolume.empty();

	{
		mTriangles.resize(scene.primitives().count);
		for (int pi = 0; pi < scene.primitives().count; ++pi) {

			Primitive_Triangle& prim = dref((Primitive_Triangle*) scene.primitives()[pi]);
		
			prim.appendToVolume(sceneVolume);
			mTriangles[pi].init(pi, prim);
		}

		buildTriangles.resize(scene.primitives().count);
	}

	{
		SoftPtr<BuildNode> rootNode = new BuildNode();

		rootNode->leaf = new BuildLeaf();
		rootNode->leaf->first = 0;
		rootNode->leaf->count = mTriangles.size;
		rootNode->volume = sceneVolume;

		if (mTriangles.size) {
			
			int hintDim = 0;
			rootNode->split(scene, mTriangles, buildTriangles, maxLeafPrimitiveCount, hintDim, planeThickness);
			//rootNode->splitBIH(scene, mTriangles, buildTriangles, maxLeafPrimitiveCount, hintDim, planeThickness);
		}

		mMinDepth = 0;
		mMaxDepth = 0;
		mNodeCount = 0;
		mLeafCount = 0;
		rootNode->recurseCount(mMinDepth, mMaxDepth, mLeafCount, mNodeCount);

		mNodes.resize(mNodeCount);
		unsigned int runningIndex = 0;

		//mRootNode = &mNodes[rootNode->recurseFlatten(mNodes, runningIndex)];
		//assert(runningIndex == mNodeCount);

		mRootNode = &mNodes[rootNode->recurseFlatten2(mNodes, runningIndex)];
		assert(runningIndex + 1 == mNodeCount);

		#ifdef RayTracerConfig_BIHNode
			mRootVolume = rootNode->volume;
		#endif

		rootNode->recurseDestroy();
	}

	#ifdef RayTracerConfig_SpatialAccel_Iter
		
		#ifdef RayTracerConfig_EnablePackets
			mPacketTraversalStack.create(mMaxDepth * 2);
		#endif
		
		mTraversalStack.create(mMaxDepth * 2);
		
	#endif
}

/*
 * http://www.cs.utah.edu/~awilliam/box/
 * Ray-box intersection using IEEE numerical properties to ensure that the
 * test is both robust and efficient, as described in:
 *
 *      Amy Williams, Steve Barrus, R. Keith Morley, and Peter Shirley
 *      "An Efficient and Robust Ray-Box Intersection Algorithm"
 *      Journal of graphics tools, 10(1):49-54, 2005
 *
 */
#ifndef RayTracerConfig_BIHNode
bool BoundingVolumeHierarchy2::Node::intersects2(const Ray& ray, float& outTMin) const {

	//const AAB& volume = mVolume;

	float tmin, tmax, tymin, tymax, tzmin, tzmax;

	tmin = (volume.parameters[ray.sign[0]].x - ray.origin.x) * ray.inv_direction.x;
	tmax = (volume.parameters[1-ray.sign[0]].x - ray.origin.x) * ray.inv_direction.x;
	tymin = (volume.parameters[ray.sign[1]].y - ray.origin.y) * ray.inv_direction.y;
	tymax = (volume.parameters[1-ray.sign[1]].y - ray.origin.y) * ray.inv_direction.y;
	if ( (tmin > tymax) || (tymin > tmax) ) 
		return false;
	if (tymin > tmin)
		tmin = tymin;
	if (tymax < tmax)
		tmax = tymax;
	tzmin = (volume.parameters[ray.sign[2]].z - ray.origin.z) * ray.inv_direction.z;
	tzmax = (volume.parameters[1-ray.sign[2]].z - ray.origin.z) * ray.inv_direction.z;
	if ( (tmin > tzmax) || (tzmin > tmax) ) 
		return false;
	if (tzmin > tmin)
		tmin = tzmin;
	if (tzmax < tmax)
		tmax = tzmax;

	if (0.0f > tmin)
		tmin = 0.0f;
	
	if ( (tmin >= 0.0f) && (tmax >= tmin)) {

		outTMin = tmin;

		return true;
	}

	return false;
}
#endif

#ifdef RayTracerConfig_BIHNode
bool BoundingVolumeHierarchy2::Node::intersects2(const AAB& volume, const Ray& ray, float& outTMin) const {

	float tmin, tmax, tymin, tymax, tzmin, tzmax;

	tmin = (volume.parameters[ray.sign[0]].x - ray.origin.x) * ray.inv_direction.x;
	tmax = (volume.parameters[1-ray.sign[0]].x - ray.origin.x) * ray.inv_direction.x;
	tymin = (volume.parameters[ray.sign[1]].y - ray.origin.y) * ray.inv_direction.y;
	tymax = (volume.parameters[1-ray.sign[1]].y - ray.origin.y) * ray.inv_direction.y;
	if ( (tmin > tymax) || (tymin > tmax) ) 
		return false;
	if (tymin > tmin)
		tmin = tymin;
	if (tymax < tmax)
		tmax = tymax;
	tzmin = (volume.parameters[ray.sign[2]].z - ray.origin.z) * ray.inv_direction.z;
	tzmax = (volume.parameters[1-ray.sign[2]].z - ray.origin.z) * ray.inv_direction.z;
	if ( (tmin > tzmax) || (tzmin > tmax) ) 
		return false;
	if (tzmin > tmin)
		tmin = tzmin;
	if (tzmax < tmax)
		tmax = tzmax;

	if (0.0f > tmin)
		tmin = 0.0f;
	
	if ( (tmin >= 0.0f) && (tmax >= tmin)) {

		outTMin = tmin;

		return true;
	}

	return false;
}
#endif

#ifndef RayTracerConfig_BIHNode
void BoundingVolumeHierarchy2::Node::shootRay3(BoundingVolumeHierarchy2& hier, Node& startNode, Stack1& stack, const RayShot& rayShot, RayShotHit& result, float minT) {

	SoftPtr<Node> nodePtr = &startNode;
	Nodes& nodes = hier.mNodes;

	while (nodePtr.isValid()) {

		const Node& node = nodePtr;
		float nodeMinT = minT;

		if (node.intersects2(dref(rayShot.ray), nodeMinT)
			&& nodeMinT < result.inters.t
			) {
		
			if (node.hasLeaf()) {

				const BVHTriangles& tris = hier.mTriangles;

				for (int pi = node.first; pi < node.first + node.count; ++pi) {

					const BVHTriangle& tri = tris.el[pi];
					intersect(tri, rayShot, result);
				}

				nodePtr = stack.popNode();

			} else {

				if (node.hasRightChild())
					stack.pushNode(&nodes[node.rightChild]);
				
				if (node.hasLeftChild())
					nodePtr = &nodes[node.leftChild];
				else
					nodePtr = stack.popNode();
			}

		} else {

			nodePtr = stack.popNode();
		}
	}
}
#endif

#ifdef RayTracerConfig_BIHNode
void BoundingVolumeHierarchy2::Node::shootRay3(BoundingVolumeHierarchy2& hier, Node& startNode, const AAB& startVolume, Stack1& stack, const RayShot& rayShot, RayShotHit& result, float minT) {

	SoftPtr<Node> nodePtr = &startNode;
	Nodes& nodes = hier.mNodes;
	AAB volume = startVolume;
	
	while (nodePtr.isValid()) {

		const Node& node = nodePtr;
		float nodeMinT = minT;
		
		if (node.intersects2(volume, dref(rayShot.ray), nodeMinT)
			&& nodeMinT < result.inters.t
			) {
		
			if (node.hasLeaf()) {

				const BVHTriangles& tris = hier.mTriangles;

				for (int pi = node.first; pi < node.first + node.count; ++pi) {

					const BVHTriangle& tri = tris.el[pi];
					intersect(tri, rayShot, result);
				}

				nodePtr = stack.popNode(volume);

			} else {

				if (node.hasRightChild()) {

					stack.pushNode(&nodes[node.rightChild], volume, 0, node.dim, node.clip[1]);
				}
				
				if (node.hasLeftChild()) {
					
					volume.max.el[node.dim] = node.clip[0];
					nodePtr = &nodes[node.leftChild];

				} else {

					nodePtr = stack.popNode(volume);
				}
			}

		} else {

			nodePtr = stack.popNode(volume);
		}
	}
}
#else
unsigned int BoundingVolumeHierarchy2::Node::debugShootRay3(BoundingVolumeHierarchy2& hier, Node& startNode, Stack1& stack, const RayShot& rayShot) {

	SoftPtr<Node> nodePtr = &startNode;
	Nodes& nodes = hier.mNodes;

	unsigned int testCount = 0;

	while (nodePtr.isValid()) {

		const Node& node = nodePtr;
		float minT = 0.0f;

		if (node.intersects2(dref(rayShot.ray), minT)) {
		
			++testCount;

			if (node.hasLeaf()) {

				nodePtr = stack.popNode();

			} else {

				if (node.hasRightChild())
					stack.pushNode(&nodes[node.rightChild]);
				
				if (node.hasLeftChild())
					nodePtr = &nodes[node.leftChild];
				else
					nodePtr = stack.popNode();
			}

		} else {

			nodePtr = stack.popNode();
		}
	}

	return testCount;
}
#endif

bool BoundingVolumeHierarchy2::shootRay(const RayShot& rayShot, RayShotHit& result) {


	if (mRootNode.isValid()) {

		#ifdef RayTracerConfig_BIHNode
			Node::shootRay3(dref(this), mRootNode, mRootVolume, mTraversalStack, rayShot, result, 0.0f);
		#else
			Node::shootRay3(dref(this), mRootNode, mTraversalStack, rayShot, result, 0.0f);
		#endif
		
		#ifdef RayTracerConfig_Trace_SpatialAccel_Traversal
			printf("\n");
		#endif

		return result.primID != -1;
	}

	return false;
}

unsigned int BoundingVolumeHierarchy2::debugShootRay(const RayShot& rayShot) {

	#ifndef RayTracerConfig_BIHNode
	if (mRootNode.isValid()) {

		return Node::debugShootRay3(dref(this), mRootNode, mTraversalStack, rayShot);
	}
	#endif

	return 0;
}

#ifdef RayTracerConfig_EnablePackets

bool BoundingVolumeHierarchy2::Node::intersects(const Constants& cts, const PacketRayShot& rayPacket, PacketRayMask& rayPacketState, Packet4& tmin, const bool forceFullTest) const {
	
	if (!forceFullTest) {

		//check 1st ray
		{
			const int rayIndex = rayPacketState.getFirst();
			
			if (rayIndex < 0)
				return false;
			
			float inter_tmin = tmin.el[rayIndex];
			float inter_tmax = std::numeric_limits<float>::infinity();

			for (int dim = 0; dim < 3; ++dim) {

				//float dtmin = (volume.parameters[rayPacket.ray4_sign.deli(dim,0)].el[dim] - rayPacket.ray4_origin.del(dim,0)) * rayPacket.ray4_rdirection.del(dim,0);
				//float dtmax = (volume.parameters[1-rayPacket.ray4_sign.deli(dim,0)].el[dim] - rayPacket.ray4_origin.del(dim,0)) * rayPacket.ray4_rdirection.del(dim,0);

				const float dt0 = (volume.min.el[dim] - rayPacket.ray4_origin.del(dim,0)) * rayPacket.ray4_rdirection.del(dim,0);
				const float dt1 = (volume.max.el[dim] - rayPacket.ray4_origin.del(dim,0)) * rayPacket.ray4_rdirection.del(dim,0);
				const float dtmin = dt0 < dt1 ? dt0 : dt1;
				const float dtmax = dt0 < dt1 ? dt1 : dt0;

				inter_tmin = (inter_tmin < dtmin) ? dtmin : inter_tmin;
				inter_tmax = (dtmax <  inter_tmax) ? dtmax : inter_tmax;
			}

			if (inter_tmin <= inter_tmax) 
				return true;

			if (rayPacketState.getActiveCount() == 1)
				return false;
		}

		//check frustum
		{
		}
	}
	
	//check all rays after 1st ray
	{
		__m128 inter_tmin = tmin.el4;
		__m128 inter_tmax = cts.infinity4;

		for (int dim = 0; dim < 3; ++dim) {

			const __m128 vol_min = _mm_set1_ps(volume.min.el[dim]);
			const __m128 vol_max = _mm_set1_ps(volume.max.el[dim]);

			const __m128 dt0 = _mm_mul_ps(_mm_sub_ps(vol_min, rayPacket.ray4_origin[dim]), rayPacket.ray4_rdirection[dim]);
			const __m128 dt1 = _mm_mul_ps(_mm_sub_ps(vol_max, rayPacket.ray4_origin[dim]), rayPacket.ray4_rdirection[dim]);

			const __m128 dtmin = _mm_min_ps(dt0, dt1);
			const __m128 dtmax = _mm_max_ps(dt0, dt1);

			inter_tmin = _mm_max_ps(inter_tmin, dtmin);
			inter_tmax = _mm_min_ps(dtmax, inter_tmax);
		}

		const __m128 inters = _mm_cmple_ps(inter_tmin, inter_tmax);
		const __m128 active_inters = _mm_and_ps(inters, rayPacketState.active.el4);

		rayPacketState.setActiveMask(active_inters);
		
		tmin.el4 = inter_tmin; 

		return (rayPacketState.getActiveCount() > 0);
	}

	return false;
}

void BoundingVolumeHierarchy2::Node::shootRayPacketIterative(BoundingVolumeHierarchy2& hier, Node& startNode, PacketStack& stack, const Constants& cts, const PacketRayShot& rayPacket, PacketRayHit& t, PacketRayMask& rayPacketState, const Packet4& minT) {

	SoftPtr<Node> nodePtr = &startNode;
	Nodes& nodes = hier.mNodes;

	PacketRayMask rayPacketMask;
	rayPacketMask.reset(cts);

	bool hasIntersections = false;
	//printf("0x%x\n", &rayPacketMask.active.el4);

	while (nodePtr.isValid()) {

		const Node& node = nodePtr;
		Packet4 nodeMinT = minT;
		
		if (
			#ifdef RayTracerConfig_EnablePacketTraversalOcclusion
				node.intersects(cts, rayPacket, rayPacketMask, nodeMinT, hasIntersections)
				&& (_mm_movemask_ps(_mm_cmplt_ps(nodeMinT, t.t)) != 0)
			#else
				node.intersects(cts, rayPacket, rayPacketMask, nodeMinT, false)
			#endif
			) {
		
			if (node.hasLeaf()) {

				BVHTriangles& tris = hier.mTriangles;
				
				for (int pi = node.first; pi < node.first + node.count; ++pi) {

					const BVHTriangle& tri = tris[pi];
					intersect(tri, cts, rayPacket, t, rayPacketState);
				}
				
				#ifdef RayTracerConfig_EnablePacketTraversalOcclusion
					hasIntersections = hasIntersections || (t.getHitCount(cts) > 0);
				#endif
				nodePtr = stack.popNode(rayPacketMask);

			} else {

				if (node.hasRightChild())
					stack.pushNode(&nodes[node.rightChild], rayPacketMask);
				
				if (node.hasLeftChild())
					nodePtr = &nodes[node.leftChild];
				else
					nodePtr = stack.popNode(rayPacketMask);
			}

		} else {

			nodePtr = stack.popNode(rayPacketMask);
		}
	}
}

bool BoundingVolumeHierarchy2::shootRayPacket(const Constants& cts, const PacketRayShot& rayPacket, PacketRayHit& t) {

	if (mRootNode.isValid()) {

		PacketRayMask rayPacketState;
		rayPacketState.reset(cts);

		Node::shootRayPacketIterative(dref(this), mRootNode, mPacketTraversalStack, cts, rayPacket, t, rayPacketState, cts.zero4);
		
		#ifdef RayTracerConfig_Trace_SpatialAccel_Traversal
			printf("\n");
		#endif
	}

	return false;
}
#endif


//ALIGN(ALIGN_CACHELINE) 
static const unsigned int modulo_table[] = {0,1,2,0,1};

void BoundingVolumeHierarchy2::BVHTriangle::init(const unsigned int primIndex, Primitive_Triangle& tri) {

	primID = primIndex;

	{
		const Vector3& vert0 = tri.V(0);
		const Vector3& vert1 = tri.V(1);
		const Vector3& vert2 = tri.V(2);

		Vector3 b, c, N;
		int k;

		b = vert1 - vert0;
		c = vert2 - vert0;
				
		c.cross(b, N);

		if (fabs(N.x) > fabs(N.y))
			if (fabs(N.x) > fabs(N.z)) k = 0; /* X */ else k=2; /* Z */
		else
			if (fabs(N.y) > fabs(N.z)) k = 1; /* Y */ else k=2; /* Z */

		int u = modulo_table[k+1]; 
		int v = modulo_table[k+2];

		N.div(N.el[k]);
		this->k = k;
		this->n_u = N.el[u];
		this->n_v = N.el[v];
		this->n_d = vert0.dot(N);

		float div_b = 1.0f / (b.el[u]*c.el[v] - b.el[v]*c.el[u]);
		this->b_nv = b.el[u] * div_b;
		this->b_nu = -b.el[v] * div_b;
		this->b_d = (b.el[v]*vert0.el[u] - b.el[u]*vert0.el[v]) * div_b;

		float div_c = 1.0f / (c.el[u]*b.el[v] - c.el[v]*b.el[u]);
		this->c_nv = c.el[u] * div_c;
		this->c_nu = -c.el[v] * div_c;
		this->c_d = (c.el[v]*vert0.el[u] - c[u]*vert0.el[v]) * div_c;
	}
}

bool BoundingVolumeHierarchy2::BVHTriangle::intersects(const RayShot& rayShot, float& inOutT) const {

	const float EPSILON = 0.000001f;

	#define ku modulo_table[this->k+1]
	#define kv modulo_table[this->k+2]

	const Vector3& orig = rayShot.ray->origin;
	const Vector3& dir = rayShot.ray->direction;
	
	// don’t prefetch here, assume data has already been prefetched
	// start high-latency division as early as possible
	const float nd_rec = (dir[this->k] + this->n_u * dir[ku] + this->n_v * dir[kv]);
	const float nd = 1.0f/nd_rec;
	const float f = (this->n_d - orig[this->k] - this->n_u * orig[ku] - this->n_v * orig[kv]) * nd;
	
	// check for valid distance.
	if (!(inOutT > f && f > EPSILON )) return 0;
	
	// compute hitpoint positions on uv plane
	const float hu = (orig[ku] + f * dir[ku]);
	const float hv = (orig[kv] + f * dir[kv]);
	
	// check first barycentric coordinate
	const float lambda = (hu * this->b_nu + hv * this->b_nv + this->b_d);
	if (lambda < 0.0f) return 0;
	
	// check second barycentric coordinate
	const float mue = (hu * this->c_nu + hv * this->c_nv + this->c_d);
	if (mue < 0.0f) return 0;
	
	// check third barycentric coordinate
	if (lambda+mue > 1.0f) return 0;
	
	// have a valid hitpoint here. store it.
	inOutT = f;
	//hit.tri = triNum;
	//*u = lambda;
	//*v = mue;

	return 1;
}

#ifdef RayTracerConfig_EnablePackets

void BoundingVolumeHierarchy2::BVHTriangle::intersect(const Constants& cts, const PacketRayShot& rayPacket, PacketRayHit& t, PacketRayMask& rayPacketMask) const {

	const float EPSILON = 0.000001f;

	#define ku modulo_table[this->k+1]
	#define kv modulo_table[this->k+2]
	
	static const __m128 eps = _mm_set_ps1(EPSILON);

	const __m128 acc_nu = _mm_set_ps1(this->n_u);
	const __m128 acc_nv = _mm_set_ps1(this->n_v);
	
	const __m128& dir_k = rayPacket.ray4_direction[this->k];
	const __m128& dir_ku = rayPacket.ray4_direction[ku];
	const __m128& dir_kv = rayPacket.ray4_direction[kv];

	// don’t prefetch here, assume data has already been prefetched
	// start high-latency division as early as possible
	//const float nd = 1.0f/(dir[this->k] + this->n_u * dir[ku] + this->n_v * dir[kv]);
	const __m128 nd_rec = (
							_mm_add_ps(
								dir_k
								,
								_mm_add_ps(
									_mm_mul_ps(acc_nu, dir_ku)
									,
									_mm_mul_ps(acc_nv, dir_kv)
								)
							)
						);
	
	//unprecise rec.
	//const __m128 nd = _mm_rcp_ps(nd_rec);
	
	//rec. with newton-raphon iter.
	//const __m128 nd = _mm_safercp(nd_rec);

	//this might be ok with new CPU's (E6850, radix-16 divider)
	const __m128 nd = _mm_div_ps(cts.one4, nd_rec);
						
		
	const __m128 acc_nd = _mm_set_ps1(this->n_d);
	const __m128& orig_k = rayPacket.ray4_origin[this->k];
	const __m128& orig_ku = rayPacket.ray4_origin[ku];
	const __m128& orig_kv = rayPacket.ray4_origin[kv];
		
	//const float f = (this->n_d - orig[this->k] - this->n_u * orig[ku] - this->n_v * orig[kv]) * nd;
	const __m128 f = _mm_mul_ps(
						_mm_sub_ps(
							_mm_sub_ps(
								_mm_sub_ps(acc_nd, orig_k), _mm_mul_ps(acc_nu, orig_ku)
							), _mm_mul_ps(acc_nv, orig_kv)
						), nd);


	// check for valid distance.
	//if (!(*t > f && f > EPSILON )) return 0;
	
	union { __m128 active_mask4; __m128i active_mask4i; };

	active_mask4 = _mm_and_ps(_mm_cmpgt_ps(f, eps), _mm_cmpgt_ps(t.t.el4, f));
	active_mask4 = _mm_and_ps(active_mask4, rayPacketMask.active.el4);

	int active_mask_bits = _mm_movemask_ps(active_mask4);
	
	if (active_mask_bits == 0) return;
	
	// compute hitpoint positions on uv plane
	//const float hu = (orig[ku] + f * dir[ku]);
	//const float hv = (orig[kv] + f * dir[kv]);

	const __m128 hu = _mm_add_ps(orig_ku, _mm_mul_ps(f, dir_ku));
	const __m128 hv = _mm_add_ps(orig_kv, _mm_mul_ps(f, dir_kv));

	// check first barycentric coordinate
	//const float lambda = (hu * this->b_nu + hv * this->b_nv + this->b_d);
	
	const __m128 acc_b_nu = _mm_set_ps1(this->b_nu);
	const __m128 acc_b_nv = _mm_set_ps1(this->b_nv);
	const __m128 acc_b_d = _mm_set_ps1(this->b_d);

	const __m128 alpha = _mm_add_ps(
								_mm_add_ps(_mm_mul_ps(hu, acc_b_nu), _mm_mul_ps(hv, acc_b_nv))
								, acc_b_d);
	
	//if (lambda < 0.0f) return 0;
	active_mask4 = _mm_and_ps(active_mask4, _mm_cmpge_ps(alpha, cts.zero4));
	active_mask_bits = _mm_movemask_ps(active_mask4);

	if (active_mask_bits == 0) return;
	
	// check second barycentric coordinate
	//const float mue = (hu * this->c_nu + hv * this->c_nv + this->c_d);
	const __m128 acc_c_nu = _mm_set_ps1(this->c_nu);
	const __m128 acc_c_nv = _mm_set_ps1(this->c_nv);
	const __m128 acc_c_d = _mm_set_ps1(this->c_d);

	const __m128 beta = _mm_add_ps(
								_mm_add_ps(_mm_mul_ps(hu, acc_c_nu), _mm_mul_ps(hv, acc_c_nv))
								, acc_c_d);
	
	//if (mue < 0.0f) return 0;
	active_mask4 = _mm_and_ps(active_mask4, _mm_cmpge_ps(beta, cts.zero4));
	active_mask_bits = _mm_movemask_ps(active_mask4);

	if (active_mask_bits == 0) return;

	// check third barycentric coordinate
	//if (lambda+mue > 1.0f) return 0;
	const __m128 gamma = _mm_add_ps(alpha, beta);

	active_mask4 = _mm_and_ps(active_mask4, _mm_cmple_ps(gamma, cts.one4));
	active_mask_bits = _mm_movemask_ps(active_mask4);

	if (active_mask_bits == 0) return;

	// have a valid hitpoint here. store it.
	//*t = f;
	t.t.el4 = _mm_or_ps(_mm_and_ps(active_mask4, f), _mm_andnot_ps(active_mask4, t.t.el4));

	//hit.tri = triNum;
	const __m128i primID = _mm_set1_epi32((int) this->primID);
	t.pid.eli4 = _mm_or_si128(_mm_and_si128(active_mask4i, primID), _mm_andnot_si128(active_mask4i, t.pid.eli4));

	/*
	*u = lambda;
	*v = mue;
	*/

	return;
}

#endif

}

#endif