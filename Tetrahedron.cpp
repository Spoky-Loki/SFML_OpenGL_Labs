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

GLint Unif_xmove;
GLint Unif_ymove;

GLuint VBO_position;
GLuint VBO_color;

struct vertex {
	GLfloat x;
	GLfloat y;
	GLfloat z;
};

float moveX = 0;
float moveY = 0;

// Исходный код вершинного шейдера
const char* VertexShaderSource = R"(
#version 330 core
in vec3 coord;
in vec4 color;

uniform float x_move;
uniform float y_move;
    
out vec4 vert_color;

void main() {
	vec3 position = vec3(coord) + vec3(x_move, y_move, 0);
	gl_Position = vec4(position[0], position[1], 0.0, 1.0);
    vert_color = color;
}
)";

// Исходный код фрагментного шейдера
const char* FragShaderSource = R"(
#version 330 core
in vec4 vert_color;

out vec4 color;
void main() {
    color = vert_color;
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

void InitVBO() {
	glGenBuffers(1, &VBO_position);
	glGenBuffers(1, &VBO_color);

	// Вершины нашего тетраэдера
	vertex triangle[] = {
		{ 0.2, 0.45, -0.5 }, { -0.6, 0, -0.5 }, { 0, 0, 0.5 },
		{ -0.6, 0, -0.5 }, { 0, 0, 0.5 }, { 0.2, -0.45, -0.5 },
		{ 0, 0, 0.5 }, { 0.2, 0.45, -0.5 }, { 0.2, -0.45, -0.5 },

	};

	float colors[9][4] = {
		{ 0.0, 0.0, 1.0, 1.0 }, { 1.0, 0.0, 0.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 },
		{ 1.0, 0.0, 0.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 1.0, 0.0, 1.0 },
		{ 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 }, { 0.0, 1.0, 0.0, 1.0 },
	};

	// Передаем вершины в буфер
	glBindBuffer(GL_ARRAY_BUFFER, VBO_position);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_color);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
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

	Unif_xmove = glGetUniformLocation(Program, "x_move");
	if (Unif_xmove == -1)
	{
		std::cout << "could not bind uniform x_move" << std::endl;
		return;
	}

	Unif_ymove = glGetUniformLocation(Program, "y_move");
	if (Unif_ymove == -1)
	{
		std::cout << "could not bind uniform y_move" << std::endl;
		return;
	}

	checkOpenGLerror();
}

void Init() {
	// Шейдеры
	InitShader();
	// Вершинный буфер
	InitVBO();
	// Включаем проверку глубины
	glEnable(GL_DEPTH_TEST);
}

void Draw() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(Program); // Устанавливаем шейдерную программу текущей

	glUniform1f(Unif_xmove, moveX);
	glUniform1f(Unif_ymove, moveY);

	glEnableVertexAttribArray(Attrib_vertex); // Включаем массив атрибутов
	glEnableVertexAttribArray(Attrib_color);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_position);
	glVertexAttribPointer(Attrib_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_color);
	glVertexAttribPointer(Attrib_color, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0); // Отключаем VBO
	glDrawArrays(GL_TRIANGLES, 0, 9); // Передаем данные на видеокарту(рисуем)

	glDisableVertexAttribArray(Attrib_vertex); // Отключаем массив атрибутов
	glDisableVertexAttribArray(Attrib_color);

	glUseProgram(0); // Отключаем шейдерную программу
	checkOpenGLerror();
}

// Освобождение буфера
void ReleaseVBO() {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &VBO_position);
	glDeleteBuffers(1, &VBO_color);
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
				case (sf::Keyboard::Up): moveY += 0.1f; break;
				case (sf::Keyboard::Down): moveY -= 0.1f; break;
				case (sf::Keyboard::Left): moveX -= 0.1f; break;
				case (sf::Keyboard::Right): moveX += 0.1f; break;
				default: break;
				}
			}
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		Draw();
		window.display();
	}
	Release();
	return 0;
}