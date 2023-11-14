#pragma once

#include<glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include<iostream>

class FrameBuffer
{
public:
	GLuint fboID = 0;
	GLuint TextureID = 0;
	GLuint rboID = 0;

	GLuint width = 800;
	GLuint height = 600;

	FrameBuffer()
	{
		// Gen frame buffer & texture
		glGenFramebuffers(1, &this->fboID);
		glBindFramebuffer(GL_FRAMEBUFFER, this->fboID);

		glGenTextures(1, &this->TextureID);
		glBindTexture(GL_TEXTURE_2D, this->TextureID);

		glGenRenderbuffers(1, &this->rboID);
		glBindRenderbuffer(GL_RENDERBUFFER, this->rboID);

		// Texture parameters
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TextureID, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->rboID);


		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
private:
};
