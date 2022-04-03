#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <streambuf>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GLM/glm.hpp>


#include <omp.h>
#include <immintrin.h>

#include <chrono>

constexpr unsigned int ITERATIONS = 256;
constexpr unsigned int WIDTH = 1200;
constexpr unsigned int HEIGHT = 1200;
constexpr double XWIDTH = 2.5;
constexpr double YWIDTH = 2.5;

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
mandel_sse2(float* pixels, unsigned int width, unsigned int height, double xstart, double xend, double ystart, double yend, unsigned int iterations)
{
	__m256d xmin = _mm256_set1_pd(xstart);
	__m256d ymin = _mm256_set1_pd(ystart);
	__m256d xscale = _mm256_set1_pd((xend - xstart) / width);
	__m256d yscale = _mm256_set1_pd((yend - ystart) / height);
	__m256d threshold = _mm256_set1_pd(4);
	__m256d one = _mm256_set1_pd(1);
	__m256d iter_scale = _mm256_set1_pd(1.0 / iterations);

	#pragma omp parallel for
	for (int y = 0; y < height; y++) {
		double res[4];
		hsv color;
		color.s = 1.0f;
		for (unsigned int x = 0; x < width; x += 4) {
			__m256d mx = _mm256_set_pd(x + 3, x + 2, x + 1, x + 0);
			__m256d my = _mm256_set1_pd(y);
			__m256d cr = _mm256_add_pd(_mm256_mul_pd(mx, xscale), xmin);
			__m256d ci = _mm256_add_pd(_mm256_mul_pd(my, yscale), ymin);
			__m256d zr = cr;
			__m256d zi = ci;
			unsigned int k = 1;
			__m256d mk = _mm256_set1_pd(k);
			while (++k < iterations) {
				/* Compute z1 from z0 */
				__m256d zr2 = _mm256_mul_pd(zr, zr);
				__m256d zi2 = _mm256_mul_pd(zi, zi);
				__m256d zrzi = _mm256_mul_pd(zr, zi);
				/* zr1 = zr0 * zr0 - zi0 * zi0 + cr */
				/* zi1 = zr0 * zi0 + zr0 * zi0 + ci */
				zr = _mm256_add_pd(_mm256_sub_pd(zr2, zi2), cr);
				zi = _mm256_add_pd(_mm256_add_pd(zrzi, zrzi), ci);

				/* Increment k */
				zr2 = _mm256_mul_pd(zr, zr);
				zi2 = _mm256_mul_pd(zi, zi);
				__m256d mag2 = _mm256_add_pd(zr2, zi2);
				__m256d mask = _mm256_cmp_pd(mag2, threshold, _CMP_LT_OQ);
				mk = _mm256_add_pd(_mm256_and_pd(mask, one), mk);

				/* Early bailout? */
				if (_mm256_movemask_pd(mask) == 0)
					break;
			}
			mk = _mm256_add_pd(mk, one);
			mk = _mm256_mul_pd(mk, iter_scale);

			_mm256_store_pd(res, mk);

			for (int i = 0; i < 4; i++) {
				double mk_fpu = res[i];
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

double zoom = 1.0f;

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	// close window when ESC has been pressed
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	//if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
	//	zoom *= 2.0f;
	//}
	//if (key == GLFW_KEY_B && action == GLFW_PRESS) {
	//	zoom /= 2.0f;
	//}

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
	auto p =  std::make_pair((int)xpos, (int)(HEIGHT - ypos));
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

void updateMandel(float* pixels, double centerX, double centerY, double mWidth, double mHeight, unsigned int width, unsigned int height, unsigned int iterations) {

	double xStart = centerX - (mWidth / 2.0);
	double xEnd = centerX + (mWidth / 2.0);

	double yStart = centerY - (mHeight / 2.0);
	double yEnd = centerY + (mHeight / 2.0);

	mandel_sse2(pixels, width, height, xStart, xEnd, yStart, yEnd, iterations);
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

	mandel_sse2(buffer, width, height, -2.0, 0.5, -1.25, 1.25, ITERATIONS);
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

void updateZoom(GLFWwindow* window, double deltaSeconds) {
	double zoomFactor = 1 + 1.5 * deltaSeconds;
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		zoom *= zoomFactor;
	}

	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
		zoom /= zoomFactor;
	}
}


double lastCenterX = -0.75f;
double lastCenterY = 0.0f;
double clickedMousePosX = 0.0f;
double clickedMousePosY = 0.0f;
std::pair<double, double> getCenter(bool lastMousePressedState, double currCenterX, double currCenterY, double xWidth, double yWidth) {
	if (!lastMousePressedState && mouseLeftPressed) {
		lastCenterX = currCenterX;
		lastCenterY = currCenterY;
		clickedMousePosX = mouseCoordx;
		clickedMousePosY = mouseCoordy;
		return std::make_pair(currCenterX, currCenterY);
	} else if (lastMousePressedState && mouseLeftPressed) {
		double mouseTranslationX = (mouseCoordx - clickedMousePosX) / WIDTH * xWidth;
		double mouseTranslationY = (mouseCoordy - clickedMousePosY) / HEIGHT * yWidth;
		return std::make_pair(lastCenterX - mouseTranslationX, lastCenterY - mouseTranslationY);
	}
	return std::make_pair(currCenterX, currCenterY);
}

int main(void) {
	// create a window with the specified width, height and title and initialize OpenGL
	unsigned int width = WIDTH;
	unsigned int height = HEIGHT;
	GLFWwindow* window = initialize(width, height, "Mandelbrot Set Viewer");
	GLuint shaderProgram = createShaderProgram(
		ASSETS_PATH"/shaders/test.vert.glsl",
		ASSETS_PATH"/shaders/test.frag.glsl");

	float* pixels = generateTextureBuffer(width, height);
	GLuint textureId = createTexture(width, height, pixels);
	GLuint vao = createBuffers();

	double currCenterX = -0.75;
	double currCenterY = 0.0;
	double xWidth = 2.5;
	double yWidth = 2.5;
	bool currMousePressedState = false;
	
	auto lastFrameStart = std::chrono::high_resolution_clock::now();
	auto currentFrameStart = std::chrono::high_resolution_clock::now();

	// loop until the user presses ESC or the window is closed programatically
	while (!glfwWindowShouldClose(window)) {

		currentFrameStart = std::chrono::high_resolution_clock::now();
		auto deltaSeconds = (currentFrameStart - lastFrameStart).count() / 1000000000.0;
		//std::cout << deltaSeconds << "\n";

		// clear the back buffer with the specified color and the depth buffer with 1
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		std::pair<double, double> center = getCenter(currMousePressedState, currCenterX, currCenterY, xWidth, yWidth);
		currCenterX = center.first;
		currCenterY = center.second;
		currMousePressedState = mouseLeftPressed;
		xWidth = 2.5f / zoom;
		yWidth = 2.5f / zoom;
		updateMandel(pixels, center.first, center.second, xWidth, yWidth, width, height, ITERATIONS);
		// render to back buffer
		render(shaderProgram, vao, textureId);

		// switch front and back buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
		updateZoom(window, deltaSeconds);

		if (mouseLeftPressed) {
			drawPixels(mouseCoordx, mouseCoordy, textureId);
		}

		lastFrameStart = currentFrameStart;
	}

	// clean up all created objects
	cleanup(window, shaderProgram, vao);
	delete pixels;

	// program exits properly
	exit(EXIT_SUCCESS);
}