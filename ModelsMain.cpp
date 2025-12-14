#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "tools/Polygon.h"
#include "tools/Cube.h"
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>
#include "stb_image.h"
using namespace std;
using namespace glm;

//circlelllllllllllllllllllllllllllll
const int segments = 180;
float radius = 7.0;
glm::vec3 sunPos = glm::vec3(0.0f);
std::vector<glm::vec3> orbitPoints;
std::vector<glm::vec3> moonOrbitPoints;
GLuint moonOrbitVAO, moonOrbitVBO;
GLuint orbitVAO, orbitVBO;
//lllllllllllllllllllllllllllllllll

vec3 moonPos;
float moonSpeed = 10.0f;
float moonAngle = 0.0f;

vec3 earthPos;
float selfRotate = 8.0f;
float earthSpeed = 0.4f;
bool stopAtEclipse = false;
float earthAngle = 0.0f;


int SCR_WIDTH = 1280;
int SCR_HEIGHT = 720;

Camera camera(vec3(0.0f, 5.0f, 7.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

vec3 firstLight(0.0f, 0.0f, 0.0f);
vec3 lightPos(0.0f, 0.0f, 0.0f);



void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

vector<GLuint> loadTextures(vector<string> paths, GLuint wrapOption = GL_REPEAT, GLuint filterOption = GL_LINEAR) {
	vector<GLuint> textures = {};

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapOption);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapOption);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterOption);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterOption);

	for (string path : paths)
	{
		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		int width, height, nrChannels;
		unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);


		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
			std::cout << "Failed to load texture" << std::endl;
		stbi_image_free(data);

		textures.push_back(texture);
	}

	return textures;
}

int main()
{
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Lecture 5", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    Shader allShader("./shaders/vs/all-model.vs", "./shaders/fs/earthShader.fs");
    Shader sunShader("./shaders/vs/all-model.vs", "./shaders/fs/sunShader.fs");
    Shader moonShader("./shaders/vs/all-model.vs", "./shaders/fs/moonShader.fs");

    glEnable(GL_DEPTH_TEST);
    stbi_set_flip_vertically_on_load(true);

    vector<string> texturePaths = {};
 	texturePaths.push_back("./textures/earth.jpg");
 	texturePaths.push_back("./textures/sun.jpg");
 	texturePaths.push_back("./textures/moon1.jpg");
 	vector<GLuint> textures = loadTextures(texturePaths);

    Model plainBall("./models/ball.glb");
    Model coolBall("./models/ball.glb");


    //earth orbit
    

    	for (int i = 0; i < segments; i++)
    	{
    		float angle = (float)i / segments * 2.0f * 3.1415926f;
    
    		float x = sunPos.x + cos(angle) * (radius);
    		float z = sunPos.z + sin(angle) * (radius- 1.7);
    
    		orbitPoints.push_back(glm::vec3(x, sunPos.y, z));
    	}
    	glGenVertexArrays(1, &orbitVAO);
    	glGenBuffers(1, &orbitVBO);
    
    	glBindVertexArray(orbitVAO);
    
    	glBindBuffer(GL_ARRAY_BUFFER, orbitVBO);
    	glBufferData(GL_ARRAY_BUFFER, orbitPoints.size() * sizeof(glm::vec3), orbitPoints.data(), GL_STATIC_DRAW);
    
    	glEnableVertexAttribArray(0);
    	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    

    	//moon orbit
    
    	for (int i = 0; i < segments; i++)
    	{
    		float angle = (float)i / segments * 2.0f * 3.1415926f;
    
    		float x = sunPos.x + cos(angle) * (radius-4);
    		float z = sunPos.z + sin(angle) * (radius -4);
    
    		moonOrbitPoints.push_back(glm::vec3(x, sunPos.y, z));
    	}
    	glGenVertexArrays(1, &moonOrbitVAO);
    	glGenBuffers(1, &moonOrbitVBO);
    
    	glBindVertexArray(moonOrbitVAO);
    
    	glBindBuffer(GL_ARRAY_BUFFER, moonOrbitVBO);
    	glBufferData(GL_ARRAY_BUFFER, moonOrbitPoints.size() * sizeof(glm::vec3), moonOrbitPoints.data(), GL_STATIC_DRAW);
    
    	glEnableVertexAttribArray(0);
    	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);


    bool watchForEclipse = false;
    bool stoppedAtEclipse = false;



    bool watchmoonEclipse = false;
    bool stoppedmoonEclipse = false;



    static bool gWasPressed = false;
    static bool hWasPressed = false;


    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mat4 projection = perspective(radians(45.0f),(float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
        mat4 view = camera.GetViewMatrix();

        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
        {
            if (!gWasPressed)
            {
                watchForEclipse = true;
                gWasPressed = true;
                earthSpeed+=4;
                moonSpeed+=4;
                selfRotate +=4;
            }
        }
        else
        {
            gWasPressed = false;
        }

        if (!stoppedAtEclipse && !stoppedmoonEclipse)
        {
            earthAngle += earthSpeed * deltaTime;
            moonAngle += moonSpeed * deltaTime;
        }
        earthPos = vec3(cos(earthAngle) * radius,0.0f,sin(earthAngle) * (radius - 1.7f));

        moonPos = earthPos + vec3(cos(moonAngle) * 1.5f,  0.0f,sin(moonAngle) * 1.5f);

        if (watchForEclipse && !stoppedAtEclipse)
        {
            vec3 sunToEarth = normalize(earthPos - vec3(-1.0f,0.0f,0.0f));
            vec3 earthToMoon = normalize(moonPos - earthPos);

            float alignment = dot(sunToEarth, earthToMoon);

            if ( alignment <= -0.99999999999999f)
            {
                stoppedAtEclipse = true;
                watchForEclipse = false;
                earthSpeed-=4;
                moonSpeed-=4;
                selfRotate = 0.0f;
            }
        }




        if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
        {
            if (!hWasPressed)
            {
                watchmoonEclipse = true;
                hWasPressed = true;
                earthSpeed += 4;
                moonSpeed += 4;
                selfRotate += 4;
            }
        }
        else
        {
            hWasPressed = false;
        }

        earthPos = vec3(cos(-earthAngle) * radius,0.0f,sin(-earthAngle) * (radius - 1.7f));

        moonPos = earthPos + vec3(cos(-moonAngle) * 1.5f,0.0f,sin(-moonAngle) * 1.5f );

        if (watchmoonEclipse && !stoppedmoonEclipse)
        {
            vec3 sunToEarth = normalize(earthPos - vec3(-1.0f, 0.0f, 0.0f));
            vec3 MoonToEarth = normalize(earthPos -moonPos );

            float alignment = dot(sunToEarth, MoonToEarth);

            if (alignment <= -0.99999999999999f)
            {
                stoppedmoonEclipse = true;
                watchmoonEclipse = false;
                earthSpeed -= 4;
                moonSpeed -= 4;
                selfRotate = 0.0f;
            }
        }






        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
        {
            stoppedAtEclipse = false;
            watchForEclipse = false;
            stoppedmoonEclipse = false;
            watchmoonEclipse = false;
            earthSpeed = 0.4;
            moonSpeed = 10.0;
            selfRotate = 8.0f;
        }



        allShader.use();
        allShader.setMat4("projection", projection);
        allShader.setMat4("view", view);
        allShader.setVec3("lightPos", lightPos);
        allShader.setVec3("viewPos", camera.Position);
        if (stoppedAtEclipse)
        {
            allShader.setVec3("objectColor", vec3(0.2f));
        }
        else
        {
            allShader.setVec3("objectColor", vec3(1.0f, 1.0f, 1.0f));
        }

        allShader.setMat4("model", mat4(1.0f));

        glBindVertexArray(orbitVAO);
        glDrawArrays(GL_LINE_LOOP, 0, orbitPoints.size());
        glBindVertexArray(0);

        mat4 model = mat4(1.0f);
        model = translate(model, earthPos);
        model = rotate(model, (float)glfwGetTime()*selfRotate, vec3(0.0f, 1.0f, 0.0f));
        model = scale(model, vec3(0.5f));

        allShader.setMat4("model", model);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[0]);
        coolBall.Draw(allShader);


		glBindVertexArray(moonOrbitVAO);
		glDrawArrays(GL_LINE_LOOP, 0, moonOrbitPoints.size());
		glBindVertexArray(0);
		


        moonShader.use();
        moonShader.setMat4("projection", projection);
        moonShader.setMat4("view", view);
        moonShader.setVec3("lightPos", lightPos);
        moonShader.setVec3("viewPos", camera.Position);
        if ( stoppedmoonEclipse)
        {
            moonShader.setVec3("objectColor", vec3(0.3f));
            
        }
        else
        {
            moonShader.setVec3("objectColor", vec3(1.0f));
        }
        
        moonShader.setMat4("model", mat4(1.0f));

        mat4 moonModel = mat4(1.0f);
        moonModel = translate(moonModel, moonPos);
        moonModel = scale(moonModel, vec3(0.25f));
        moonShader.setMat4("model", moonModel);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[2]);
        plainBall.Draw(moonShader);

        sunShader.use();
        sunShader.setMat4("projection", projection);
        sunShader.setMat4("view", view);

        mat4 sunModel = mat4(1.0f);
        sunModel = scale(sunModel, vec3(1.0f));
        sunModel = translate(sunModel, vec3(-1.0f,0.0f,0.0f));

        sunShader.setMat4("model", sunModel);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[1]);
        plainBall.Draw(sunShader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
