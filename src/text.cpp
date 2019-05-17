#include <iostream>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "utils.hpp"
#include "ft2build.h"
#include "text.hpp"
// the FT_FREETYPE_H macro uses <> so the linter cannot find the header unless it scans the entire src dir recursively
#include FT_FREETYPE_H  

FontRenderer::FontRenderer(std::string fontFileName, GLuint screenWidth, GLuint screenHeight) {
	buildFont(fontFileName);
	initGL();
	setScreenSize(screenWidth, screenHeight);
}

void FontRenderer::RenderText(std::string text, GLfloat initX, GLfloat initY, GLfloat scale, glm::vec3 color)
{
	glDisable(GL_DEPTH_TEST);
	GLfloat x = initX,
					y = initY;
	// Activate corresponding render state	
	glUseProgram(_shaderProgram);
	glUniform3f(glGetUniformLocation(_shaderProgram, "textColor"), color.x, color.y, color.z);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);

	// Iterate through all characters
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		if(*c == '\n') {
			y -= _characters['I'].Size.y * 1.3 * scale;
			x = initX;
			continue;
		}

		Character ch = _characters[*c];

		GLfloat xpos = x + ch.Bearing.x * scale;
		GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		GLfloat w = ch.Size.x * scale;
		GLfloat h = ch.Size.y * scale;
		// Update VBO for each character
		GLfloat vertices[6][4] = {
			{ xpos,     ypos + h,   0.0, 0.0 },            
			{ xpos,     ypos,       0.0, 1.0 },
			{ xpos + w, ypos,       1.0, 1.0 },

			{ xpos,     ypos + h,   0.0, 0.0 },
			{ xpos + w, ypos,       1.0, 1.0 },
			{ xpos + w, ypos + h,   1.0, 0.0 }           
		};
		// Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void FontRenderer::setScreenSize(GLuint screenWidth, GLuint screenHeight) {
	_projection	= glm::ortho(0.0f, (float)screenWidth, 0.0f, (float)screenHeight);

	glUseProgram(_shaderProgram);
	GLint projLoc = glGetUniformLocation(_shaderProgram, "projection");
	if(projLoc == -1)
		std::cerr << "Could not set uniform value projection - uniform not found.\n";
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(_projection));
}

void FontRenderer::buildFont(std::string fontFileName) {
	Characters characters;
	FT_Library ft;
	if (FT_Init_FreeType(&ft))
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

	FT_Face face;
	if (FT_New_Face(ft, fontFileName.c_str(), 0, &face))
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;  

	FT_Set_Pixel_Sizes(face, 0, 48);  

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

	for (GLubyte c = 0; c < 128; c++)
	{
		// Load character glyph 
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
			continue;
		}
		// Generate texture
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RED,
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows,
				0,
				GL_RED,
				GL_UNSIGNED_BYTE,
				face->glyph->bitmap.buffer
				);
		// Set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Now store character for later use
		Character character = {
			texture, 
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x
		};
		_characters.insert(std::pair<GLchar, Character>(c, character));
	}
}

void FontRenderer::initGL() {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

	_shaderProgram = loadShaderProgram({
			{ GL_VERTEX_SHADER, "../data/shaders/text.vert" },
			{ GL_FRAGMENT_SHADER, "../data/shaders/text.frag" },
			});
	if(!_shaderProgram)
		std::cerr << "Failed to load shader program\n";

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);  
}
