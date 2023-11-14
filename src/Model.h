#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

#include<glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Mesh.h"

class Model
{
public:
	// Constructor, expects a filepath to a 3D model.
	Model(GLchar* path)
	{
		this->loadModel(path);
	}
	// Draws the model, and thus all its meshes
	void Draw(Shader shader)
	{
		for (GLuint i = 0; i < this->meshes.size(); i++)
		{
			this->meshes[i].Draw(shader);
		}
	}

	glm::vec3 position;
	glm::vec3 forward;
	glm::vec3 right;
	glm::vec3 up;

private:
	/* Model Data */
	std::vector<Mesh> meshes;
	std::string directory;
	std::vector<Texture> textures_loaded;

	void loadModel(std::string path)
	{
		// Read file via ASSIMP
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path,aiProcess_Triangulate | aiProcess_FlipUVs);

		// Check for errors
		if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene -> mRootNode)
		{
			std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
			return;
		}

		// Retrieve the directory path of the filepath
		this->directory = path.substr(0, path.find_last_of('/'));

		// Process ASSIMP's root node recursively
		this->processNode(scene->mRootNode, scene);
	}

	void processNode(aiNode* node, const aiScene* scene)
	{
		// Process each mesh located at the current node
		for (GLuint i = 0; i < node->mNumMeshes; i++)
		{
			// The node object only contains indices to index the actual objects in the scene.
			// The scene contains all the data, node is just to keep stuff organized(like relations between nodes).
			
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			this->meshes.push_back(this->processMesh(mesh, scene));
		}

		for (GLuint i = 0; i < node->mNumChildren; i++)
		{
			this->processNode(node->mChildren[i], scene);
		}
	}

	Mesh processMesh(aiMesh* mesh, const aiScene* scene)
	{
		// Data to fill
		std::vector<Vertex> vertices;
		std::vector<GLuint> indices;
		std::vector<Texture> textures;

		// Walk through each of the mesh's vertices
		for (GLuint i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			glm::vec3 vector;

			// Positions
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.Position = vector;

			if (mesh->mNormals != nullptr)
			{
				vector.x = mesh->mNormals[i].x;
				vector.y = mesh->mNormals[i].y;
				vector.z = mesh->mNormals[i].z;
				vertex.Normal = vector;
			}

			// Texture Coordinates
			if (mesh->mTextureCoords[0]) // Does the mesh contain texture coordinates ?
			{
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
			}
			else
			{
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);
			}

			vertices.push_back(vertex);
		}

		for (GLuint i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (GLuint j = 0; j < face.mNumIndices; j++)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		// Process materials
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			// Diffuse maps
			std::vector<Texture> diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

			// Specular maps
			std::vector<Texture> specularMaps = this->loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

			// Specular maps
			std::vector<Texture> heightMaps = this->loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_height");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

			// Specular maps
			std::vector<Texture> ambientMaps = this->loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_ambient");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		}

		return Mesh(vertices, indices, textures);
	}

	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
	{
		std::vector<Texture> textures;

		for (GLuint i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);

			GLboolean skip = false;
			for (GLuint j = 0; j < textures_loaded.size(); j++)
			{
				if (textures_loaded[j].path == str)
				{
					textures.push_back(textures_loaded[j]);
					skip = true;
					break;
				}
			}

			if (!skip)
			{ // If texture hasn't been loaded already, load it
				Texture texture;
				texture.id = TextureFromFile(str.C_Str(), this->directory);
				texture.type = typeName;
				texture.path = str;

				textures.push_back(texture);
				// Store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
				this->textures_loaded.push_back(texture);
			}
		}
		return textures;
	}

	GLint TextureFromFile(const char* path, std::string directory)
	{
		//Generate texture ID and load texture data
		std::string filename = std::string(path);
		filename = directory + '/' + filename;
		GLuint textureID;
		glGenTextures(1, &textureID);
		int width = 0, height = 0;

		cv::Mat img;
		img = cv::imread(filename, cv::IMREAD_COLOR);

		height = img.rows;
		width= img.cols;

		//use fast 4-byte alignment (default anyway) if possible
		glPixelStorei(GL_UNPACK_ALIGNMENT, (img.step & 3) ? 1 : 4);

		//set length of one complete row in data (doesn't need to equal image.cols)
		glPixelStorei(GL_UNPACK_ROW_LENGTH, img.step / img.elemSize());

		// Assign texture to ID
		glBindTexture(GL_TEXTURE_2D, textureID);

		if (img.type() == CV_8UC3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.cols, img.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, img.data);
		else if (img.type() == CV_8UC4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, img.cols, img.rows, 0, GL_BGRA, GL_UNSIGNED_BYTE, img.data);
		
		glGenerateMipmap(GL_TEXTURE_2D);

		// Parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		img.release();

		return textureID;

	}
};
