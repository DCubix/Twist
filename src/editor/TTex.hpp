#ifndef T_TEX_H
#define T_TEX_H

#include <string>

#include "../glad/glad.h"
#include "../stb/stb_image.h"

class TTex { 
public:
	TTex() {}

	TTex(const std::string& fileName) {
		int comp;
		uint8_t* data = stbi_load(fileName.c_str(), &m_width, &m_height, &comp, STBI_rgb_alpha);
		if (data) {
			glGenTextures(1, &m_id);
			glBindTexture(GL_TEXTURE_2D, m_id);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glBindTexture(GL_TEXTURE_2D, 0);
			stbi_image_free(data);
		}
	}

	TTex(uint8_t* pngdata, int len) {
		int comp;
		uint8_t* data = stbi_load_from_memory(pngdata, len, &m_width, &m_height, &comp, STBI_rgb_alpha);
		if (data) {
			glGenTextures(1, &m_id);
			glBindTexture(GL_TEXTURE_2D, m_id);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glBindTexture(GL_TEXTURE_2D, 0);
			stbi_image_free(data);
		}
	}

	int width() const { return m_width; }
	int height() const { return m_height; }

	GLuint id() const { return m_id; }

private:
	GLuint m_id;
	int m_width, m_height;
};

#endif // T_TEX_H