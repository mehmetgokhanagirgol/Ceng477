#include "EclipseMap.h"

using namespace std;
struct vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texture;

    vertex() {}

    vertex(const glm::vec3 &position, const glm::vec3 &normal, const glm::vec2 &texture) : position(position),
                                                                                           normal(normal),
                                                                                           texture(texture) {}
};

struct triangle {
    int vertex1;
    int vertex2;
    int vertex3;

    triangle() {}

    triangle(const int &vertex1, const int &vertex2, const int &vertex3) : vertex1(vertex1), vertex2(vertex2),
                                                                           vertex3(vertex3) {}
};

void EclipseMap::Render(const char *coloredTexturePath, const char *greyTexturePath, const char *moonTexturePath) {
    // Open window
    GLFWwindow *window = openWindow(windowName, screenWidth, screenHeight);

    // Moon commands
    // Load shaders
    GLuint moonShaderID = initShaders("moonShader.vert", "moonShader.frag");
    GLuint vertexArrayObject;

    initMoonColoredTexture(moonTexturePath, moonShaderID);

    
    // TODO: Set moonVertices
    //vector<vertex> moonVertexes;
    // horizontalSplitCount -> sectors
    // verticalSplitCount -> stacks
    float x, y, z, xy;
    float nx, ny, nz, len = 1.0f / moonRadius;
    float s, t;    

    float sectorStep = 2 * PI / horizontalSplitCount;
    float stackStep = PI / verticalSplitCount;
    float sectorAngle, stackAngle;

    for(int i = 0; i <= verticalSplitCount; i++ ) {
        
        stackAngle = PI / 2 - i * stackStep;
        xy = moonRadius * cosf(stackAngle);
        z = moonRadius * sinf(stackAngle);
        
        for (int j = 0; j <= horizontalSplitCount; j++ ) {

            sectorAngle = j * sectorStep;
            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);
            moonVertices.push_back(x);
            moonVertices.push_back(y);
            moonVertices.push_back(z);
            nx = x * len;
            ny = y * len;
            nz = z * len;
            moonNormals.push_back(nx);
            moonNormals.push_back(ny);
            moonNormals.push_back(nz);

            s = float(j) / horizontalSplitCount;
            t = float(i) / verticalSplitCount;
            moonTextures.push_back(s);
            moonTextures.push_back(t);

        }
    }
    // moon indices 
    int k1, k2;
    for(int i = 0; i < verticalSplitCount; ++i) {
        k1 = i * (horizontalSplitCount + 1);
        k2 = k1 + horizontalSplitCount + 1;

        for(int j = 0; j < horizontalSplitCount; ++j) {
            if(i != 0) {
                moonIndices.push_back(k1);
                moonIndices.push_back(k2);
                moonIndices.push_back(k1 + 1);
            }

            if (i != (verticalSplitCount - 1)) {
                moonIndices.push_back(k1 + 1);
                moonIndices.push_back(k2);
                moonIndices.push_back(k2 + 1);
            }
            // TODO : lineIndices
        }
 
    }
    

    //std::cout << "Hello World !!!!!!!! " << moonTextures.size() << std::endl;
    // TODO: Configure Buffers
    
    
    // World commands
    // Load shaders
    GLuint worldShaderID = initShaders("worldShader.vert", "worldShader.frag");

    initColoredTexture(coloredTexturePath, worldShaderID);
    initGreyTexture(greyTexturePath, worldShaderID);

    // TODO: Set worldVertices
    
    // TODO: Configure Buffers
    glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);

	/* Init VBOs */

	glBufferData(GL_ARRAY_BUFFER, moonVertices.size() * sizeof(vertex), moonVertices.data(), GL_STATIC_DRAW);


	glBufferData(GL_ELEMENT_ARRAY_BUFFER, moonIndices.size() * sizeof(int), moonIndices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) nullptr);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) offsetof(vertex, normal));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (offsetof(vertex, texture)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
    // Enable depth test
    glEnable(GL_DEPTH_TEST);

    // Main rendering loop
    do {
        glViewport(0, 0, screenWidth, screenHeight);

        glClearStencil(0);
        glClearDepth(1.0f);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


        // TODO: Handle key presses
        handleKeyPress(window);

        // TODO: Manipulate rotation variables
        
        // TODO: Bind textures
        
        // TODO: Use moonShaderID program
        
        // TODO: Update camera at every frame
        
        // TODO: Update uniform variables at every frame
        
        // TODO: Bind moon vertex array        

        // TODO: Draw moon object
        // interleaved array
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glVertexPointer(3, GL_FLOAT, interleavedStride, &moonIndices[0]);
        glNormalPointer(GL_FLOAT, interleavedStride, &moonNormals[0]);
        glTexCoordPointer(2, GL_FLOAT, interleavedStride, &moonTextures[0]);

        glDrawElements(GL_TRIANGLES, moonIndices.size(), GL_UNSIGNED_INT, nullptr);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        
        /*************************/

        // TODO: Use worldShaderID program
        
        // TODO: Update camera at every frame

        // TODO: Update uniform variables at every frame
        
        // TODO: Bind world vertex array
        
        // TODO: Draw world object
        

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    } while (!glfwWindowShouldClose(window));

    // Delete buffers
    glDeleteBuffers(1, &moonVAO);
    glDeleteBuffers(1, &moonVBO);
    glDeleteBuffers(1, &moonEBO);

    
    // Delete buffers
    glDeleteBuffers(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
   
    glDeleteProgram(moonShaderID);
    glDeleteProgram(worldShaderID);

    // Close window
    glfwTerminate();
}

void EclipseMap::handleKeyPress(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

}

GLFWwindow *EclipseMap::openWindow(const char *windowName, int width, int height) {
    if (!glfwInit()) {
        getchar();
        return 0;
    }

    const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(width, height, windowName, NULL, NULL);
    glfwSetWindowMonitor(window, NULL, 1, 31, screenWidth, screenHeight, mode->refreshRate);

    if (window == NULL) {
        getchar();
        glfwTerminate();
        return 0;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        getchar();
        glfwTerminate();
        return 0;
    }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glClearColor(0, 0, 0, 0);

    return window;
}


void EclipseMap::initColoredTexture(const char *filename, GLuint shader) {
    int width, height;
    glGenTextures(1, &textureColor);
    cout << shader << endl;
    glBindTexture(GL_TEXTURE_2D, textureColor);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    GL_CLAMP_TO_EDGE);    // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // set texture filtering parameters
