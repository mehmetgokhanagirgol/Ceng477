#include "EclipseMap.h"

using namespace std;

struct vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texture;

    vertex() {}

    vertex(const glm::vec3 &position, const glm::vec3 &normal, const glm::vec2 &texture) : position(position),
                                                                                           normal(normal),
                                                                                           texture(texture) {}
};

struct triangle
{
    int vertex1;
    int vertex2;
    int vertex3;

    triangle() {}

    triangle(const int &vertex1, const int &vertex2, const int &vertex3) : vertex1(vertex1), vertex2(vertex2),
                                                                           vertex3(vertex3) {}
};
vector<vertex> worldVerticeArr;
vector<vertex> moonVerticeArr;
void generateSphereVertices(const EclipseMap& context, vector<vertex> &vertices, vector<unsigned int> &indices, float modelRadius)
{
    float x, y, z, xy, u, v, alpha, beta;
	for (int i = 0; i <= context.verticalSplitCount; i++) {
		beta = M_PI / 2 - (i * M_PI / context.verticalSplitCount); // pi/2 to -pi/2
		xy = modelRadius * cosf(beta);
		z = modelRadius * sinf(beta);
		for (int j = 0; j <= context.horizontalSplitCount; j++) {
			alpha = j * 2 * M_PI / context.horizontalSplitCount;  // 0 to 2pi
			x = xy * cosf(alpha);
			y = xy * sinf(alpha);
			u = (float)j / context.horizontalSplitCount;
			v = (float)i / context.verticalSplitCount;

			vertex vertex;
			vertex.position = glm::vec3(x, y, z);
			vertex.normal = glm::normalize(glm::vec3(x/modelRadius, y/modelRadius, z/modelRadius));
			vertex.texture = glm::vec2(u, v);
			vertices.push_back(vertex);
		}
	}

    int k1, k2;
    for (int i = 0; i < context.verticalSplitCount; ++i)
    {
        k1 = i * (context.horizontalSplitCount + 1);
        k2 = k1 + context.horizontalSplitCount + 1;

        for (int j = 0; j < context.horizontalSplitCount; ++j, ++k1, ++k2)
        {
            if (i != 0)
            {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (context.horizontalSplitCount - 1))
            {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
    
}
void EclipseMap::initScene() {
    glm::vec3 left = glm::cross(cameraStartUp, cameraDirection);
	cameraStartUp = glm::rotate(cameraStartUp, -1.07999f, left);
	cameraDirection = glm::rotate(cameraDirection, -1.07999f, left);
	M_model = glm::rotate(M_model, (float) glm::radians(-60.0), glm::vec3(1, 0, 0));
	M_view = glm::lookAt(cameraStartPosition, cameraStartPosition + cameraDirection, cameraStartUp);
	M_projection = glm::perspective(projectionAngle, aspectRatio, near, far);
	MVP = M_projection * M_view * M_model;
}

void EclipseMap::updateCamera(GLuint shaderProgram) {
    
}
glm::mat4 ProjectionMatrix;
glm::mat4 MVP; 
glm::mat4 MV; 
glm::mat4 ViewMatrix;
void EclipseMap::handleUniforms(GLuint idProgramShader) {
	glUniformMatrix4fv(glGetUniformLocation(idProgramShader, "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
	glUniform1f(glGetUniformLocation(idProgramShader, "heightFactor"), heightFactor);
	glUniform1i(glGetUniformLocation(idProgramShader, "textureOffset"), textureOffset);
	glUniform3fv(glGetUniformLocation(idProgramShader, "cameraPosition"), 1, glm::value_ptr(cameraPosition));
	glUniform3fv(glGetUniformLocation(idProgramShader, "lightPosition"), 1, glm::value_ptr(lightPos));
	glUniform1i(glGetUniformLocation(idProgramShader, "TexColor"), 0);
	glUniform1i(glGetUniformLocation(idProgramShader, "TexGrey"), 2);
	glUniform1i(glGetUniformLocation(idProgramShader, "MoonTexColor"), 1);


}
void EclipseMap::handleCamera(GLuint idProgramShader) {
    cameraPosition += speed * cameraDirection;
	M_view = glm::lookAt(cameraPosition, cameraPosition + cameraDirection, cameraUp);
	M_projection = glm::perspective(projectionAngle, aspectRatio, near, far);
	MVP = M_projection * M_view * M_model;
    glUniformMatrix4fv(glGetUniformLocation(idProgramShader, "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
	glUniform3fv(glGetUniformLocation(idProgramShader, "cameraPosition"), 1, glm::value_ptr(cameraPosition));
	glUniform1f(glGetUniformLocation(idProgramShader, "heightFactor"), heightFactor);
}
void EclipseMap::Render(const char *coloredTexturePath, const char *greyTexturePath, const char *moonTexturePath)
{
    // Open window
    GLFWwindow *window = openWindow(windowName, screenWidth, screenHeight);
    //glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  

    // Moon commands
    // Load shaders
    GLuint moonShaderID = initShaders("moonShader.vert", "moonShader.frag");

    initMoonColoredTexture(moonTexturePath, moonShaderID);

    // TODO: Set moonVertices
    moonVerticeArr.clear();
    moonIndices.clear();

    // TODO: Configure Buffers


    glGenVertexArrays(1, &moonVAO);
	glBindVertexArray(moonVAO);

	/* Init VBOs */
	glGenBuffers(1, &moonVBO);
	glBindBuffer(GL_ARRAY_BUFFER, moonVBO);

    generateSphereVertices(*this, moonVerticeArr, moonIndices, moonRadius);
    glBufferData(GL_ARRAY_BUFFER, moonVerticeArr.size() * sizeof(vertex), moonVerticeArr.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &moonEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, moonEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, moonIndices.size() * sizeof(unsigned int), moonIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) nullptr);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) offsetof(vertex, normal));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (offsetof(vertex, texture)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

    // World commands
    // Load shaders
    GLuint worldShaderID = initShaders("worldShader.vert", "worldShader.frag");

    initColoredTexture(coloredTexturePath, worldShaderID);
    initGreyTexture(greyTexturePath, worldShaderID);
    
    // TODO: Set worldVertices
    worldVerticeArr.clear();
    worldIndices.clear();

    // TODO: Configure Buffers
    glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	/* Init VBOs */
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

    generateSphereVertices(*this, worldVerticeArr, worldIndices, radius);
    glBufferData(GL_ARRAY_BUFFER, worldVerticeArr.size() * sizeof(vertex), worldVerticeArr.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, worldIndices.size() * sizeof(unsigned int), worldIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) nullptr);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) offsetof(vertex, normal));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (offsetof(vertex, texture)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
    initScene();

    // Enable depth test
    glEnable(GL_DEPTH_TEST);

    // Main rendering loop
    do
    {
        glViewport(0, 0, screenWidth, screenHeight);
        glClearStencil(0);
        glClearDepth(1.0f);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        // TODO: Handle key presses
        handleKeyPress(window);
        // TODO: Manipulate rotation variables
        // TODO: Bind texture
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, moonTextureColor);
        // TODO: Use moonShaderID program
        glUseProgram(moonShaderID);
        // TODO: Update camera at every frame
        handleCamera(moonShaderID);    
        // TODO: Update uniform variables at every frame
        handleUniforms(moonShaderID);
        // TODO: Bind moon vertex array        
        glBindVertexArray(moonVAO);
        // TODO: Draw moon object
        glDrawElements(GL_TRIANGLES, moonIndices.size(), GL_UNSIGNED_INT, nullptr);

        /*************************/
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureColor);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, textureGrey);
        // TODO: Use worldShaderID program
        glUseProgram(worldShaderID);
        // TODO: Update camera at every frame
        handleCamera(worldShaderID);
        // TODO: Update uniform variables at every frame
        handleUniforms(worldShaderID);
        // TODO: Bind world vertex array
        glBindVertexArray(VAO);
        // TODO: Draw world object
        glDrawElements(GL_TRIANGLES, worldIndices.size(), GL_UNSIGNED_INT, nullptr);
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

void EclipseMap::handleKeyPress(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

GLFWwindow *EclipseMap::openWindow(const char *windowName, int width, int height)
{
    if (!glfwInit())
    {
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

    if (window == NULL)
    {
        getchar();
        glfwTerminate();
        return 0;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = true;
    if (glewInit() != GLEW_OK)
    {
        getchar();
        glfwTerminate();
        return 0;
    }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glClearColor(0, 0, 0, 0);

    return window;
}

void EclipseMap::initColoredTexture(const char *filename, GLuint shader)
{
    int width, height;
    glGenTextures(1, &textureColor);
    cout << shader << endl;
    glBindTexture(GL_TEXTURE_2D, textureColor);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    GL_CLAMP_TO_EDGE); // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    unsigned char *raw_image = NULL;
    int bytes_per_pixel = 3;   /* or 1 for GRACYSCALE images */
    int color_space = JCS_RGB; /* or JCS_GRAYSCALE for grayscale images */

    /* these are standard libjpeg structures for reading(decompression) */
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    /* libjpeg data structure for storing one row, that is, scanline of an image */
    JSAMPROW row_pointer[1];

    FILE *infile = fopen(filename, "rb");
    unsigned long location = 0;
    int i = 0, j = 0;

    if (!infile)
    {
        printf("Error opening jpeg file %s\n!", filename);
        return;
    }
    printf("Texture filename = %s\n", filename);

    /* here we set up the standard libjpeg error handler */
    cinfo.err = jpeg_std_error(&jerr);
    /* setup decompression process and source, then read JPEG header */
    jpeg_create_decompress(&cinfo);
    /* this makes the library read from infile */
    jpeg_stdio_src(&cinfo, infile);
    /* reading the image header which contains image information */
    jpeg_read_header(&cinfo, TRUE);
    /* Start decompression jpeg here */
    jpeg_start_decompress(&cinfo);

    /* allocate memory to hold the uncompressed image */
    raw_image = (unsigned char *)malloc(cinfo.output_width * cinfo.output_height * cinfo.num_components);
    /* now actually read the jpeg into the raw buffer */
    row_pointer[0] = (unsigned char *)malloc(cinfo.output_width * cinfo.num_components);
    /* read one scan line at a time */
    while (cinfo.output_scanline < cinfo.image_height)
    {
        jpeg_read_scanlines(&cinfo, row_pointer, 1);
        for (i = 0; i < cinfo.image_width * cinfo.num_components; i++)
            raw_image[location++] = row_pointer[0][i];
    }

    height = cinfo.image_height;
    width = cinfo.image_width;

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, raw_image);

    imageWidth = width;
    imageHeight = height;

    glGenerateMipmap(GL_TEXTURE_2D);

    glUseProgram(shader); // don't forget to activate/use the shader before setting uniforms!
    // either set it manually like so:

    glUniform1i(glGetUniformLocation(shader, "TexColor"), 0);
    /* wrap up decompression, destroy objects, free pointers and close open files */
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    free(row_pointer[0]);
    free(raw_image);
    fclose(infile);
}

void EclipseMap::initGreyTexture(const char *filename, GLuint shader)
{

    glGenTextures(1, &textureGrey);
    glBindTexture(GL_TEXTURE_2D, textureGrey);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    GL_CLAMP_TO_EDGE); // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height;

    unsigned char *raw_image = NULL;
    int bytes_per_pixel = 3;   /* or 1 for GRACYSCALE images */
    int color_space = JCS_RGB; /* or JCS_GRAYSCALE for grayscale images */

    /* these are standard libjpeg structures for reading(decompression) */
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    /* libjpeg data structure for storing one row, that is, scanline of an image */
    JSAMPROW row_pointer[1];

    FILE *infile = fopen(filename, "rb");
    unsigned long location = 0;
    int i = 0, j = 0;

    if (!infile)
    {
        printf("Error opening jpeg file %s\n!", filename);
        return;
    }
    printf("Texture filename = %s\n", filename);

    /* here we set up the standard libjpeg error handler */
    cinfo.err = jpeg_std_error(&jerr);
    /* setup decompression process and source, then read JPEG header */
    jpeg_create_decompress(&cinfo);
    /* this makes the library read from infile */
    jpeg_stdio_src(&cinfo, infile);
    /* reading the image header which contains image information */
    jpeg_read_header(&cinfo, TRUE);
    /* Start decompression jpeg here */
    jpeg_start_decompress(&cinfo);

    /* allocate memory to hold the uncompressed image */
    raw_image = (unsigned char *)malloc(cinfo.output_width * cinfo.output_height * cinfo.num_components);
    /* now actually read the jpeg into the raw buffer */
    row_pointer[0] = (unsigned char *)malloc(cinfo.output_width * cinfo.num_components);
    /* read one scan line at a time */
    while (cinfo.output_scanline < cinfo.image_height)
    {
        jpeg_read_scanlines(&cinfo, row_pointer, 1);
        for (i = 0; i < cinfo.image_width * cinfo.num_components; i++)
            raw_image[location++] = row_pointer[0][i];
    }

    height = cinfo.image_height;
    width = cinfo.image_width;

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, raw_image);

    glGenerateMipmap(GL_TEXTURE_2D);

    glUseProgram(shader); // don't forget to activate/use the shader before setting uniforms!
    // either set it manually like so:

    glUniform1i(glGetUniformLocation(shader, "TexGrey"), 1);
    /* wrap up decompression, destroy objects, free pointers and close open files */
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    free(row_pointer[0]);
    free(raw_image);
    fclose(infile);
}

void EclipseMap::initMoonColoredTexture(const char *filename, GLuint shader)
{
    int width, height;
    glGenTextures(1, &moonTextureColor);
    cout << shader << endl;
    glBindTexture(GL_TEXTURE_2D, moonTextureColor);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    GL_CLAMP_TO_EDGE); // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    unsigned char *raw_image = NULL;
    int bytes_per_pixel = 3;   /* or 1 for GRACYSCALE images */
    int color_space = JCS_RGB; /* or JCS_GRAYSCALE for grayscale images */

    /* these are standard libjpeg structures for reading(decompression) */
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    /* libjpeg data structure for storing one row, that is, scanline of an image */
    JSAMPROW row_pointer[1];

    FILE *infile = fopen(filename, "rb");
    unsigned long location = 0;
    int i = 0, j = 0;

    if (!infile)
    {
        printf("Error opening jpeg file %s\n!", filename);
        return;
    }
    printf("Texture filename = %s\n", filename);

    /* here we set up the standard libjpeg error handler */
    cinfo.err = jpeg_std_error(&jerr);
    /* setup decompression process and source, then read JPEG header */
    jpeg_create_decompress(&cinfo);
    /* this makes the library read from infile */
    jpeg_stdio_src(&cinfo, infile);
    /* reading the image header which contains image information */
    jpeg_read_header(&cinfo, TRUE);
    /* Start decompression jpeg here */
    jpeg_start_decompress(&cinfo);

    /* allocate memory to hold the uncompressed image */
    raw_image = (unsigned char *)malloc(cinfo.output_width * cinfo.output_height * cinfo.num_components);
    /* now actually read the jpeg into the raw buffer */
    row_pointer[0] = (unsigned char *)malloc(cinfo.output_width * cinfo.num_components);
    /* read one scan line at a time */
    while (cinfo.output_scanline < cinfo.image_height)
    {
        jpeg_read_scanlines(&cinfo, row_pointer, 1);
        for (i = 0; i < cinfo.image_width * cinfo.num_components; i++)
            raw_image[location++] = row_pointer[0][i];
    }

    height = cinfo.image_height;
    width = cinfo.image_width;

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, raw_image);

    imageWidth = width;
    imageHeight = height;

    glGenerateMipmap(GL_TEXTURE_2D);

    glUseProgram(shader); // don't forget to activate/use the shader before setting uniforms!
    // either set it manually like so:

    glUniform1i(glGetUniformLocation(shader, "MoonTexColor"), 2);
    /* wrap up decompression, destroy objects, free pointers and close open files */
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    free(row_pointer[0]);
    free(raw_image);
    fclose(infile);
}
