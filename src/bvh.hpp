#ifndef BVH_HPP_19_04_24_14_47_14
#define BVH_HPP_19_04_24_14_47_14 
#include <vector>
#include <memory>
#include "types.hpp"

struct PrimitiveInfo {
	unsigned indices[3];
	glm::vec3 centroid;
};

using Plane = glm::vec4;

inline void normalizePlane(Plane& p) {
	float l = glm::length(glm::vec3(p));
	p /= l;
}

class BVH {
	struct BVHBuildNode {
		AABB bounds;
		std::unique_ptr<BVHBuildNode> children[2];
		unsigned firstPrimitive;
		unsigned primitiveCount;
		unsigned compressedNodeI;
	};

	struct BVHNode {
		AABB bounds;
		uint32_t rightChild;
		uint8_t firstFrustumTestPlane;
	};

	struct NodePrimitives {
		unsigned first;
		unsigned count;
	};

	//TODO test different size - unaligned cache lines
	static_assert(sizeof(BVHNode) == 32, "");

	public:
	std::vector<unsigned> build(const std::vector<Vertex>& vertices, const std::vector<PrimitiveInfo>& primitivesInfo, unsigned maxPrimitivesInLeaf);
	const std::vector<unsigned>& nodesInFrustum(const std::vector<Plane>& frustumPlanes);

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
