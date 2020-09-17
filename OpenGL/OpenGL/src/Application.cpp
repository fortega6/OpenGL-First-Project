#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCALL(x) GLClearError();\
	x;\
	ASSERT(GLLogCall(#x, __FILE__, __LINE__))

static void GLClearError()
{
	while (glGetError() != GL_NO_ERROR);
}

static bool GLLogCall(const char* function, const char* file, int line)
{
	while (GLenum error = glGetError())
	{
		std::cout << "[OpenGL Error] (" << error << "): " << function << 
			" " << file << ":" << line << std::endl;
		return false;
	}
	return true;
}

struct ShaderProgramSource
{
	std::string VertexSource;
	std::string FragmentSource;
};

static ShaderProgramSource ParceShader(const std::string& filepath)
{
	std::ifstream stream(filepath);

	enum ShaderType
	{
		NONE = -1, VERTEX = 0, FRAGMENT = 1
	};

	std::string line;
	std::stringstream ss[2];
	ShaderType type = ShaderType::NONE;
	while (getline(stream, line))
	{
		if (line.find("#shader") != std::string::npos)
		{
			if (line.find("vertex") != std::string::npos)
				type = ShaderType::VERTEX;
			else if (line.find("fragment") != std::string::npos)
				type = ShaderType::FRAGMENT;
		}
		else
		{
			ss[(int)type] << line << '\n';
		}
	}

	return { ss[0].str(), ss[1].str() };
}

static int CompileShader(unsigned int type, const std::string& source)
{
	GLCALL(unsigned int id = glCreateShader(type));
	const char* src = source.c_str();
	GLCALL(glShaderSource(id, 1, &src, nullptr));
	GLCALL(glCompileShader(id));
	
	int result;
	GLCALL(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
	if (result == GL_FALSE)
	{
		int length;
		GLCALL(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
		char* message = (char*)alloca(length * sizeof(char));
		GLCALL(glGetShaderInfoLog(id, length, &length, message));
		std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
		std::cout << message << std::endl;
		GLCALL(glDeleteShader(id));
		
		return 0;
	}

	return id;
}

static unsigned int CreateShader(const std::string& vertexShader, const std::string& frangmentShader)
{
	GLCALL(unsigned int program = glCreateProgram());
	GLCALL(unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader));
	GLCALL(unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, frangmentShader));

	GLCALL(glAttachShader(program, vs));
	GLCALL(glAttachShader(program, fs));
	GLCALL(glLinkProgram(program));
	GLCALL(glValidateProgram(program));

	GLCALL(glDeleteShader(vs));
	GLCALL(glDeleteShader(fs));

	return program;
}

int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;


	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	glfwSwapInterval(1);

	if (glewInit() != GLEW_OK)
		std::cout << "Error!" << std::endl;

	std::cout << glGetString(GL_VERSION) << std::endl;

	float positions[8] = {
		-0.5f, -0.5f, // 0 bottom left
		 0.5f, -0.5f, // 1 bottom right
		 0.5f,  0.5f, // 2 top right
		-0.5f,  0.5f  // 3 top left
	};

	unsigned int indices[] = {  // note that we start from 0!
		0, 1, 2,   // first triangle
		2, 3, 0    // second triangle
	};

	unsigned int vao; // vertex array object
	GLCALL(glGenVertexArrays(1, &vao));
	GLCALL(glBindVertexArray(vao));

	unsigned int buffer;
	GLCALL(glGenBuffers(1, &buffer));
	GLCALL(glBindBuffer(GL_ARRAY_BUFFER, buffer));
	GLCALL(glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), positions, GL_STATIC_DRAW));

	GLCALL(glEnableVertexAttribArray(0));
	GLCALL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0));

	unsigned int ibo; // Index buffer object
	GLCALL(glGenBuffers(1, &ibo));
	GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
	GLCALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW));

	ShaderProgramSource source = ParceShader("res/shaders/Basic.shader");
	unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);
	GLCALL(glUseProgram(shader));

	GLCALL(int location = glGetUniformLocation(shader, "u_Color"));
	ASSERT(location != -1);
	glUniform4f(location, 0.8f, 0.3f, 0.8f, 1.0f);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Unbind
	GLCALL(glBindVertexArray(0));
	GLCALL(glUseProgram(0));
	GLCALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

	float r = 0.0f;
	float increment = 0.05f;
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);

		GLCALL(glUseProgram(shader));
		glUniform4f(location, r, 0.3f, 0.8f, 1.0f);

		/*GLCALL(glBindBuffer(GL_ARRAY_BUFFER, buffer));
		GLCALL(glEnableVertexAttribArray(0));
		GLCALL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0));*/
		GLCALL(glBindVertexArray(vao));
		GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));

		GLCALL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

		if (r > 1.0f)
			increment = -0.05f;
		else if (r < 0.0f)
			increment = 0.05f;

		r += increment;

		/*glBegin(GL_TRIANGLES);
		glVertex2f(-0.5f, -0.5f);
		glVertex2f( 0.0f,  0.5f);
		glVertex2f( 0.5f, -0.5f);
		glEnd();*/

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	GLCALL(glDeleteProgram(shader));

	glfwTerminate();
	return 0;
}