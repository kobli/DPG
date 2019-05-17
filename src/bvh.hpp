#ifndef BVH_HPP_19_04_24_14_47_14
#define BVH_HPP_19_04_24_14_47_14 
#include <vector>
#include <memory>
#include "types.hpp"

/** Information about a primitive (triangle).
 */
struct PrimitiveInfo {
	unsigned indices[3];
	glm::vec3 centroid;
};

/** Range of primitives given by index of the first and count.
 */
struct NodePrimitives {
	unsigned first;
	unsigned count;
};

/** BVH used for frustum culling.
 * Bounding volumes are axis-aligned boxes.
 */
class BVH {
	/** Temporary node used only for construction.
	 * The tree is afterwards rebuilt (compressed) into array using implicit pointers.
	 */
	struct BVHBuildNode {
		AABB bounds;
		std::unique_ptr<BVHBuildNode> children[2];
		unsigned depth;
		unsigned firstPrimitive;
		unsigned primitiveCount;
		unsigned compressedNodeI;
	};

	/** Final node used for traversal.
	 */
	struct BVHNode {
		AABB bounds; // 2*3*sizeof(float) = 24 B
		uint32_t rightChild;
		// data for plane coherency optimization
		uint8_t firstFrustumTestPlane;
		// data for octant test optimization
		glm::vec3 centroid;
		float boundingSphereRadius;
	};

	public:
	std::vector<unsigned> build(const std::vector<Vertex>& vertices, const std::vector<PrimitiveInfo>& primitivesInfo, unsigned maxPrimitivesInLeaf);

	/** Returns a reference to nodes, which contain potentially visible primitives.
	 * The referenced vector will be reused in next call.
	 */
	const std::vector<unsigned>& nodesInFrustum(const std::vector<Plane>& frustumPlanes, const glm::vec3& frustumCenter, const glm::vec3& lookDir, const glm::vec3& up);

	/** Returns array of primitive ranges for each node.
	 */
	const std::vector<NodePrimitives>& getNodePrimitiveRanges() const;

	private:
		/** Transforms a dynamic BVH with pointers into compressed array form with implicit pointers to be used for traversal.
		 */
		void compress(BVHBuildNode&& root);
		
		/** Calculates AABB of primitives and AABB of their centroids.
		 */
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
