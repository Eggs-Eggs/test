#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include<glad/glad.h>
#include <assimp/Importer.hpp>

#include"RenderUtilities/BufferObject.h"
#include"RenderUtilities/Shader.h"
#include"RenderUtilities/Texture.h"

struct Texture
{
	GLuint id;
	std::string type;
	aiString path;
};


class Mesh
{
public:
	/* Mesh Data */
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<Texture> textures;

	GLuint VAO, VBO, EBO;

	// Constructor
	Mesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, std::vector<Texture>& textures)
	{
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;
		// Now that we have all the required data, set the vertex buffers 
		//and its attribute pointers.
		this->setupMesh();
	}

	Mesh(Mesh&& data) noexcept :VAO(data.VAO), VBO(data.VBO), EBO(data.EBO)
	{
		this->vertices = std::move(data.vertices);
		this->indices= std::move(data.indices);
		this->textures = std::move(data.textures);
	}

	// Render the mesh
	void Draw(Shader shader)
	{
		// Bind appropriate textures
		GLuint diffuseNr = 1;
		GLuint specularNr = 1;
		for (GLuint i = 0; i < this->textures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i); // Active proper texture unit before binding
			// Retrieve texture number (the N in diffuse_textureN)
			std::stringstream ss;
			std::string number;
			std::string name = this->textures[i].type;
			if (name == "texture_diffuse")
			{
				ss << diffuseNr++; // Transfer GLuint to stream
			}
			else if (name == "texture_specular")
			{
				ss << specularNr++; // Transfer GLuint to stream
			}

			number = ss.str();
			// Now set the sampler to the correct texture unit
			glUniform1i(glGetUniformLocation(shader.Program, (name + number).c_str()), i);

			// And finally bind the texture
			glBindTexture(GL_TEXTURE_2D, this->textures[i].id);
		}

		glUniform1f(glGetUniformLocation(shader.Program, "material.shininess"), 16.0f);

		// Draw mesh
		glBindVertexArray(this->VAO);
		glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		for (GLuint i = 0; i < this->textures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	void setupMesh()
	{
		// Create buffers/arrays
		glGenVertexArrays(1, &this->VAO);
		glGenBuffers(1, &this->VBO);
		glGenBuffers(1, &this->EBO);
		glBindVertexArray(this->VAO);

		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), &this->vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), &this->indices[0], GL_STATIC_DRAW);

		// Vertex Positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);

		// Vertex Normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Normal));

		// Vertex Texture Coords
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, TexCoords));

		glBindVertexArray(0);
	}
};