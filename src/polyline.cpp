#include "polyline.hpp"

PolyLineNode PolyLineNode::interpolate(const PolyLineNode& n2, float tg) {
	assert(n2.t-t != 0);
	float tn = (tg-t)/(n2.t-t);
	PolyLineNode n;
	n.position = (1-tn)*position + tn*n2.position;
	n.direction = direction*glm::pow(glm::rotation(n2.direction, direction), tn);
	n.t = tg;
	n.distance = (1-tn)*distance + tn*n2.distance;
	return n;
}

// returns direct distance from a node (not along the line)
float PolyLineNode::distanceFrom(const PolyLineNode& n) {
	return glm::distance(position, n.position);
}

std::ostream& operator<<(std::ostream& os, const PolyLineNode& n) {
	return os << n.t << " " << n.distance << " " << n.position << " " << n.direction;
}

std::istream& operator>>(std::istream& is, PolyLineNode& n) {
	return is >> n.t >> n.distance >> n.position >> n.direction;
}
