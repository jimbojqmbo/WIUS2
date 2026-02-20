#define _USE_MATH_DEFINES
#include <cmath>

#include "MeshBuilder.h"
#include <GL\glew.h>
#include <vector>
#include "LoadOBJ.h"

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
	v.pos = glm::vec3(-0.5f * length, -0.5f * length, 0.f); v.normal = glm::vec3(0, 0, 1); v.texCoord = glm::vec2(1, 0); vertex_buffer_data.push_back(v); //v3
	v.pos = glm::vec3(0.5f * length, -0.5f * length, 0.f); v.normal = glm::vec3(0, 1, 0); v.texCoord = glm::vec2(1, 1); vertex_buffer_data.push_back(v); //v0
	v.pos = glm::vec3(0.5f * length, 0.5f * length, 0.f); v.normal = glm::vec3(0, 0, 0); v.texCoord = glm::vec2(0, 1); vertex_buffer_data.push_back(v); //v1
	v.pos = glm::vec3(-0.5f * length, 0.5f * length, 0.f); v.normal = glm::vec3(1, 1, 0); v.texCoord = glm::vec2(0, 0); vertex_buffer_data.push_back(v); //v2

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

Mesh* MeshBuilder::GenerateCube(const std::string& meshName, glm::vec3 color, float length)
{
	Vertex v; // Vertex definition
	std::vector<Vertex> vertex_buffer_data; // Vertex Buffer Objects (VBOs)
	std::vector<GLuint> index_buffer_data; // Element Buffer Objects (EBOs)

	v.color = color;

	v.pos = glm::vec3(0.5f * length, 0.5f * length, 0.5f * length);	v.normal = glm::vec3(0, 0, 1);	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * length, 0.5f * length, 0.5f * length);	v.normal = glm::vec3(0, 0, 1);	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * length, -0.5f * length, 0.5f * length);	v.normal = glm::vec3(0, 0, 1);	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * length, 0.5f * length, 0.5f * length);	v.normal = glm::vec3(0, 0, 1);	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * length, -0.5f * length, 0.5f * length);	v.normal = glm::vec3(0, 0, 1);	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * length, -0.5f * length, 0.5f * length);	v.normal = glm::vec3(0, 0, 1);	vertex_buffer_data.push_back(v);

	v.pos = glm::vec3(0.5f * length, 0.5f * length, -0.5f * length);	v.normal = glm::vec3(1, 0, 0);	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * length, 0.5f * length, 0.5f * length);	v.normal = glm::vec3(1, 0, 0);	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * length, -0.5f * length, 0.5f * length);	v.normal = glm::vec3(1, 0, 0);	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * length, 0.5f * length, -0.5f * length);	v.normal = glm::vec3(1, 0, 0);	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * length, -0.5f * length, 0.5f * length);	v.normal = glm::vec3(1, 0, 0);	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * length, -0.5f * length, -0.5f * length);	v.normal = glm::vec3(1, 0, 0);	vertex_buffer_data.push_back(v);

	v.pos = glm::vec3(0.5f * length, 0.5f * length, -0.5f * length);	v.normal = glm::vec3(0, 1, 0);	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * length, 0.5f * length, -0.5f * length);	v.normal = glm::vec3(0, 1, 0);	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * length, 0.5f * length, 0.5f * length);	v.normal = glm::vec3(0, 1, 0);	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * length, 0.5f * length, -0.5f * length);	v.normal = glm::vec3(0, 1, 0);	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * length, 0.5f * length, 0.5f * length);	v.normal = glm::vec3(0, 1, 0);	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * length, 0.5f * length, 0.5f * length);	v.normal = glm::vec3(0, 1, 0);	vertex_buffer_data.push_back(v);

	v.pos = glm::vec3(-0.5f * length, 0.5f * length, -0.5f * length);	v.normal = glm::vec3(0, 0, -1);	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * length, 0.5f * length, -0.5f * length);	v.normal = glm::vec3(0, 0, -1);	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * length, -0.5f * length, -0.5f * length);	v.normal = glm::vec3(0, 0, -1);	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * length, 0.5f * length, -0.5f * length);	v.normal = glm::vec3(0, 0, -1);	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * length, -0.5f * length, -0.5f * length);	v.normal = glm::vec3(0, 0, -1);	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * length, -0.5f * length, -0.5f * length);	v.normal = glm::vec3(0, 0, -1);	vertex_buffer_data.push_back(v);

	v.pos = glm::vec3(-0.5f * length, 0.5f * length, 0.5f * length);	v.normal = glm::vec3(-1, 0, 0);	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * length, 0.5f * length, -0.5f * length);	v.normal = glm::vec3(-1, 0, 0);	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * length, -0.5f * length, -0.5f * length);	v.normal = glm::vec3(-1, 0, 0);	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * length, 0.5f * length, 0.5f * length);	v.normal = glm::vec3(-1, 0, 0);	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * length, -0.5f * length, -0.5f * length);	v.normal = glm::vec3(-1, 0, 0);	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * length, -0.5f * length, 0.5f * length);	v.normal = glm::vec3(-1, 0, 0);	vertex_buffer_data.push_back(v);

	v.pos = glm::vec3(-0.5f * length, -0.5f * length, -0.5f * length);	v.normal = glm::vec3(0, -1, 0);	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * length, -0.5f * length, -0.5f * length);	v.normal = glm::vec3(0, -1, 0);	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * length, -0.5f * length, 0.5f * length);	v.normal = glm::vec3(0, -1, 0);	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * length, -0.5f * length, -0.5f * length);	v.normal = glm::vec3(0, -1, 0);	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * length, -0.5f * length, 0.5f * length);	v.normal = glm::vec3(0, -1, 0);	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * length, -0.5f * length, 0.5f * length);	v.normal = glm::vec3(0, -1, 0);	vertex_buffer_data.push_back(v);

	for (unsigned i = 0; i < 36; ++i)
	{
		index_buffer_data.push_back(i);
	}

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

Mesh* MeshBuilder::GenerateHemisphere(const std::string& meshName, glm::vec3 color, unsigned numStack, unsigned numSlice, float radius)
{
	std::vector<Vertex> vertex_buffer_data;
	std::vector<GLuint> index_buffer_data;

	Vertex v;
	v.color = color;

	const float stackStep = glm::radians(90.f / numStack);
	const float sliceStep = glm::radians(360.f / numSlice);

	// ---------------------------------------------------------
	// 1. Generate vertices (hemisphere surface)
	// ---------------------------------------------------------

	for (unsigned stack = 0; stack <= numStack; ++stack)
	{
		float phi = stack * stackStep;     // elevation angle (0 ? 90 degrees)
		float y = radius * sin(phi);
		float r = radius * cos(phi);       // radius of current ring

		for (unsigned slice = 0; slice <= numSlice; ++slice)
		{
			float theta = slice * sliceStep;

			v.pos = glm::vec3(
				r * cos(theta),
				y,
				r * sin(theta)
			);

			v.normal = glm::normalize(v.pos); // perfect lighting
			v.color = color;

			vertex_buffer_data.push_back(v);
		}
	}

	// ---------------------------------------------------------
	// 2. Generate indices (triangles)
	// ---------------------------------------------------------
	// (stack rings connected by quads ? split into 2 triangles)
	// ---------------------------------------------------------

	unsigned ringVertexCount = numSlice + 1;

	for (unsigned stack = 0; stack < numStack; ++stack)
	{
		for (unsigned slice = 0; slice < numSlice; ++slice)
		{
			// Four vertices of a quad
			unsigned v0 = stack * ringVertexCount + slice;
			unsigned v1 = v0 + 1;
			unsigned v2 = v0 + ringVertexCount;
			unsigned v3 = v2 + 1;

			// First triangle
			index_buffer_data.push_back(v0);
			index_buffer_data.push_back(v2);
			index_buffer_data.push_back(v1);

			// Second triangle
			index_buffer_data.push_back(v1);
			index_buffer_data.push_back(v2);
			index_buffer_data.push_back(v3);
		}
	}

	// ---------------------------------------------------------
	// 3. bottom cap (flat circle)
	// ---------------------------------------------------------
	
	unsigned centerIndex = vertex_buffer_data.size();
	v.pos = glm::vec3(0, 0, 0);
	v.normal = glm::vec3(0, -1, 0);
	v.color = color;
	vertex_buffer_data.push_back(v);

	for (unsigned slice = 0; slice <= numSlice; ++slice)
	{
		unsigned rim = slice;
		index_buffer_data.push_back(centerIndex);
		index_buffer_data.push_back(rim + 1);
		index_buffer_data.push_back(rim);
	}


	// ---------------------------------------------------------
	// 4. Upload to GPU
	// ---------------------------------------------------------

	Mesh* mesh = new Mesh(meshName);

	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() * sizeof(Vertex), &vertex_buffer_data[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_data.size() * sizeof(GLuint), &index_buffer_data[0], GL_STATIC_DRAW);

	mesh->indexSize = index_buffer_data.size();
	mesh->mode = Mesh::DRAW_TRIANGLES;

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


Mesh* MeshBuilder::GenerateHead(const std::string& meshName, glm::vec3 color, unsigned numStack, unsigned numSlice, float radius, float cylHeight)
{
	std::vector<Vertex> vData;
	std::vector<GLuint> iData;

	Vertex v;
	v.color = color;

	// HEMISPHERE
	float stackStep = glm::radians(90.f / numStack);
	float sliceStep = glm::radians(360.f / numSlice);

	for (unsigned stack = 0; stack <= numStack; ++stack)
	{
		float phi = stack * stackStep;
		float y = radius * sin(phi);
		float r = radius * cos(phi);

		for (unsigned slice = 0; slice <= numSlice; ++slice)
		{
			float theta = slice * sliceStep;

			v.pos = glm::vec3(r * cos(theta), y, r * sin(theta));
			v.normal = glm::normalize(v.pos);
			vData.push_back(v);
		}
	}

	unsigned hemiCount = vData.size();
	unsigned ring = numSlice + 1;

	for (unsigned stack = 0; stack < numStack; ++stack)
	{
		for (unsigned slice = 0; slice < numSlice; ++slice)
		{
			GLuint v0 = stack * ring + slice;
			GLuint v1 = v0 + 1;
			GLuint v2 = v0 + ring;
			GLuint v3 = v2 + 1;

			iData.push_back(v0);
			iData.push_back(v2);
			iData.push_back(v1);

			iData.push_back(v1);
			iData.push_back(v2);
			iData.push_back(v3);
		}
	}

	// CYLINDER	
	unsigned cylStart = vData.size();
	float yShift = cylHeight;

	// bottom center
	v.pos = glm::vec3(0, -yShift, 0);
	v.normal = glm::vec3(0, -1, 0);
	vData.push_back(v);

	unsigned bottomCenter = vData.size() - 1;

	// bottom rim
	for (unsigned slice = 0; slice <= numSlice; ++slice)
	{
		float theta = glm::radians(slice * (360.f / numSlice));
		v.pos = glm::vec3(radius * cos(theta), -yShift, radius * sin(theta));
		v.normal = glm::vec3(0, -1, 0);
		vData.push_back(v);
	}

	// bottom fan
	for (unsigned slice = 1; slice <= numSlice; ++slice)
	{
		iData.push_back(bottomCenter);
		iData.push_back(bottomCenter + slice);
		iData.push_back(bottomCenter + slice + 1);
	}

	// side vertices
	unsigned sideStart = vData.size();

	for (unsigned slice = 0; slice <= numSlice; ++slice)
	{
		float theta = glm::radians(slice * (360.f / numSlice));
		float cx = cos(theta), cz = sin(theta);

		// bottom
		v.pos = glm::vec3(radius * cx, -yShift, radius * cz);
		v.normal = glm::vec3(cx, 0, cz);
		vData.push_back(v);

		// top
		v.pos = glm::vec3(radius * cx, 0, radius * cz);
		v.normal = glm::vec3(cx, 0, cz);
		vData.push_back(v);
	}

	// side triangles
	for (unsigned slice = 0; slice < numSlice; ++slice)
	{
		GLuint i0 = sideStart + slice * 2;
		GLuint i1 = i0 + 1;
		GLuint i2 = i0 + 2;
		GLuint i3 = i0 + 3;

		iData.push_back(i0);
		iData.push_back(i1);
		iData.push_back(i2);

		iData.push_back(i2);
		iData.push_back(i1);
		iData.push_back(i3);
	}
	Mesh* mesh = new Mesh(meshName);

	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vData.size() * sizeof(Vertex), &vData[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, iData.size() * sizeof(GLuint), &iData[0], GL_STATIC_DRAW);

	mesh->indexSize = iData.size();
	mesh->mode = Mesh::DRAW_TRIANGLES;
	return mesh;
}


Mesh* MeshBuilder::GenerateOBJ(const std::string& meshName, const
	std::string& file_path)
{
	// Read vertices, texcoords & normals from OBJ
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;

	bool success = LoadOBJ(file_path.c_str(), vertices, uvs, normals);
	if (!success) { return NULL; }

	// Index the vertices, texcoords & normals properly
	std::vector<Vertex> vertex_buffer_data;
	std::vector<GLuint> index_buffer_data;
	IndexVBO(vertices, uvs, normals, index_buffer_data, vertex_buffer_data);

	Mesh* mesh = new Mesh(meshName);

	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() * sizeof(Vertex), &vertex_buffer_data[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_data.size() * sizeof(GLuint), &index_buffer_data[0], GL_STATIC_DRAW);
	mesh->indexSize = index_buffer_data.size();
	mesh->mode = Mesh::DRAW_TRIANGLES;

	return mesh;
}


Mesh* MeshBuilder::GenerateOBJMTL(const std::string& meshName,
	const std::string& file_path, const std::string& mtl_path)
{
	//Read vertices, texcoords & normals from OBJ
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	std::vector<Material> materials;

	bool success = LoadOBJMTL(file_path.c_str(), mtl_path.c_str(), vertices, uvs, normals, materials);
	if (!success) return NULL;

	//Index the vertices, texcoords & normals properly
	std::vector<Vertex> vertex_buffer_data;
	std::vector<GLuint> index_buffer_data;
	IndexVBO(vertices, uvs, normals, index_buffer_data, vertex_buffer_data);

	Mesh* mesh = new Mesh(meshName);

	for (Material& material : materials) 
		mesh->materials.push_back(material);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() * sizeof(Vertex), &vertex_buffer_data[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_data.size() * sizeof(GLuint), &index_buffer_data[0], GL_STATIC_DRAW);
	mesh->indexSize = index_buffer_data.size();
	mesh->mode = Mesh::DRAW_TRIANGLES;
	return mesh;
}


Mesh* MeshBuilder::GenerateText(const std::string& meshName, unsigned numRow, unsigned numCol)
{
	Vertex v;
	std::vector<Vertex> vertex_buffer_data;
	std::vector<unsigned> index_buffer_data;

	float width = 1.f / numCol;
	float height = 1.f / numRow;
	unsigned offset = 0;

	for (unsigned row = 0; row < numRow; ++row)
	{
		for (unsigned col = 0; col < numCol; ++col)
		{
			// Go to Step 6 for the implementation
			v.pos = glm::vec3(0.5f, 0.5f, 0.f);
			v.normal = glm::vec3(0, 0, 1);
			v.texCoord = glm::vec2(width * (col + 1), height * (numRow - row));
			vertex_buffer_data.push_back(v);
			v.pos = glm::vec3(-0.5f, 0.5f, 0.f);
			v.normal = glm::vec3(0, 0, 1);
			v.texCoord = glm::vec2(width * (col + 0), height * (numRow - row));
			vertex_buffer_data.push_back(v);
			v.pos = glm::vec3(-0.5f, -0.5f, 0.f);
			v.normal = glm::vec3(0, 0, 1);
			v.texCoord = glm::vec2(width * (col + 0), height * (numRow - 1 - row));
			vertex_buffer_data.push_back(v);
			v.pos = glm::vec3(0.5f, -0.5f, 0.f);
			v.normal = glm::vec3(0, 0, 1);
			v.texCoord = glm::vec2(width * (col + 1), height * (numRow - 1 - row));
			vertex_buffer_data.push_back(v);
			index_buffer_data.push_back(0 + offset);
			index_buffer_data.push_back(1 + offset);
			index_buffer_data.push_back(2 + offset);
			index_buffer_data.push_back(0 + offset);
			index_buffer_data.push_back(2 + offset);
			index_buffer_data.push_back(3 + offset);
			offset += 4;
		}
	}

	Mesh* mesh = new Mesh(meshName);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() * sizeof(Vertex), &vertex_buffer_data[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_data.size() * sizeof(GLuint), &index_buffer_data[0], GL_STATIC_DRAW);

	mesh->mode = Mesh::DRAW_TRIANGLES;
	mesh->indexSize = index_buffer_data.size();
	return mesh;
}
