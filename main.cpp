#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>
#include "shader.h"
#include <iostream>
#include "point.h"
#include "link.h"
constexpr auto PPR = 60;
constexpr auto TOTALPOINTS = PPR * PPR;
constexpr auto MAXLINKS = TOTALPOINTS * 2;
constexpr auto DAMPING = 0.99f;
constexpr auto GRAVITY = 0.0001f;
constexpr auto PHYSIC_STEP_PER_UPDATE = 3;
constexpr auto SCREEN_X = 1920;
constexpr auto SCREEN_Y = 1080;
void updatePoints(Point points[])
{
	for (int i = 0; i < TOTALPOINTS; i++)
	{
		Point oldPoint = points[i];
		if (oldPoint.isAnchor) {
			continue;
		}
		points[i].x += (oldPoint.x - oldPoint.oldX) * DAMPING;
		points[i].y += (oldPoint.y - oldPoint.oldY) * DAMPING;
		points[i].y -= GRAVITY;
		points[i].oldX = oldPoint.x;
		points[i].oldY = oldPoint.y;
	}
}

float dist(Point* p1, Point* p2) {
	float dx = p2->x - p1->x;
	float dy = p2->y - p1->y;

	return sqrt(dx * dx + dy * dy);
}

void updateLinks(Link links[])
{
	for (int i = 0; i < MAXLINKS; i++)
	{
		Link link = links[i];
		if (!link.p1) {
			continue;
		}
		float distance = dist(link.p1, link.p2);
		float difference = link.len - distance;
		float percent = 0.0f;

		if (difference != 0.0f && distance != 0.0f)
		{
			percent = difference / distance / 2;
		}

		float offsetX = (link.p2->x - link.p1->x) * percent;
		float offsetY = (link.p2->y - link.p1->y) * percent;
		if (!link.p1->isAnchor)
		{
			links[i].p1->x -= offsetX;
			links[i].p1->y -= offsetY;
		}
		if (!link.p2->isAnchor)
		{
			links[i].p2->x += offsetX;
			links[i].p2->y += offsetY;
		}
	}

}

int main() {

	glfwInit();

	// Tell GLFW what version of OpenGL we are using 
	// In this case we are using OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Tell GLFW we are using the CORE profile
	// So that means we only have the modern functions
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	GLfloat vertices[TOTALPOINTS * 2]{};
	Point points[TOTALPOINTS]{};
	Link links[MAXLINKS]{};
	for (int i = 0; i < PPR; i++)
	{
		for (int j = 0; j < PPR; j++) {
			points[PPR * j + i].x = (float)i / PPR - 0.5;
			points[PPR * j + i].y = (float)j / PPR - 0.5;
			points[PPR * j + i].oldX = (float)i / PPR - 0.5;
			points[PPR * j + i].oldY = (float)j / PPR - 0.5;
		}
	}
	int l = 0;
	for (int i = 0; i < PPR; i++)
	{
		for (int j = 0; j < PPR; j++)
		{
			if (!(i == PPR - 1)) {
				links[l].p1 = &points[PPR * j + i];
				links[l].p2 = &points[PPR * j + i + 1];
				links[l].len = dist(links[l].p1, links[l].p2);
				links[l].p1->AddLink(&links[l]);
				links[l].p2->AddLink(&links[l]);
				l++;
			}
			if (!(j == PPR - 1)) {
				links[l].p1 = &points[PPR * j + i];
				links[l].p2 = &points[PPR * (j + 1) + i];
				links[l].len = dist(links[l].p1, links[l].p2);
				links[l].p1->AddLink(&links[l]);
				links[l].p2->AddLink(&links[l]);
				l++;
			}
		}
	}
	points[TOTALPOINTS - 1].isAnchor = true;
	points[TOTALPOINTS - PPR].isAnchor = true;
	points[TOTALPOINTS - PPR / 2].isAnchor = true;

	for (int i = 0; i < TOTALPOINTS; i++)
	{
		vertices[i * 2] = points[i].x;
		vertices[i * 2 + 1] = points[i].y;
	}


	GLFWwindow* window = glfwCreateWindow(SCREEN_X, SCREEN_Y, "Cloth", NULL, NULL);
	// Error check if the window fails to create
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	// Introduce the window into the current context
	glfwMakeContextCurrent(window);



	//Load GLAD so it configures OpenGL
	gladLoadGL();
	// Specify the viewport of OpenGL in the Window
	// In this case the viewport goes from x = 0, y = 0, to x = 800, y = 800
	glViewport(0, 0, SCREEN_X, SCREEN_Y);

	glEnable(GL_DEPTH_TEST);
	glPointSize(2.0f);

	Shader shader("default.vert", "default.frag");

	GLuint VAO, VBO;

	glGenVertexArrays(1, &VAO);

	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	//glEnableVertexAttribArray(1);

//	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	//glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	// The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
	unsigned char* data = stbi_load("text.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	// Specify the color of the background
	// Swap the back buffer with the front buffer
	glfwSwapBuffers(window);



	// Main while loop
	bool shouldUpdate = false;
	bool selectMode = false;
	int currentlySelectedIndex = -1;
	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.1f, 0.13f, 0.17f, 1.0f);
		// Clean the back buffer and assign the new color to it
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			points[0].y = 0.8f;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			points[0].x = -0.8f;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			points[0].x = 0.8f;
		}
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			points[0].x = -3.0f;
		}
		if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
		{
			shouldUpdate = true;
		}
		if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
		{
			double x, y;
			glfwGetCursorPos(window, &x, &y);
			x = (x / SCREEN_X) * 2 - 1;
			y = (y / SCREEN_Y) * 2 - 1;
			float fx = floorf(x * 100) / 100;
			// For some reason y is mirrored
			float fy = floorf(y * 100 * -1) / 100;
			for (int i = 0; i < TOTALPOINTS; i++)
			{
				float pointX = floorf(points[i].x * 100) / 100;
				float pointY = floorf(points[i].y * 100) / 100;
				if (pointX == fx && pointY == fy)
				{
					currentlySelectedIndex = i;
				}
			}
		}
		if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE)
		{
			currentlySelectedIndex = -1;
		}
		if (currentlySelectedIndex != -1)
		{
			double x, y;
			glfwGetCursorPos(window, &x, &y);
			x = (x / SCREEN_X) * 2 - 1;
			y = (y / SCREEN_Y) * 2 - 1;
			float fx = floorf(x * 100) / 100;
			// For some reason y is mirrored
			float fy = floorf(y * 100 * -1) / 100;
			points[currentlySelectedIndex].x = fx;
			points[currentlySelectedIndex].y = fy;
		}
		glBindTexture(GL_TEXTURE_2D, texture);

		if (shouldUpdate)
		{
			updatePoints(points);
			for (int i = 0; i < PHYSIC_STEP_PER_UPDATE; i++) 
			{
				updateLinks(links);
			}
		}

		for (int i = 0; i < TOTALPOINTS; i++)
		{
			vertices[i * 2] = points[i].x;
			vertices[i * 2 + 1] = points[i].y;
		}

		GLuint VAO, VBO;

		glGenVertexArrays(1, &VAO);

		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		//glEnableVertexAttribArray(1);

	//	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		//glEnableVertexAttribArray(2);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		shader.Activate();
		glBindVertexArray(VAO);
		glDrawArrays(GL_POINTS, 0, TOTALPOINTS);
		glfwSwapBuffers(window);
		// Take care of all GLFW events
		glfwPollEvents();
	}


	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	shader.Delete();
	// Delete window before ending the program
	glfwDestroyWindow(window);
	// Terminate GLFW before ending the program
	glfwTerminate();

	return 0;
}