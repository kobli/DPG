#include "polyline.hpp"

PolylineNode PolylineNode::interpolate(const PolylineNode& n2, float tg) {
	assert(n2.t-t != 0);
	float tn = (tg-t)/(n2.t-t);
	PolylineNode n;
	n.position = (1-tn)*position + tn*n2.position;
	n.direction = direction*glm::pow(glm::rotation(n2.direction, direction), tn);
	n.t = tg;
	n.distance = (1-tn)*distance + tn*n2.distance;
	return n;
}

// returns direct distance from a node (not along the line)
float PolylineNode::distanceFrom(const PolylineNode& n) {
	return glm::distance(position, n.position);
}

std::ostream& operator<<(std::ostream& os, const PolylineNode& n) {
	return os << n.t << " " << n.distance << " " << n.position << " " << n.direction;
}

std::istream& operator>>(std::istream& is, PolylineNode& n) {
	return is >> n.t >> n.distance >> n.position >> n.direction;
}
