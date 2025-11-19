#define _USE_MATH_DEFINES
#include <cmath>

#include "MeshBuilder.h"
#include <GL\glew.h>
#include <vector>

/******************************************************************************/
/*!
\brief
Generate the vertices of a reference Axes; Use red for x-axis, green for y-axis, blue for z-axis
Then generate the VBO/IBO and store them in Mesh object

\param meshName - name of mesh
\param lengthX - x-axis should start at -lengthX / 2 and end at lengthX / 2
\param lengthY - y-axis should start at -lengthY / 2 and end at lengthY / 2
\param lengthZ - z-axis should start at -lengthZ / 2 and end at lengthZ / 2

\return Pointer to mesh storing VBO/IBO of reference axes
*/
/******************************************************************************/
Mesh* MeshBuilder::GenerateAxes(const std::string& meshName, float lengthX, float lengthY, float lengthZ)
{
	Vertex v;
	std::vector<Vertex> vertex_buffer_data;
	std::vector<GLuint> index_buffer_data;

	//x-axis
	v.pos = glm::vec3(-lengthX, 0, 0); v.color = glm::vec3(1, 0, 0); vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(lengthX, 0, 0); v.color = glm::vec3(1, 0, 0); vertex_buffer_data.push_back(v);

	//y-axis
	v.pos = glm::vec3(0, -lengthY, 0); v.color = glm::vec3(0, 1, 0); vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0, lengthY, 0); v.color = glm::vec3(0, 1, 0); vertex_buffer_data.push_back(v);

	//z-axis
	v.pos = glm::vec3(0, 0, -lengthZ); v.color = glm::vec3(0, 0, 1); vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0, 0, lengthZ); v.color = glm::vec3(0, 0, 1); vertex_buffer_data.push_back(v);

	index_buffer_data.push_back(0);
	index_buffer_data.push_back(1);
	index_buffer_data.push_back(2);
	index_buffer_data.push_back(3);
	index_buffer_data.push_back(4);
	index_buffer_data.push_back(5);

	Mesh *mesh = new Mesh(meshName);

	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() * sizeof(Vertex), &vertex_buffer_data[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_data.size() * sizeof(GLuint), &index_buffer_data[0], GL_STATIC_DRAW);

	mesh->indexSize = index_buffer_data.size();
	mesh->mode = Mesh::DRAW_LINES;

	return mesh;
}

/******************************************************************************/
/*!
\brief
Generate the vertices of a quad; Use random color for each vertex
Then generate the VBO/IBO and store them in Mesh object

\param meshName - name of mesh
\param lengthX - width of quad
\param lengthY - height of quad

\return Pointer to mesh storing VBO/IBO of quad
*/
/******************************************************************************/
Mesh* MeshBuilder::GenerateQuad(const std::string& meshName, glm::vec3 color, float length)
{
	Vertex v;
	std::vector<Vertex> vertex_buffer_data;
	std::vector<GLuint> index_buffer_data;

	v.color = color;

	// Add the vertices here
	v.pos = glm::vec3(-0.5f * length, -0.5f * length, 0.f); vertex_buffer_data.push_back(v); //v3
	v.pos = glm::vec3(0.5f * length, -0.5f * length, 0.f); vertex_buffer_data.push_back(v); //v0
	v.pos = glm::vec3(0.5f * length, 0.5f * length, 0.f); vertex_buffer_data.push_back(v); //v1
	v.pos = glm::vec3(-0.5f * length, 0.5f * length, 0.f); vertex_buffer_data.push_back(v); //v2

	index_buffer_data.push_back(0);
	index_buffer_data.push_back(1);
	index_buffer_data.push_back(2);

	index_buffer_data.push_back(0);
	index_buffer_data.push_back(2);
	index_buffer_data.push_back(3);


	// Create the new mesh
	Mesh* mesh = new Mesh(meshName);

	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() * sizeof(Vertex), &vertex_buffer_data[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_data.size() * sizeof(GLuint), &index_buffer_data[0], GL_STATIC_DRAW);

	mesh->indexSize = index_buffer_data.size();
	mesh->mode = Mesh::DRAW_TRIANGLES;

	return mesh;
}

Mesh* MeshBuilder::GenerateSphere(const std::string& meshName, glm::vec3 color, float radius, int numSlice, int numStack)
{
	Vertex v; // Vertex definition
	std::vector<Vertex> vertex_buffer_data; // Vertex Buffer Objects (VBOs)
	std::vector<GLuint> index_buffer_data; // Element Buffer Objects (EBOs)

	v.color = color;

	float degreePerStack = glm::pi<float>() / numStack;
	float degreePerSlice = glm::two_pi<float>() / numSlice;


	for (unsigned stack = 0; stack < numStack + 1; ++stack) //stack
	{
		float phi = -glm::half_pi<float>() + stack * degreePerStack;
		for (unsigned slice = 0; slice < numSlice + 1; ++slice) //slice
		{
			float theta = slice * degreePerSlice;
			v.pos = glm::vec3(radius * glm::cos(phi) * glm::cos(theta),
				radius * glm::sin(phi),
				radius * glm::cos(phi) * glm::sin(theta));

			// position with length of one
			v.normal = glm::vec3(glm::cos(phi) * glm::cos(theta), glm::sin(phi), glm::cos(phi) * glm::sin(theta));

			vertex_buffer_data.push_back(v);
		}
	}

	for (unsigned stack = 0; stack < numStack; ++stack)
	{
		for (unsigned slice = 0; slice < numSlice + 1; ++slice)
		{
			index_buffer_data.push_back(stack * (numSlice + 1) + slice); // fill the index to push
			index_buffer_data.push_back((stack + 1) * (numSlice + 1) + slice); // fill the index to push
		}
	}

	// Create the new mesh
	Mesh* mesh = new Mesh(meshName);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() *
		sizeof(Vertex),
		&vertex_buffer_data[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_data.size() *
		sizeof(GLuint),
		&index_buffer_data[0], GL_STATIC_DRAW);
	mesh->indexSize = index_buffer_data.size();
	mesh->mode = Mesh::DRAW_TRIANGLE_STRIP;
	return mesh;
}

Mesh* MeshBuilder::GenerateTorus(const std::string& meshName, glm::vec3 color, float innerR, float outerR, int numSlice, int numStack)
{
	Vertex v; // Vertex definition
	std::vector<Vertex> vertex_buffer_data; // Vertex Buffer Objects (VBOs)
	std::vector<GLuint> index_buffer_data; // Element Buffer Objects (EBOs)

	v.color = color;

	float degreePerStack = glm::two_pi<float>() / numStack;
	float degreePerSlice = glm::two_pi<float>() / numSlice;
	for (unsigned stack = 0; stack < numStack + 1; ++stack) //stack
	{
		float phi = stack * degreePerStack;
		for (unsigned slice = 0; slice < numSlice + 1; ++slice) //slice
		{
			float theta = slice * degreePerSlice;
			v.pos = glm::vec3((outerR + innerR * glm::cos(theta)) * glm::
				sin(phi),
				innerR * glm::sin(theta),
				(outerR + innerR * glm::cos(theta)) *
				glm::cos(phi));
			vertex_buffer_data.push_back(v);
		}
	}

	for (unsigned stack = 0; stack < numStack; ++stack)
	{
		for (unsigned slice = 0; slice < numSlice + 1; ++slice)
		{
			index_buffer_data.push_back(stack * (numSlice + 1) + slice);
			index_buffer_data.push_back((stack + 1) * (numSlice + 1) +
				slice);
		}
	}

	// Create the new mesh
	Mesh* mesh = new Mesh(meshName);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() *
		sizeof(Vertex),
		&vertex_buffer_data[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_data.size() *
		sizeof(GLuint),
		&index_buffer_data[0], GL_STATIC_DRAW);
	mesh->indexSize = index_buffer_data.size();
	mesh->mode = Mesh::DRAW_TRIANGLE_STRIP;
	return mesh;
}


Mesh* MeshBuilder::GenerateCylinder(const std::string& meshName, glm::vec3 color, unsigned numSlice, float radius, float height)
{
	Vertex v;
	v.color = color;

	std::vector<Vertex> vertex_buffer_data;
	std::vector<GLuint> index_buffer_data;

	float degreePerSlice = 360.f / numSlice;

	// -------------------------------------
	// 1. Bottom Cap
	// -------------------------------------
	unsigned bottomCenterIndex = vertex_buffer_data.size();
	v.pos = glm::vec3(0, -0.5f * height, 0);
	v.normal = glm::vec3(0, -1, 0);
	vertex_buffer_data.push_back(v);

	// Outer rim vertices
	for (unsigned slice = 0; slice <= numSlice; ++slice)
	{
		float theta = glm::radians(slice * degreePerSlice);
		v.pos = glm::vec3(radius * cos(theta), -0.5f * height, radius * sin(theta));
		v.normal = glm::vec3(0, -1, 0);
		vertex_buffer_data.push_back(v);
	}

	// Bottom triangles (fan)
	for (unsigned slice = 1; slice <= numSlice; ++slice)
	{
		index_buffer_data.push_back(bottomCenterIndex);
		index_buffer_data.push_back(bottomCenterIndex + slice);
		index_buffer_data.push_back(bottomCenterIndex + slice + 1);
	}

	// -------------------------------------
	// 2. Side Wall
	// -------------------------------------
	unsigned sideStartIndex = vertex_buffer_data.size();

	for (unsigned slice = 0; slice <= numSlice; ++slice)
	{
		float theta = glm::radians(slice * degreePerSlice);
		float cosT = cos(theta);
		float sinT = sin(theta);

		// bottom vertex
		v.pos = glm::vec3(radius * cosT, -0.5f * height, radius * sinT);
		v.normal = glm::vec3(cosT, 0, sinT); // smooth shading
		vertex_buffer_data.push_back(v);

		// top vertex
		v.pos = glm::vec3(radius * cosT, 0.5f * height, radius * sinT);
		v.normal = glm::vec3(cosT, 0, sinT);
		vertex_buffer_data.push_back(v);
	}

	// Side triangles
	for (unsigned slice = 0; slice < numSlice; ++slice)
	{
		int i0 = sideStartIndex + slice * 2;
		int i1 = i0 + 1;
		int i2 = i0 + 2;
		int i3 = i0 + 3;

		// First triangle
		index_buffer_data.push_back(i0);
		index_buffer_data.push_back(i1);
		index_buffer_data.push_back(i2);

		// Second triangle
		index_buffer_data.push_back(i2);
		index_buffer_data.push_back(i1);
		index_buffer_data.push_back(i3);
	}

	// -------------------------------------
	// 3. Top Cap
	// -------------------------------------
	unsigned topCenterIndex = vertex_buffer_data.size();
	v.pos = glm::vec3(0, 0.5f * height, 0);
	v.normal = glm::vec3(0, 1, 0);
	vertex_buffer_data.push_back(v);

	for (unsigned slice = 0; slice <= numSlice; ++slice)
	{
		float theta = glm::radians(slice * degreePerSlice);
		v.pos = glm::vec3(radius * cos(theta), 0.5f * height, radius * sin(theta));
		v.normal = glm::vec3(0, 1, 0);
		vertex_buffer_data.push_back(v);
	}

	// Top triangles (fan)
	for (unsigned slice = 1; slice <= numSlice; ++slice)
	{
		index_buffer_data.push_back(topCenterIndex);
		index_buffer_data.push_back(topCenterIndex + slice + 1);
		index_buffer_data.push_back(topCenterIndex + slice);
	}

	// -------------------------------------
	// 4. Upload to GPU
	// -------------------------------------
	Mesh* mesh = new Mesh(meshName);

	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER,
		vertex_buffer_data.size() * sizeof(Vertex),
		&vertex_buffer_data[0],
		GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		index_buffer_data.size() * sizeof(GLuint),
		&index_buffer_data[0],
		GL_STATIC_DRAW);

	mesh->indexSize = index_buffer_data.size();
	mesh->mode = Mesh::DRAW_TRIANGLES;

	return mesh;
}
