#ifndef POLYLINE_HPP_19_04_21_16_05_21
#define POLYLINE_HPP_19_04_21_16_05_21 
#include <string>
#include <fstream>
#include <vector>
#include "libs.hpp"
#include "utils.hpp"

/** A concrete node representing 3D position and direction.
 * It uses linear interpolation for position and spherical quaternion interpolation for direction.
 */
struct PolyLineNode {
	PolyLineNode(const glm::vec3& pos = {}, const glm::vec3& dir = {}): position{pos}, direction{dir}
	{}

	/** Returns a new virtual node with interpolated values from this and n2.
	 * @param a second node with higher /t/ value than this.
	 * @param is a global interpolation parameter <0;1> (with respect to the entire polyLine)
	 * It holds that this.t <= tg <= n2.t
	 */
	PolyLineNode interpolate(const PolyLineNode& n2, float tg);

	/** Returns direct distance from a node (not along the polyLine).
	 */
	float distanceFrom(const PolyLineNode& n);

	/** Normalized distance from the beginning of the line (in range <0;1>).
	 * It changes as more nodes are added to the line.
	 */
	float t;

	/** Total absolute distance from the beginning of the polyline, measured along the line.
	 */
	float distance;

	glm::vec3 position;
	glm::vec3 direction;
};

std::ostream& operator<<(std::ostream& os, const PolyLineNode& n);
std::istream& operator>>(std::istream& is, PolyLineNode& n);

/** Sequence of nodes, where each two consequent nodes are connected by a line.
 * The node contains a fixed value that is linked to specific position on the line.
 * The line does not necessarily have to be a line or polyline.
 * The type of interpolation (and its implementation) are handled by the node.
 */
template <typename NodeT>
class PolyLine {
	template <typename T>
	friend std::ostream& operator<<(std::ostream& os, const PolyLine<T>& l);
	template <typename T>
	friend std::istream& operator>>(std::istream& os, PolyLine<T>& l);

	public:
		/** Adds another node to the end of the line.
		 */
		void appendNode(const NodeT& node) {
			_nodes.push_back(node);
			if(_nodes.size() >= 2) {
				const NodeT& nBeforeLast = _nodes[_nodes.size()-2];
				_nodes.back().distance = nBeforeLast.distance + _nodes.back().distanceFrom(nBeforeLast);
			}
			updateNodeTs();
		}

		/** Returns interpolated value of possibly virtual node that would be placed at the given position along the line.
		 * @param is in range <0.0, 1.0>
		 */
		NodeT getNode(float t) {
			assert(t >= 0);
			assert(t <= 1);
			unsigned i = 0;
			for(; i < _nodes.size(); ++i)
				if(_nodes[i].t >= t)
					break;
			// i is now index of the second of the two nodes between which we will interpolate
			if(i == 0)
				return _nodes[i];
			else
				return _nodes[i-1].interpolate(_nodes[i], t);
		}

		/** Returns a possibly virtual node whose distance from node n along the line is /d/
		 * Or end of the line if distance is too large.
		 */
		NodeT getNode(const NodeT& n, float d) {
			NodeT& lastNode = _nodes.back(); // use last node to compute the t/distance ratio because it has non-zero distance
			float newT = lastNode.t/lastNode.distance*(n.distance+d);
			newT = std::min(newT, 1.f);
			return getNode(newT);
		}

		bool save(const std::string& fileName) const {
			std::ofstream of(fileName);
			if(!of)
				return false;
			else {
				of << *this;
				return true;
			}
		}

		bool load(const std::string& fileName) {
			std::ifstream ifile(fileName);
			if(!ifile)
				return false;
			else {
				ifile >> *this;
				return true;
			}
		}

	private:
		/** Updates the value of t stored inside every node.
		 */
		void updateNodeTs() {
			float length = _nodes.back().distance;
			for(NodeT& n : _nodes)
				n.t = n.distance/length;
		}

		std::vector<NodeT> _nodes;
};

template <typename NodeT>
std::ostream& operator<<(std::ostream& os, const PolyLine<NodeT>& l) {
	for(const NodeT& n : l._nodes)
		os << n << std::endl;
	return os;
}

template <typename NodeT>
std::istream& operator>>(std::istream& is, PolyLine<NodeT>& l) {
	NodeT n;
	while(is.good()) {
		is >> n;
		l._nodes.push_back(n);
	}
	return is;
}

#endif /* POLYLINE_HPP_19_04_21_16_05_21 */
