#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include "objLoader/objLoader.h"
#include "utils.hpp"
#include "object.hpp"

Object::Object(const std::string& fileName): 
	_queryID{0},
	_vao{0},
	_indexBuffer{0},
	_vertexBuffer{0},
	_queryActive{false}
{
	objLoader objData;
	std::vector<char> fileNameCharArray(fileName.begin(), fileName.end());
	fileNameCharArray.push_back('\0');
	if(!objData.load(fileNameCharArray.data()))
		throw std::string("Failed to read the object from .obj file: ")+fileName+std::string("\n");

	if(objData.normalCount <= 0)
		std::cerr << "The model " << fileName << " does not contain normals.\n";

	_vertexCount = objData.vertexCount;
	std::vector<Vertex> vertices(_vertexCount);
	for(int i = 0; i < objData.vertexCount; ++i) {
		obj_vector& vdata = (*objData.vertexList[i]);
		vertices[i].position = {vdata.e[0], vdata.e[1], vdata.e[2]};
		vertices[i].normal = {0,0,0};
		_aabb.unite(vertices[i].position);
	}
	std::vector<PrimitiveInfo> primitivesInfo(objData.faceCount);
	for(int i = 0; i < objData.faceCount; ++i) {
		obj_face& face = (*objData.faceList[i]);
		assert(face.vertex_count == 3);
		primitivesInfo[i].indices[0] = face.vertex_index[0];
		primitivesInfo[i].indices[1] = face.vertex_index[1];
		primitivesInfo[i].indices[2] = face.vertex_index[2];
		primitivesInfo[i].centroid = (
				vertices[face.vertex_index[0]].position +
				vertices[face.vertex_index[1]].position +
				vertices[face.vertex_index[2]].position
				)/3.f;
		const obj_vector& n0 = *objData.normalList[face.normal_index[0]];
		const obj_vector& n1 = *objData.normalList[face.normal_index[1]];
		const obj_vector& n2 = *objData.normalList[face.normal_index[2]];
		vertices[face.vertex_index[0]].normal += glm::vec3(n0.e[0], n0.e[1], n0.e[2]);
		vertices[face.vertex_index[1]].normal += glm::vec3(n1.e[0], n1.e[1], n1.e[2]);
		vertices[face.vertex_index[2]].normal += glm::vec3(n2.e[0], n2.e[1], n2.e[2]);
	}
	_triangleCount = objData.faceCount;
	for(Vertex& v : vertices)
		v.normal = glm::normalize(v.normal);

	const unsigned MAX_PRIMITIVES_IN_LEAF = 10000;
	std::vector<unsigned> primitiveOrder = _bvh.build(vertices, primitivesInfo, MAX_PRIMITIVES_IN_LEAF);
	std::vector<unsigned int> indices(objData.faceCount*3);
	for(unsigned i = 0; i < primitiveOrder.size(); ++i) {
		unsigned primID = primitiveOrder[i];
		indices[i*3+0] = primitivesInfo[primID].indices[0];
		indices[i*3+1] = primitivesInfo[primID].indices[1];
		indices[i*3+2] = primitivesInfo[primID].indices[2];
	}

	if(objData.materialCount >= 1) {
		obj_material& om = *objData.materialList[0];
		Material& m = _material;

		if(objData.materialCount > 1)
			std::cerr << "Only one material is supported. Using the first material ("
				<< om.name << ") for the entire model ... \n";

		//TODO per channel Ka, Kd, Ks
		m.ambientK = om.amb[0];
		m.diffuseK = om.diff[0];
		m.specularK = om.spec[0];
		m.shininess = om.shiny;
		m.color = {1,1,1,1};
	}
	else {
		std::cerr << "The model does not contain material/s ... setting Kd=1, Ks=Ka=Alpha=0\n";
		_material.diffuseK = 1;
	}

	glGenQueries(1, &_queryID);

	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	glGenBuffers(1, &_indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &_vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, _vertexCount*sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(offsetof(Vertex, normal)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
}

Object::~Object() {
	glDeleteQueries(1, &_queryID);
	glDeleteVertexArrays(1, &_vao);
	glDeleteBuffers(1, &_indexBuffer);
	glDeleteBuffers(1, &_vertexBuffer);
}

void Object::setPosition(glm::vec3 pos) {
	_transform[3][0] = pos.x;
	_transform[3][1] = pos.y;
	_transform[3][2] = pos.z;
}

glm::vec3 Object::getPosition() const {
	return _transform[3];
}

double Object::getDrawTime() const {
	return _drawTime;
}

unsigned Object::getRenderedTriangleCount() const {
	return _renderedTriangleCount;
}

unsigned Object::getTriangleCount() const {
	return _triangleCount;
}

glm::mat4 Object::getTransform() const {
	return _transform;
}

Material& Object::getMaterial() {
	return _material;
}

const AABB& Object::getAABB() const {
	return _aabb;
}

void Object::draw(const std::vector<Plane>& frustumPlanes, const glm::vec3& frustumCenter, const glm::vec3& lookDir, const glm::vec3& up, bool doTimerQuery) {
	glBindVertexArray(_vao);
	GLuint64 qr = GL_FALSE;
	if(_queryActive && doTimerQuery)
		glGetQueryObjectui64v(_queryID, GL_QUERY_RESULT_AVAILABLE, &qr);
	bool queryResultReady = (qr == GL_TRUE);
	if(queryResultReady && doTimerQuery) {
		glGetQueryObjectui64v(_queryID, GL_QUERY_RESULT, &qr);
		_drawTime = double(qr)/1000/1000;
		_queryActive = false;
	}
	if(!_queryActive && doTimerQuery) {
		glBeginQuery(GL_TIME_ELAPSED, _queryID);
		doDrawing(frustumPlanes, frustumCenter, lookDir, up);
		glEndQuery(GL_TIME_ELAPSED);
		_queryActive = true;
	}
	else
		doDrawing(frustumPlanes, frustumCenter, lookDir, up);
}

void Object::doDrawing(const std::vector<Plane>& frustumPlanes, const glm::vec3& frustumCenter, const glm::vec3& up, const glm::vec3& lookDir) {
	if(_prevFrustumCenter != frustumCenter) {
		_visibleNodes = _bvh.nodesInFrustum(frustumPlanes, frustumCenter, lookDir, up);
		_prevFrustumCenter = frustumCenter;
	}
	const std::vector<NodePrimitives>& nodePrimitives = _bvh.getNodePrimitiveRanges();
	_renderedTriangleCount = 0;
	for(const unsigned& nID: _visibleNodes) {
		glDrawElements(GL_TRIANGLES, nodePrimitives[nID].count*3, GL_UNSIGNED_INT, BUFFER_OFFSET(sizeof(unsigned)*3*nodePrimitives[nID].first));
		_renderedTriangleCount += nodePrimitives[nID].count;
	}
}
