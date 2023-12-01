#include <GL/glew.h>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
#include <GL/gl.h>
#include <iostream>

using namespace sf;
using namespace std;

GLuint Program;

GLint Attrib_vertex;
GLint Attrib_color;
GLint Attrib_texture;

GLuint VBO_position;
GLuint VBO_texture;

sf::Image ourTextureImage1;
sf::Image ourTextureImage2;
GLuint textureHandle1;
GLuint textureHandle2;

GLint Unif_Coef;

float coef = 0.1f;

struct vertex {
	GLfloat x;
	GLfloat y;
	GLfloat z;
};

// �������� ��� ���������� �������
const char* VertexShaderSource = R"(
#version 330 core
in vec3 coord;
in vec2 texCoord;
    
out vec2 TexCoord;

void main() {
	vec3 position = coord * mat3(
           1, 0, 0,
            0, cos(1), -sin(1),
            0, sin(1), cos(1)
        ) * mat3(
            cos(-1), 0, sin(-1),
            0, 1, 0,
            -sin(-1), 0, cos(-1)
		);
	gl_Position = vec4(position, 1.0);
	TexCoord = texCoord;
}
)";

// �������� ��� ������������ �������
const char* FragShaderSource = R"(
#version 330 core
in vec2 TexCoord;

uniform sampler2D Texture1;
uniform sampler2D Texture2;
uniform float coef;

out vec4 color;
void main() {
    color = mix(texture(Texture1, TexCoord), texture(Texture2, TexCoord), coef);
}
)";

void ShaderLog(unsigned int shader)
{
	int infologLen = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLen);
	if (infologLen > 1)
	{
		int charsWritten = 0;
		std::vector<char> infoLog(infologLen);
		glGetShaderInfoLog(shader, infologLen, &charsWritten, infoLog.data());
		std::cout << "InfoLog: " << infoLog.data() << std::endl;
	}
}

void checkOpenGLerror() {
	GLenum errCode;
	if ((errCode = glGetError()) != GL_NO_ERROR)
		std::cout << "OpenGl error!: " << errCode << std::endl;
}

bool InitTextures()
{
	if (!ourTextureImage1.loadFromFile("texture.png"))
	{
		return false;
	}

	if (!ourTextureImage2.loadFromFile("mem1.png"))
	{
		return false;
	}

	glGenTextures(1, &textureHandle1);
	glBindTexture(GL_TEXTURE_2D, textureHandle1);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, ourTextureImage1.getSize().x, ourTextureImage1.getSize().y, 0, GL_RGB, GL_UNSIGNED_BYTE,
		ourTextureImage1.getPixelsPtr());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glGenTextures(1, &textureHandle2);
	glBindTexture(GL_TEXTURE_2D, textureHandle2);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, ourTextureImage2.getSize().x, ourTextureImage2.getSize().y, 0, GL_RGB, GL_UNSIGNED_BYTE,
		ourTextureImage2.getPixelsPtr());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void InitVBO() {
	glGenBuffers(1, &VBO_position);
	glGenBuffers(1, &VBO_texture);

	// ������� ������ ����������
	vertex triangle[] = {
		{ -0.5, -0.5, +0.5 }, { -0.5, +0.5, +0.5 }, { +0.5, +0.5, +0.5 },
		{ +0.5, +0.5, +0.5 }, { +0.5, -0.5, +0.5 }, { -0.5, -0.5, +0.5 },
		{ -0.5, -0.5, -0.5 }, { +0.5, +0.5, -0.5 }, { -0.5, +0.5, -0.5 },
		{ +0.5, +0.5, -0.5 }, { -0.5, -0.5, -0.5 }, { +0.5, -0.5, -0.5 },
		{ -0.5, +0.5, -0.5 }, { -0.5, +0.5, +0.5 }, { +0.5, +0.5, +0.5 },
		{ +0.5, +0.5, +0.5 }, { +0.5, +0.5, -0.5 }, { -0.5, +0.5, -0.5 },
		{ -0.5, -0.5, -0.5 }, { +0.5, -0.5, +0.5 }, { -0.5, -0.5, +0.5 },
		{ +0.5, -0.5, +0.5 }, { -0.5, -0.5, -0.5 }, { +0.5, -0.5, -0.5 },
		{ +0.5, -0.5, -0.5 }, { +0.5, -0.5, +0.5 }, { +0.5, +0.5, +0.5 },
		{ +0.5, +0.5, +0.5 }, { +0.5, +0.5, -0.5 }, { +0.5, -0.5, -0.5 },
		{ -0.5, -0.5, -0.5 }, { -0.5, +0.5, +0.5 }, { -0.5, -0.5, +0.5 },
		{ -0.5, +0.5, +0.5 }, { -0.5, -0.5, -0.5 }, { -0.5, +0.5, -0.5 },
	};

	float textures[36][2] =
	{
		{1.0, 1.0},{0.0, 1.0},{0.0, 0.0},
		{0.0, 0.0},{1.0, 0.0},{1.0, 1.0},

		{1.0, 1.0},{0.0, 1.0},{0.0, 0.0},
		{0.0, 0.0},{1.0, 0.0},{1.0, 1.0},

		{1.0, 1.0},{0.0, 1.0},{0.0, 0.0},
		{0.0, 0.0},{1.0, 0.0},{1.0, 1.0},

		{1.0, 1.0},{0.0, 1.0},{0.0, 0.0},
		{0.0, 0.0},{1.0, 0.0},{1.0, 1.0},

		{1.0, 1.0},{0.0, 1.0},{0.0, 0.0},
		{0.0, 0.0},{1.0, 0.0},{1.0, 1.0},

		{1.0, 1.0},{0.0, 1.0},{0.0, 0.0},
		{0.0, 0.0},{1.0, 0.0},{1.0, 1.0},
	};

	// �������� ������� � �����
	glBindBuffer(GL_ARRAY_BUFFER, VBO_position);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_texture);
	glBufferData(GL_ARRAY_BUFFER, sizeof(textures), textures, GL_STATIC_DRAW);
	checkOpenGLerror();
}

void InitShader() {
	// ������� ��������� ������
	GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
	// �������� �������� ���
	glShaderSource(vShader, 1, &VertexShaderSource, NULL);
	// ����������� ������
	glCompileShader(vShader);
	std::cout << "vertex shader \n";
	// ������� ������ ���� �������
	ShaderLog(vShader); //������ ������� ���� � ������������
	// ������� ����������� ������
	GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
	// �������� �������� ���
	glShaderSource(fShader, 1, &FragShaderSource, NULL);
	// ����������� ������
	glCompileShader(fShader);
	std::cout << "fragment shader \n";
	// ������� ������ ���� �������
	ShaderLog(fShader);
	// ������� ��������� � ����������� ������� � ���
	Program = glCreateProgram();
	glAttachShader(Program, vShader);
	glAttachShader(Program, fShader);
	// ������� ��������� ���������
	glLinkProgram(Program);
	// ��������� ������ ������
	int link_ok;
	glGetProgramiv(Program, GL_LINK_STATUS, &link_ok);
	if (!link_ok) {
		std::cout << "error attach shaders \n";
		return;
	}
	// ���������� ID �������� �� ��������� ���������
	Attrib_vertex = glGetAttribLocation(Program, "coord");
	if (Attrib_vertex == -1) {
		std::cout << "could not bind attrib coord" << std::endl;
		return;
	}

	Attrib_texture = glGetAttribLocation(Program, "texCoord");
	if (Attrib_texture == -1)
	{
		std::cout << "could not bind attrib texCoord" << std::endl;
		return;
	}

	Unif_Coef = glGetUniformLocation(Program, "coef");
	if (Unif_Coef == -1)
	{
		std::cout << "could not bind uniform coef" << std::endl;
		return;
	}

	checkOpenGLerror();
}

void Init() {
	// �������
	InitShader();
	// ��������
	InitTextures();
	// ��������� �����
	InitVBO();
	// �������� �������� �������
	glEnable(GL_DEPTH_TEST);
}

void Draw() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(Program); // ������������� ��������� ��������� �������

	glUniform1f(Unif_Coef, coef);

	glEnableVertexAttribArray(Attrib_vertex); // �������� ������ ���������
	glEnableVertexAttribArray(Attrib_texture);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_position);
	glVertexAttribPointer(Attrib_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_texture);
	glVertexAttribPointer(Attrib_texture, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureHandle1);
	glUniform1i(glGetUniformLocation(Program, "Texture1"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textureHandle2);
	glUniform1i(glGetUniformLocation(Program, "Texture2"), 1);

	glBindBuffer(GL_ARRAY_BUFFER, 0); // ��������� VBO
	glDrawArrays(GL_TRIANGLES, 0, 36); // �������� ������ �� ����������(������)
	glBindVertexArray(0);

	glDisableVertexAttribArray(Attrib_vertex); // ��������� ������ ���������
	glDisableVertexAttribArray(Attrib_texture);

	glUseProgram(0); // ��������� ��������� ���������
	checkOpenGLerror();
}

// ������������ ������
void ReleaseVBO() {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &VBO_position);
	glDeleteBuffers(1, &VBO_texture);
}

// ������������ ��������
void ReleaseShader() {
	// ��������� ����, �� ��������� ��������� ���������
	glUseProgram(0);
	// ������� ��������� ���������
	glDeleteProgram(Program);
}

void Release() {
	// �������
	ReleaseShader();
	// ��������� �����
	ReleaseVBO();
}

int main() {
	sf::Window window(sf::VideoMode(800, 800), "My OpenGL window", sf::Style::Default, sf::ContextSettings(24));
	window.setVerticalSyncEnabled(true);
	window.setActive(true);
	glewInit();
	Init();
	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) { window.close(); }
			else if (event.type == sf::Event::Resized) { glViewport(0, 0, event.size.width, event.size.height); }
			else if (event.type == sf::Event::KeyPressed) {
				switch (event.key.code) {
				case (sf::Keyboard::Up): if (coef > 1)
											break;
										else
											coef += 0.05f; break;
				case (sf::Keyboard::Down): if (coef < 0)
												break;
											else
												coef -= 0.05f; break;
				default: break;
				}
			}
		}

		Draw();
		window.display();
	}
	Release();
	return 0;
}
