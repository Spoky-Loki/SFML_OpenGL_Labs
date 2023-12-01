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
GLuint VBO_color;
GLuint VBO_texture;

sf::Image ourTextureImage;
GLuint textureHandle;

GLint Unif_Coef;

float coef = 0.05f;

struct vertex {
	GLfloat x;
	GLfloat y;
	GLfloat z;
};

// Исходный код вершинного шейдера
const char* VertexShaderSource = R"(
#version 330 core
in vec3 coord;
in vec4 color;
in vec2 texCoord;
    
out vec4 vert_color;
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
    vert_color = color;
	TexCoord = texCoord;
}
)";

// Исходный код фрагментного шейдера
const char* FragShaderSource = R"(
#version 330 core
in vec4 vert_color;
in vec2 TexCoord;

uniform sampler2D Texture;
uniform float coef;

out vec4 color;
void main() {
    color = mix(texture(Texture, TexCoord), vec4(vert_color), coef);
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
	if (!ourTextureImage.loadFromFile("texture.png"))
	{
		return false;
	}

	glGenTextures(1, &textureHandle);
	glBindTexture(GL_TEXTURE_2D, textureHandle);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, ourTextureImage.getSize().x, ourTextureImage.getSize().y, 0, GL_RGB, GL_UNSIGNED_BYTE,
		ourTextureImage.getPixelsPtr());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void InitVBO() {
	glGenBuffers(1, &VBO_position);
	glGenBuffers(1, &VBO_color);
	glGenBuffers(1, &VBO_texture);

	// Вершины нашего тетраэдера
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

	float colors[36][4] = 
	{
		{ 1.0, 0.0, 1.0, 1.0 },{ 1.0, 0.0, 0.0, 1.0 },{ 1.0, 1.0, 0.0, 1.0 },
		{ 1.0, 1.0, 0.0, 1.0 },{ 1.0, 1.0, 1.0, 1.0 },{ 1.0, 0.0, 1.0, 1.0 },//невидимый

		{ 1.0, 1.0, 1.0, 1.0 },{ 0.0, 1.0, 0.0, 1.0 },{ 1.0, 1.0, 0.0, 1.0 },
		{ 0.0, 1.0, 0.0, 1.0 },{ 1.0, 1.0, 1.0, 1.0 },{ 0.0, 1.0, 1.0, 1.0 },

		{ 1.0, 1.0, 0.0, 1.0 },{ 1.0, 0.0, 0.0, 1.0 },{ 1.0, 0.0, 0.0, 1.0 },//невидимый одна непонятная цвет
		{ 0.0, 0.0, 0.0, 1.0 },{ 0.0, 1.0, 0.0, 1.0 },{ 1.0, 1.0, 0.0, 1.0 },

		{ 1.0, 1.0, 1.0, 1.0 },{ 0.0, 0.0, 1.0, 1.0 },{ 1.0, 0.0, 1.0, 1.0 },
		{ 0.0, 0.0, 1.0, 1.0 },{ 1.0, 1.0, 1.0, 1.0 },{ 0.0, 1.0, 1.0, 1.0 },

		{ 0.0, 1.0, 1.0, 1.0 },{ 0.0, 0.0, 1.0, 1.0 },{ 0.0, 0.0, 0.0, 1.0 },//невидимый одна непонятная цветовая грань
		{ 0.0, 0.0, 0.0, 1.0 },{ 0.0, 1.0, 0.0, 1.0 },{ 0.0, 1.0, 1.0, 1.0 },

		{ 1.0, 1.0, 1.0, 1.0 },{ 1.0, 0.0, 0.0, 1.0 },{ 1.0, 0.0, 1.0, 1.0 },
		{ 1.0, 0.0, 0.0, 1.0 },{ 1.0, 1.0, 1.0, 1.0 },{ 1.0, 1.0, 0.0, 1.0 },
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

	// Передаем вершины в буфер
	glBindBuffer(GL_ARRAY_BUFFER, VBO_position);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_color);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_texture);
	glBufferData(GL_ARRAY_BUFFER, sizeof(textures), textures, GL_STATIC_DRAW);
	checkOpenGLerror();
}

void InitShader() {
	// Создаем вершинный шейдер
	GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
	// Передаем исходный код
	glShaderSource(vShader, 1, &VertexShaderSource, NULL);
	// Компилируем шейдер
	glCompileShader(vShader);
	std::cout << "vertex shader \n";
	// Функция печати лога шейдера
	ShaderLog(vShader); //Пример функции есть в лабораторной
	// Создаем фрагментный шейдер
	GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
	// Передаем исходный код
	glShaderSource(fShader, 1, &FragShaderSource, NULL);
	// Компилируем шейдер
	glCompileShader(fShader);
	std::cout << "fragment shader \n";
	// Функция печати лога шейдера
	ShaderLog(fShader);
	// Создаем программу и прикрепляем шейдеры к ней
	Program = glCreateProgram();
	glAttachShader(Program, vShader);
	glAttachShader(Program, fShader);
	// Линкуем шейдерную программу
	glLinkProgram(Program);
	// Проверяем статус сборки
	int link_ok;
	glGetProgramiv(Program, GL_LINK_STATUS, &link_ok);
	if (!link_ok) {
		std::cout << "error attach shaders \n";
		return;
	}
	// Вытягиваем ID атрибута из собранной программы
	Attrib_vertex = glGetAttribLocation(Program, "coord");
	if (Attrib_vertex == -1) {
		std::cout << "could not bind attrib coord" << std::endl;
		return;
	}

	Attrib_color = glGetAttribLocation(Program, "color");
	if (Attrib_color == -1)
	{
		std::cout << "could not bind attrib color" << std::endl;
		return;
	}

	Attrib_texture = glGetAttribLocation(Program, "texCoord");
	if (Attrib_texture == -1)
	{
		std::cout << "could not bind attrib texCoord" << std::endl;
		return;
	}

	Unif_Coef = glGetUniformLocation(Program, "coef");
	if (Unif_Coef < 0 || Unif_Coef > 1)
	{
		std::cout << "could not bind uniform coef" << std::endl;
		return;
	}

	checkOpenGLerror();
}

void Init() {
	// Шейдеры
	InitShader();
	// Текстура
	InitTextures();
	// Вершинный буфер
	InitVBO();
	// Включаем проверку глубины
	glEnable(GL_DEPTH_TEST);
}

void Draw() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(Program); // Устанавливаем шейдерную программу текущей

	glUniform1f(Unif_Coef, coef);

	glEnableVertexAttribArray(Attrib_vertex); // Включаем массив атрибутов
	glEnableVertexAttribArray(Attrib_color);
	glEnableVertexAttribArray(Attrib_texture);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_position);
	glVertexAttribPointer(Attrib_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_color);
	glVertexAttribPointer(Attrib_color, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_texture);
	glVertexAttribPointer(Attrib_texture, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindTexture(GL_TEXTURE_2D, textureHandle);
	glBindBuffer(GL_ARRAY_BUFFER, 0); // Отключаем VBO
	glDrawArrays(GL_TRIANGLES, 0, 36); // Передаем данные на видеокарту(рисуем)
	glBindVertexArray(0);

	glDisableVertexAttribArray(Attrib_vertex); // Отключаем массив атрибутов
	glDisableVertexAttribArray(Attrib_color);
	glDisableVertexAttribArray(Attrib_texture);

	glUseProgram(0); // Отключаем шейдерную программу
	checkOpenGLerror();
}

// Освобождение буфера
void ReleaseVBO() {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &VBO_position);
	glDeleteBuffers(1, &VBO_color);
	glDeleteBuffers(1, &VBO_texture);
}

// Освобождение шейдеров
void ReleaseShader() {
	// Передавая ноль, мы отключаем шейдерную программу
	glUseProgram(0);
	// Удаляем шейдерную программу
	glDeleteProgram(Program);
}

void Release() {
	// Шейдеры
	ReleaseShader();
	// Вершинный буфер
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
				case (sf::Keyboard::Up): coef += 0.05f; break;
				case (sf::Keyboard::Down): coef -= 0.05f; break;
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