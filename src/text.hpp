#ifndef TEXT_HPP_18_12_31_16_48_03
#define TEXT_HPP_18_12_31_16_48_03 
#include <map>
#include <string>
#include <glm/glm.hpp>
#include <GL/gl.h>

/** This class renders text using a freetype font.
 *
 * source: https://learnopengl.com/In-Practice/Text-Rendering
 */
class FontRenderer {
	struct Character {
		GLuint     TextureID;  // ID handle of the glyph texture
		glm::ivec2 Size;       // Size of glyph
		glm::ivec2 Bearing;    // Offset from baseline to left/top of glyph
		GLuint     Advance;    // Offset to advance to next glyph
	};

	using Characters = std::map<GLchar, Character>;

	public:
		FontRenderer(std::string fontFileName, GLuint screenWidth, GLuint screenHeight);

		/** Renders text into current framebuffer.
		 * @param text can contain newlines.
		 * @param starting X coordinate
		 * @param starting Y coordinate - 0 is down.
		 */
		void RenderText(std::string text, GLfloat initX, GLfloat initY, GLfloat scale, glm::vec3 color);

		void setScreenSize(GLuint screenWidth, GLuint screenHeight);

	private:
		void buildFont(std::string fontFileName);
		void initGL();

		Characters _characters;
		GLuint VAO;
		GLuint VBO;
		GLuint _shaderProgram;
		glm::mat4 _projection;
};
#endif /* TEXT_HPP_18_12_31_16_48_03 */
