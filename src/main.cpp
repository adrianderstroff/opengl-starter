#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <streambuf>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GLM/glm.hpp>

#include <xmmintrin.h>

typedef struct {
	double r;       // a fraction between 0 and 1
	double g;       // a fraction between 0 and 1
	double b;       // a fraction between 0 and 1
} rgb;

typedef struct {
	double h;       // angle in degrees
	double s;       // a fraction between 0 and 1
	double v;       // a fraction between 0 and 1
} hsv;

static rgb   hsv2rgb(hsv in);

rgb hsv2rgb(hsv in)
{
	double      hh, p, q, t, ff;
	long        i;
	rgb         out;

	if (in.s <= 0.0) {       // < is bogus, just shuts up warnings
		out.r = in.v;
		out.g = in.v;
		out.b = in.v;
		return out;
	}
	hh = in.h;
	if (hh >= 360.0) hh = 0.0;
	hh /= 60.0;
	i = (long)hh;
	ff = hh - i;
	p = in.v * (1.0 - in.s);
	q = in.v * (1.0 - (in.s * ff));
	t = in.v * (1.0 - (in.s * (1.0 - ff)));

	switch (i) {
	case 0:
		out.r = in.v;
		out.g = t;
		out.b = p;
		break;
	case 1:
		out.r = q;
		out.g = in.v;
		out.b = p;
		break;
	case 2:
		out.r = p;
		out.g = in.v;
		out.b = t;
		break;

	case 3:
		out.r = p;
		out.g = q;
		out.b = in.v;
		break;
	case 4:
		out.r = t;
		out.g = p;
		out.b = in.v;
		break;
	case 5:
	default:
		out.r = in.v;
		out.g = p;
		out.b = q;
		break;
	}
	return out;
}

void
mandel_sse2(float* pixels, unsigned int width, unsigned int height, float xstart, float xend, float ystart, float yend, unsigned int iterations)
{
	__m128 xmin = _mm_set_ps1(xstart);
	__m128 ymin = _mm_set_ps1(ystart);
	__m128 xscale = _mm_set_ps1((xend - xstart) / width);
	__m128 yscale = _mm_set_ps1((yend - ystart) / height);
	__m128 threshold = _mm_set_ps1(4);
	__m128 one = _mm_set_ps1(1);
	__m128 iter_scale = _mm_set_ps1(1.0f / iterations);

	float res[4];
	hsv color;
	color.s = 1.0f;
	for (unsigned int y = 0; y < height; y++) {
		for (unsigned int x = 0; x < width; x += 4) {
			__m128 mx = _mm_set_ps(x + 3, x + 2, x + 1, x + 0);
			__m128 my = _mm_set_ps1(y);
			__m128 cr = _mm_add_ps(_mm_mul_ps(mx, xscale), xmin);
			__m128 ci = _mm_add_ps(_mm_mul_ps(my, yscale), ymin);
			__m128 zr = cr;
			__m128 zi = ci;
			unsigned int k = 1;
			__m128 mk = _mm_set_ps1(k);
			while (++k < iterations) {
				/* Compute z1 from z0 */
				__m128 zr2 = _mm_mul_ps(zr, zr);
				__m128 zi2 = _mm_mul_ps(zi, zi);
				__m128 zrzi = _mm_mul_ps(zr, zi);
				/* zr1 = zr0 * zr0 - zi0 * zi0 + cr */
				/* zi1 = zr0 * zi0 + zr0 * zi0 + ci */
				zr = _mm_add_ps(_mm_sub_ps(zr2, zi2), cr);
				zi = _mm_add_ps(_mm_add_ps(zrzi, zrzi), ci);

				/* Increment k */
				zr2 = _mm_mul_ps(zr, zr);
				zi2 = _mm_mul_ps(zi, zi);
				__m128 mag2 = _mm_add_ps(zr2, zi2);
				__m128 mask = _mm_cmplt_ps(mag2, threshold);
				mk = _mm_add_ps(_mm_and_ps(mask, one), mk);

				/* Early bailout? */
				if (_mm_movemask_ps(mask) == 0)
					break;
			}
			mk = _mm_add_ps(mk, one);
			mk = _mm_mul_ps(mk, iter_scale);

			_mm_store_ps(res, mk);

			for (int i = 0; i < 4; i++) {
				float mk_fpu = res[i];
				color.h = mk_fpu * 360.0f;
				color.v = mk_fpu >= 1.0f? 0.0f : 1.0f;

				rgb colorRGB = hsv2rgb(color);

				pixels[y * (width * 4) + (x + i) * 4 + 0] = colorRGB.r;
				pixels[y * (width * 4) + (x + i) * 4 + 1] = colorRGB.g;
				pixels[y * (width * 4) + (x + i) * 4 + 2] = colorRGB.b;
				pixels[y * (width * 4) + (x + i) * 4 + 3] = 1.0;
			}
		}
	}
}



//________________________________________________CALLBACK_FUNCTIONS_________________________________________________//

static void errorCallback(int error, const char* description) {
	fputs(description, stderr);
}

float zoom = 1.0;
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	// close window when ESC has been pressed
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

}
bool mouseLeftPressed = false;
int mouseCoordx = 0;
int mouseCoordy = 0;

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button == 0) {
		mouseLeftPressed = !mouseLeftPressed;
	}

}

std::pair<int, int> convertMouseToCoordinateSystem(double xpos, double ypos) {
	auto p =  std::make_pair((int)xpos, (int)(480 - ypos));
	return p;
}

static void mousePositionCallback(GLFWwindow* window, double xpos, double ypos) {
	auto tuple = convertMouseToCoordinateSystem(xpos, ypos);
	mouseCoordx = tuple.first;
	mouseCoordy = tuple.second;
}


//_________________________________________________INITIALIZATION____________________________________________________//

GLFWwindow* initialize(int width, int height, std::string title) {
	GLFWwindow* window;
	glfwSetErrorCallback(errorCallback);

	// initialize glfw window
	if (!glfwInit())
		exit(EXIT_FAILURE);

	// we want to use the opengl 3.3 core profile
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// actually create the window
	window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);

	// make sure the window creation was successful
	if (!window) {
		fprintf(stderr, "Failed to open GLFW window.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);

	// initialize glad for loading all OpenGL functions
	if (!gladLoadGL()) {
		printf("Something went wrong!\n");
		exit(-1);
	}

	// print some information about the supported OpenGL version
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	fprintf(stdout, "Renderer: %s\n", renderer);
	fprintf(stdout, "OpenGL version supported %s\n", version);

	// register user callbacks
	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCursorPosCallback(window, mousePositionCallback);

	// set the clear color of the window
	glClearColor(0.3f, 0.3f, 0.3f, 0.0f);

	return window;
}

GLuint createBuffers() {
	// specify the layout of the vertex data, being the vertex position followed by the vertex color
	struct Vertex {
		glm::vec3 pos;
		glm::vec2 color;
	};

	// we specify a triangle with red, green, blue at the tips of the triangle
	Vertex vertexData[] = {
		Vertex{glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
		Vertex{glm::vec3(1.0f, 1.0f, 0.0f), glm::vec2(1.f, 1.f)},
		Vertex{glm::vec3(-1.0f,  1.0f, 0.0f), glm::vec2(0.f, 1.f)},

		Vertex{glm::vec3(1.0f, 1.0f, 0.0f), glm::vec2(1.f, 1.f)},
		Vertex{glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec2(0.f, 0.f)},
		Vertex{glm::vec3(1.0f, -1.0f, 0.0f), glm::vec2(1.f, 0.f)}
	};

	// create the vertex array object that holds all vertex buffers
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// create a vertex buffer that contains all vertex positions and copy the vertex positions into that buffer
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

	// we need to tell the buffer in which format the data is and we need to explicitly enable it
	// first we specify the layout of the vertex position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(struct Vertex, pos)));
	glEnableVertexAttribArray(0);
	// then we specify the layout of the vertex color
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(struct Vertex, color)));
	glEnableVertexAttribArray(1);

	return vao;
}

void drawPixels(unsigned int x, unsigned int y, GLuint texture) {
	float a[16] = { 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0 };
	glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, 2, 2, GL_RGBA, GL_FLOAT, a);
}


GLuint createTexture(unsigned int width, unsigned int height, float* pixels) {
	// create a texture buffer
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, pixels);

	return textureID;
}


GLuint compileShader(std::string path, GLenum shaderType) {
	// grab the contents of the file and store the source code in a string
	std::ifstream filestream(path);
	std::string shaderSource((std::istreambuf_iterator<char>(filestream)),
		std::istreambuf_iterator<char>());

	// create and compile the shader
	GLuint shaderHandle = glCreateShader(shaderType);
	const char* shaderSourcePtr = shaderSource.c_str();
	glShaderSource(shaderHandle, 1, &shaderSourcePtr, NULL);
	glCompileShader(shaderHandle);

	// check if compilation was successful
	int  success;
	char infoLog[512];
	glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shaderHandle, 512, NULL, infoLog);
		std::cerr << "Error while compiling shader\n" << infoLog << std::endl;
	}

	// return the shader handle
	return shaderHandle;
}
GLuint createShaderProgram(std::string vertexShaderPath, std::string fragmentShaderPath) {
	// create and compile shaders
	GLenum vertexShader = compileShader(vertexShaderPath, GL_VERTEX_SHADER);
	GLenum fragmentShader = compileShader(fragmentShaderPath, GL_FRAGMENT_SHADER);

	// create a shader program, attach both shaders and link them together
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	// check for errors while linking the shaders together
	int  success;
	char infoLog[512];
	glGetShaderiv(shaderProgram, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cerr << "Error while linking shaders\n" << infoLog << std::endl;
	}

	// after creating the shader program we don't need the two shaders anymore
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// return the shader program handle
	return shaderProgram;
}

void updateMandel(float* pixels, unsigned int width, unsigned int height) {
	if (mouseLeftPressed) {
		zoom *= 1.05;
	}
	else {
		zoom /= 1.05;
	}
	mandel_sse2(pixels, width, height, -0.75 - ((1 / zoom) * 1.25), -0.75 + (1 / zoom) * 1.25, -1.25 * (1 / zoom), 1.25 * (1 / zoom), 90);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_FLOAT, pixels);
}

//______________________________________________________RENDER_______________________________________________________//

void render(GLuint shaderProgram, GLuint vao, GLuint textureID) {
	glUseProgram(shaderProgram);
	glBindVertexArray(vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

//______________________________________________________CLEANUP______________________________________________________//

void cleanup(GLFWwindow* window, GLuint& shaderProgram, GLuint& vao) {
	// do some custom cleanup here
	glDeleteProgram(shaderProgram);
	glDeleteVertexArrays(1, &vao);

	// lastly destroy the window and terminate glfw
	glfwDestroyWindow(window);
	glfwTerminate();
}

float* generateTextureBuffer(unsigned int width, unsigned int height) {
	float* buffer = new float[width * height * 4];

	mandel_sse2(buffer, width, height, -2.0f, 0.5f, -1.25f, 1.25f, 90);
	//for (unsigned int y = 0; y < height; y++) {
	//	for (unsigned int x = 0; x < width; x ++) {
	//		unsigned int idx = y * width * 4 + (x * 4);

	//		buffer[idx] = (float)x / width;
	//		buffer[idx + 1] = (float)y / height;
	//		buffer[idx + 2] = 1.0f;
	//		buffer[idx + 3] = 1.0f;
	//	}
	//}
	//buffer[0] = 0.0f;
	//buffer[1] = 1.0f;
	//buffer[2] = 0.0f;
	//buffer[3] = 1.0f;

	//buffer[4] = 0.0f;
	//buffer[5] = 1.0f;
	//buffer[6] = 0.0f;
	//buffer[7] = 1.0f;



	return buffer;
}
//_______________________________________________________MAIN________________________________________________________//

int main(void) {
	// create a window with the specified width, height and title and initialize OpenGL
	unsigned int width = 640;
	unsigned int height = 480;
	GLFWwindow* window = initialize(width, height, "OpenGL Starter Project");
	GLuint shaderProgram = createShaderProgram(
		ASSETS_PATH"/shaders/test.vert.glsl",
		ASSETS_PATH"/shaders/test.frag.glsl");

	float* pixels = generateTextureBuffer(width, height);
	GLuint textureId = createTexture(width, height, pixels);
	GLuint vao = createBuffers();

	// loop until the user presses ESC or the window is closed programatically
	while (!glfwWindowShouldClose(window)) {
		// clear the back buffer with the specified color and the depth buffer with 1
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		updateMandel(pixels, width, height);
		// render to back buffer
		render(shaderProgram, vao, textureId);

		// switch front and back buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		if (mouseLeftPressed) {
			drawPixels(mouseCoordx, mouseCoordy, textureId);
		}
	}

	// clean up all created objects
	cleanup(window, shaderProgram, vao);
	delete pixels;

	// program exits properly
	exit(EXIT_SUCCESS);
}