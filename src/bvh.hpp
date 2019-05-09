#ifndef BVH_HPP_19_04_24_14_47_14
#define BVH_HPP_19_04_24_14_47_14 
#include <vector>
#include <memory>
#include "types.hpp"

struct PrimitiveInfo {
	unsigned indices[3];
	glm::vec3 centroid;
};

struct NodePrimitives {
	unsigned first;
	unsigned count;
};

class BVH {
	struct BVHBuildNode {
		AABB bounds;
		std::unique_ptr<BVHBuildNode> children[2];
		unsigned depth;
		unsigned firstPrimitive;
		unsigned primitiveCount;
		unsigned compressedNodeI;
	};

	struct BVHNode {
		AABB bounds; // 2*3*sizeof(float) = 24 B
		uint32_t rightChild;
		// data for plane coherency optimization
		uint8_t firstFrustumTestPlane;
		// data for octant test optimization
		glm::vec3 centroid;
		float boundingSphereRadius;
	};

	//TODO test different size - unaligned cache lines
	//static_assert(sizeof(BVHNode) == 32, "BVHNode size is not 32 bytes.");

	public:
	std::vector<unsigned> build(const std::vector<Vertex>& vertices, const std::vector<PrimitiveInfo>& primitivesInfo, unsigned maxPrimitivesInLeaf);
	const std::vector<unsigned>& nodesInFrustum(const std::vector<Plane>& frustumPlanes, const glm::vec3& frustumCenter, const glm::vec3& lookDir, const glm::vec3& up);
	const std::vector<NodePrimitives>& getNodePrimitiveRanges() const;

	private:
		void compress(BVHBuildNode&& root);
		void primitivesAndCentroidsAABB(
				const std::vector<Vertex>& vertices,
				const std::vector<PrimitiveInfo>& primitivesInfo,
				std::vector<unsigned>::iterator primitiveIndexBegin,
				std::vector<unsigned>::iterator primitiveIndexEnd,
				AABB& primitivesAABBout,
				AABB& centroidsAABBout);

		std::vector<BVHNode> _nodes;
		std::vector<NodePrimitives> _nodePrimitives;
};

#endif /* BVH_HPP_19_04_24_14_47_14 */
