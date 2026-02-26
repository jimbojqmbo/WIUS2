
#include "Mesh.h"
#include "GL\glew.h"
#include "Vertex.h"
#include <glm/gtc/type_ptr.hpp>

/******************************************************************************/
/*!
\brief
Default constructor - generate VBO/IBO here

\param meshName - name of mesh
*/
/******************************************************************************/
Mesh::Mesh(const std::string& meshName)
	: name(meshName)
	, mode(DRAW_TRIANGLES)
	, textureID(0)
{
	glGenBuffers(1, &vertexBuffer);
	glGenBuffers(1, &indexBuffer);
}

/******************************************************************************/
/*!
\brief
Destructor - delete VBO/IBO here
*/
/******************************************************************************/
Mesh::~Mesh()
{
	glDeleteBuffers(1, &vertexBuffer);
	glDeleteBuffers(1, &indexBuffer);

	if (textureID > 0)
		glDeleteTextures(1, &textureID);
}

/******************************************************************************/
/*!
\brief
OpenGL render code
*/
/******************************************************************************/
	void Mesh::Render()
	{
		glEnableVertexAttribArray(0); // 1st attribute buffer : positions
		glEnableVertexAttribArray(1); // 2nd attribute buffer : colors
		glEnableVertexAttribArray(2); // 3rd attribute buffer : normal
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

		if (textureID > 0)
			glEnableVertexAttribArray(3);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(glm::vec3));
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3) + sizeof(glm::vec3)));

		if (textureID > 0)
			glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec3)));


		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		if (materials.size() == 0)
		{
			if (mode == DRAW_TRIANGLE_STRIP)
				glDrawElements(GL_TRIANGLE_STRIP, indexSize, GL_UNSIGNED_INT, 0);
			else if (mode == DRAW_LINES)
				glDrawElements(GL_LINES, indexSize, GL_UNSIGNED_INT, 0);
			else
				glDrawElements(GL_TRIANGLES, indexSize, GL_UNSIGNED_INT, 0);
		}
		else
		{
			for (unsigned i = 0, offset = 0; i < materials.size(); ++i)
			{
				Material& material = materials[i];
				glUniform3fv(locationKa, 1, &material.kAmbient.r);
				glUniform3fv(locationKd, 1, &material.kDiffuse.r);
				glUniform3fv(locationKs, 1, &material.kSpecular.r);
				glUniform1f(locationNs, material.kShininess);
				if (mode == DRAW_TRIANGLE_STRIP)
					glDrawElements(GL_TRIANGLE_STRIP, material.size, GL_UNSIGNED_INT, (void*)(offset * sizeof(unsigned)));
				else if (mode == DRAW_LINES)
					glDrawElements(GL_LINES, material.size, GL_UNSIGNED_INT, (void*)(offset * sizeof(unsigned)));
				else
					glDrawElements(GL_TRIANGLES, material.size, GL_UNSIGNED_INT, (void*)(offset * sizeof(unsigned)));
				offset += material.size;
			}
		}

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		if (textureID > 0)
			glDisableVertexAttribArray(3);
		}


	void Mesh::Render(unsigned offset, unsigned count)
	{
		glEnableVertexAttribArray(0); // 1st attribute buffer: vertices
			glEnableVertexAttribArray(1); // 2nd attribute buffer: colors
			glEnableVertexAttribArray(2); // 3rd attribute buffer: normals

			if (textureID > 0)
				glEnableVertexAttribArray(3); // 4th attribute: texture coordinate

			glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(glm::vec3));
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3) + sizeof(glm::vec3)));

		if (textureID > 0)
			glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec3)));

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

		if (mode == DRAW_LINES)
			glDrawElements(GL_LINES, count, GL_UNSIGNED_INT, (void*)(offset * sizeof(GLuint)));
		else if (mode == DRAW_TRIANGLE_STRIP)
			glDrawElements(GL_TRIANGLE_STRIP, count, GL_UNSIGNED_INT, (void*)(offset * sizeof(GLuint)));
		else
			glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, (void*)(offset * sizeof(GLuint)));

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		if (textureID > 0)
			glDisableVertexAttribArray(3);
	
	}

	void Mesh::ComputeAABB()
	{
		if (vertices.empty())
		{
			aabbMinLocal = glm::vec3(0.0f);
			aabbMaxLocal = glm::vec3(0.0f);
			return;
		}

		glm::vec3 mn = vertices[0];
		glm::vec3 mx = vertices[0];
		for (const auto& v : vertices)
		{
			mn.x = glm::min(mn.x, v.x);
			mn.y = glm::min(mn.y, v.y);
			mn.z = glm::min(mn.z, v.z);
			mx.x = glm::max(mx.x, v.x);
			mx.y = glm::max(mx.y, v.y);
			mx.z = glm::max(mx.z, v.z);
		}
		aabbMinLocal = mn;
		aabbMaxLocal = mx;
	}

	std::pair<glm::vec3, glm::vec3> Mesh::GetAABBWorld(const glm::mat4& model) const
	{
		// Transform the 8 local corners and recompute AABB in world space
		glm::vec3 localMin = aabbMinLocal;
		glm::vec3 localMax = aabbMaxLocal;

		glm::vec3 corners[8] = {
			{localMin.x, localMin.y, localMin.z},
			{localMax.x, localMin.y, localMin.z},
			{localMin.x, localMax.y, localMin.z},
			{localMin.x, localMin.y, localMax.z},
			{localMax.x, localMax.y, localMin.z},
			{localMax.x, localMin.y, localMax.z},
			{localMin.x, localMax.y, localMax.z},
			{localMax.x, localMax.y, localMax.z}
		};

		glm::vec3 wMin(FLT_MAX), wMax(-FLT_MAX);
		for (int i = 0; i < 8; ++i)
		{
			glm::vec4 t = model * glm::vec4(corners[i], 1.0f);
			glm::vec3 p = glm::vec3(t) / t.w;
			wMin = glm::min(wMin, p);
			wMax = glm::max(wMax, p);
		}
		return { wMin, wMax };
	}

unsigned Mesh::locationKa;
unsigned Mesh::locationKd;
unsigned Mesh::locationKs;
unsigned Mesh::locationNs;
void Mesh::SetMaterialLoc(unsigned ambient, unsigned diffuse, unsigned specular, unsigned shininess)
{
	locationKa = ambient;
	locationKd = diffuse;
	locationKs = specular;
	locationNs = shininess;
}