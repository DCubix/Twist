#ifndef T_TEX_H
#define T_TEX_H

#include <string>

#include "../glad/glad.h"
#include "../stb/stb_image.h"

class TTex {
public:
	TTex() = default;

	TTex(const std::string& fileName);
	TTex(uint8_t* pngdata, int len);

	int width() const { return m_width; }
	int height() const { return m_height; }

	GLuint id() const { return m_id; }

private:
	GLuint m_id;
	int m_width, m_height;
};

#endif // T_TEX_H