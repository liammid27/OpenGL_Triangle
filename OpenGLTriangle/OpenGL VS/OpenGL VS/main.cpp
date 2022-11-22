
#define GLEW_STATIC
#include <string.h>
#include <stdio.h>
#include <cmath>
#include <iostream>

//GLEW
#include <GL/glew.h>

//GLFW
#include <GLFW/glfw3.h>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


//windows dimensions
const GLint WIDTH = 800, HEIGHT = 600;

// formula to convert degrees to radians.
// multiply any degree value by this to get it 
// in radians
const float degreeToRadians = 3.14159265f / 180.0f;

GLuint VAO, VBO, shader, uniformModel;

//for translation
bool direction = true;

float xOffset = 0.0f;
float yOffset = 0.0f;
float triMaxOffset = 0.7f;
float triIncrement = 0.0005f;

//for rotation
float curAngle = 0.0f;

//for scaling
bool sizeDirection = true;
float currSize = 0.4f;
float maxSize = 0.8f;
float minSize = 0.1f;

#pragma region Vertex&Fragment

// creating vertex shader 
// taking each point that you pass into the vertices
// allows you to do things with the values (such as display or move around)
// allows us to modify the vertices values and pass it to the fragment shader
// vec4(0.4 * pos.x, 0.4 * pos.y, pos.z, 1.0); final pos values of the vertices on screen

static const char* vShader = "       \n\
# version 330                        \n\
                                     \n\
layout (location = 0) in vec3 pos;   \n\
                                     \n\
uniform mat4 model;                  \n\
                                     \n\
                                     \n\
void main()                          \n\
{                                    \n\
  gl_Position = model * vec4(pos, 1.0);      \n\
}";

// fragment shader
// handling each pixel on the screen
// how each value works with the vertices
// out vec4 colour (the out value for the colour values for each pixel)
// colour = vec4(1.0, 0.0, 0.0, 1.0) -  setting the colour values (RGBa)
static const char* fShader = "       \n\
# version 330                        \n\
                                     \n\
out vec4 colour;                     \n\
                                     \n\
void main()                          \n\
{                                    \n\
  colour = vec4(1.0, 0.0, 0.0, 1.0); \n\
}";

#pragma endregion

//Creating a VAO which is a vertex array object and VBO which is a 
//vertex buffer object
//- VAO holds multiple VBOs and other types of buffers to define
void CreateTriangle()
{
    //array with GLfloats
    //points that make up the triangle
    GLfloat vertices[] =
    {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        0.0f, 1.0f, 0.0f
    };

    //Create the VAO. (#ofArrays, IDofArray). 
    //creates a vertex array in gpu. defines space in mem
    glGenVertexArrays(1, &VAO);


    //Binding the VAO
    glBindVertexArray(VAO);

#pragma region creating VBO
    //create a buffer object
    // 1 buffer and assign the ID to VBO
    glGenBuffers(1, &VBO);
    //binding the VBO
    //Define which buffer to bind to (Multiple buffers availble)
    //using the array buffer, buffer ID
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    //connect the buffer data
    // takes 4 arguements
    //(Target, sizeOfData, data,Static/dynamic_draw)
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    //(LocationOfAttribute,
    //XYZ which are 3 values,
    //TypeOfValues, 
    //NormaliseValues,
    //Stride(take a vertex value and skip n amount)
    // Offset(where the data starts))
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    //                     \\
            //                      \\   
            //enable Attribute 0     \/
    glEnableVertexAttribArray(0);
    //unbinding
    glBindBuffer(GL_ARRAY_BUFFER, 0);
#pragma endregion
    glBindVertexArray(0);
}

void AddShader(GLuint theProgram, const char* shaderCode, GLenum shaderType)
{

    GLuint theShader = glCreateShader(shaderType); // create the individual shader

    const GLchar* theCode[1]; //array with a pointer to the first element of the array
    theCode[0] = shaderCode;

    GLint codeLength[1];  // length of the code
    codeLength[0] = strlen(shaderCode); // get the length of the code

    //Passing the created shader code into the shader source
    glShaderSource(theShader, 1, theCode, codeLength);  //modifies the source for the shader

    glCompileShader(theShader); // compile the shader code that is in memory

#pragma endregion Error Check

    GLint result = 0;
    GLchar eLog[1024] = { 0 };

    glGetShaderiv(theShader, GL_COMPILE_STATUS, &result);
    if (!result) {
        glGetShaderInfoLog(theShader, 1024, NULL, eLog);
        fprintf(stderr, "Error compiling the %d shader: '%s'\n", shaderType, eLog);
        return;
    }
#pragma endregion

    //attaching the shader to the program
    glAttachShader(theProgram, theShader);
}


void CompileShader() {
    //creating the program
    shader = glCreateProgram();

    // make sure the shader is created correctly.
    if (!shader) {
        printf("Failed to create shader");
        return;

    }

    // adding shader to the program
    // pass in the prog, string vShader
    // indicate the type of shader
    AddShader(shader, vShader, GL_VERTEX_SHADER);

    // adding shader to the program
    // pass in the prog, string fShader
    // indicate the type of shader
    AddShader(shader, fShader, GL_FRAGMENT_SHADER);

#pragma region ErrorChecking
    //  create the roles on the gpu
       //linking the prog and making sure
       //everything is working
       //check if its linked properly and validate the settings
       //for the openGL

    // getting error codes from the creation
    // of the shaders
    GLint result = 0; // result of the two functions
    GLchar eLog[1024] = { 0 };  // logging the error

    //checking if the program is linked correctly
    glLinkProgram(shader);
    glGetProgramiv(shader, GL_LINK_STATUS, &result); // get the info. Check if the prog is linked

    //check if the result is false.
    if (!result) {
        glGetProgramInfoLog(shader, sizeof(eLog), NULL, eLog);
        fprintf(stderr, "Error linking program: '%s'\n", eLog);
        return;
    }

    //validate the program/
    // checking if openGL was setup correctly for the shader
    glValidateProgram(shader);
    glGetProgramiv(shader, GL_VALIDATE_STATUS, &result);

    if (!result) {
        glGetProgramInfoLog(shader, sizeof(eLog), NULL, eLog);
        fprintf(stderr, "Error linking program: '%s'\n", eLog);
        return;
    }
#pragma endregion

    //uniformModel = glGetUniformLocation(shader, "xMove");
    uniformModel = glGetUniformLocation(shader, "model");
}



int main()
{
    //iniliste glfw
    if (!glfwInit())
    {
        printf("GLFW init failed!");
        glfwTerminate();
        return 1;
    }
    //setup GLFW window properties
    //OpenGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // Core profile = not backward compat
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // Allow for forward compat
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    GLFWwindow* mainWindow = glfwCreateWindow(WIDTH, HEIGHT, "TEST WINDOW", NULL, NULL);
    if (!mainWindow)
    {
        printf("GLFW window creation failed");
        glfwTerminate();
        return 1;
    }
    //get buffer size info
    int bufferWidth, bufferHeight;
    glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);
    // Set context for GLEW to use
    glfwMakeContextCurrent(mainWindow);
    // Allow modern extension features
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) // glewInit() returns GLEW_OK if it has been initialised.
    {
        printf("GLEW initialisation failed!");
        glfwDestroyWindow(mainWindow);
        glfwTerminate();
        return 1;
    }
    // Setup Viewport size
    glViewport(0, 0, bufferWidth, bufferHeight);
    CreateTriangle();
    CompileShader();
    // Loop until window closed
    while (!glfwWindowShouldClose(mainWindow))
    {
        // Get and Handle user input events
        glfwPollEvents();    // checks if any events has happened
        //check if direction is going to the right
        
        
        //when it reaches the maxOffset we want to switch directions
        
#pragma region matrix Trans

        //======================================================
        //               matrix transforms
        //======================================================
        // used to change the value of the angle
        // when a full rotation is made, the value resets
        

        // adjusting the size of the triangle
        
        //=========================================================================
#pragma endregion        

        //Setting the window colour
        //              R     G     B    Alpha (Transparency)
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        // Clear window
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shader); // Get the ID of the shader created

        // creating the model matrix
        // 4x4 identiy matrix
        glm::mat4 model(1.0f);

        // take the identiy matrix
        //apply a translation to it by altering the x values
        // top right value in the matrix will change to triOffSet (x values)


        //Use right key to translate right
        int stateT = glfwGetKey(mainWindow, GLFW_KEY_RIGHT);
        if (stateT == GLFW_PRESS)
        {
            xOffset += triIncrement; //right
        }
        
        //Use left key to translate left
        int stateT2 = glfwGetKey(mainWindow, GLFW_KEY_LEFT);
        if (stateT2 == GLFW_PRESS)
        {
            xOffset -= triIncrement; //left
        }

        //Use up key to translate up
        int stateT3 = glfwGetKey(mainWindow, GLFW_KEY_UP);
        if (stateT3 == GLFW_PRESS)
        {
            yOffset += triIncrement; //up
        }

        //Use down key to translate down
        int stateT4 = glfwGetKey(mainWindow, GLFW_KEY_DOWN);
        if (stateT4 == GLFW_PRESS)
        {
            yOffset -= triIncrement; //down
        }
            
        model = glm::translate(model, glm::vec3(xOffset, yOffset, 0.0f));

        
        //Use W to scale up the triangle
        int stateS = glfwGetKey(mainWindow, GLFW_KEY_W);
        if (stateS == GLFW_PRESS) 
        {
            currSize += 0.0003f;

            if (currSize >= maxSize || currSize <= minSize)
            {
                sizeDirection = !sizeDirection;
            }

        }
        
        //Use S to scale down the triangle
        int stateS2 = glfwGetKey(mainWindow, GLFW_KEY_S);
        if (stateS2 == GLFW_PRESS)
        {
            currSize -= 0.0003f;

            if (currSize >= maxSize || currSize <= minSize)
            {
                sizeDirection = !sizeDirection;
            }

        }

        model = glm::scale(model, glm::vec3(currSize, currSize, 0.0f));

        //Use A key to rotate counter-clockwise
        int stateR = glfwGetKey(mainWindow, GLFW_KEY_A);
        if (stateR == GLFW_PRESS)
        {
            curAngle += 0.05f;
            if (curAngle >= 360)
            {
                curAngle += 360;
            }
            
        }
     
        //Use D key to rotate clockwise
        int stateR2 = glfwGetKey(mainWindow, GLFW_KEY_D);
        if (stateR2 == GLFW_PRESS)
        {
            curAngle -= 0.05f;
            if (curAngle >= 360)
            {
                curAngle += 360;
            }

        }
       
        model = glm::rotate(model, curAngle * degreeToRadians, glm::vec3(0.0f, 0.0f, 1.0f));
        //TRANSLATE
        //model = glm::translate(model, glm::vec3(triOffset, 0.0f, 0.0f));

        //ROTATE
        //model = glm::rotate(model, curAngle * degreeToRadians, glm::vec3(0.0f, 0.0f, 1.0f));


        //model = glm::scale(model, glm::vec3(currSize, 0.4f, 1.0f));

#pragma region OldMove
        //attaching the uniformModel to triOffset
    //glUniform1f(uniformModel, triOffset);
#pragma endregion

//going to hold a 4x4 matrix with 4 float values
// pass in the model matrix
// 1 matrix and not transposing it (flipping)
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(VAO); // binding the VAO (working with the VAO)

        glDrawArrays(GL_TRIANGLES, 0, 3);//(mode, starting pos, number of points)

        glBindVertexArray(0); //unbind
        glUseProgram(0);// unassigning the shader

        glfwSwapBuffers(mainWindow);
    }

    return 0;


}