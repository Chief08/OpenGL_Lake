// Include C++ headers
#include <iostream>
#include <string>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Shader loading utilities and other
#include <common/shader.h>
#include <common/util.h>
#include <common/camera.h>
#include <common/model.h>
#include <common/texture.h>

using namespace std;
using namespace glm;

// Function prototypes
void initialize();
void createContext();
void renderRefraction();
void renderReflection();
void mainLoop();
void free();
void pollKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);


#define W_WIDTH 1024
#define W_HEIGHT 768
#define TITLE "Lake"

// Global variables
GLFWwindow* window;
Camera* camera;
GLuint shaderProgram, shaderProgram0;
GLuint timeUniform;
GLuint wave0point, wave0time, wave1point, wave1time;
GLuint terrainD, terrainS, barkD, leavesD,leavesT, rockD, lake0, lake1, lake2, duckD ;
GLuint diffuseColorSampler, diffuseColorSampler1, specularColorSampler;
GLuint  MLocation,VLocation,PLocation, LightPosLocation, textureflag;
GLuint diffuseColorSampler2, diffuseColorSampler3, specularColorSampler2;
GLuint  MLocation0, VLocation0, PLocation0, LightPosLocation0, textureflag0 , timeUniform0;
GLuint modelVAO, modelVerticiesVBO, modelNormalsVBO, modelUVVBO;
std::vector<vec3> modelVertices, modelNormals;
std::vector<vec2> modelUVs;
Drawable *terrain, *tree, *rock, *bark, *leaves ,*lake,*duck;
float timeH, timeF, yH, yF;
GLuint ReflectionBuffer = 0;
GLuint renderedTexture;
GLuint depthrenderbuffer;
GLuint planeLocation;
GLuint planeLocation0;
GLuint cameraPos;


void createContext() {
    // Create and compile our GLSL program from the shaders
    shaderProgram = loadShaders("Shader.vertexshader", "Shader.fragmentshader");
    shaderProgram0 = loadShaders("Shader0.vertexshader", "Shader0.fragmentshader");

    // Draw wire frame triangles or fill: GL_LINE, or GL_FILL
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    //create extra buffers and their textures
    glGenFramebuffers(1, &ReflectionBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, ReflectionBuffer);
    glGenTextures(1, &renderedTexture);
    glBindTexture(GL_TEXTURE_2D, renderedTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 768, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    
    glGenRenderbuffers(1, &depthrenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1024, 768);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);

    // Set the list of draw buffers.
    GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, DrawBuffers);

    glBindFramebuffer(GL_RENDERBUFFER, 0);
    // Get a pointer location to model matrix in the vertex shader
    MLocation = glGetUniformLocation(shaderProgram, "M");
    VLocation = glGetUniformLocation(shaderProgram, "V");
    PLocation = glGetUniformLocation(shaderProgram, "P");
    LightPosLocation = glGetUniformLocation(shaderProgram, "light_position_cameraspace");

    MLocation0 = glGetUniformLocation(shaderProgram0, "M");
    VLocation0 = glGetUniformLocation(shaderProgram0, "V");
    PLocation0 = glGetUniformLocation(shaderProgram0, "P");
    LightPosLocation0 = glGetUniformLocation(shaderProgram0, "light_position_cameraspace");


    timeF = 100000;
    timeH = 100000;
    yF = 0;
    yH = 2;

    terrain = new Drawable("obj/terrain.obj");
    bark = new Drawable("obj/stem.obj");
    leaves = new Drawable("obj/leaves.obj");
    rock = new Drawable("obj/rock.obj");
    lake = new Drawable("obj/lake.obj");
    duck = new Drawable("obj/duck.obj");

    

    terrainD = loadSOIL("textures/grounddiff.png");
    lake0 = loadSOIL("textures/water.png");
    lake1 = loadSOIL("textures/water1.png");
    terrainS = loadSOIL("textures/groundspec.png");
    barkD = loadSOIL("textures/barkdiff.png");
    leavesD = loadSOIL("textures/leaves0.png");
    leavesT = loadSOIL("textures/leavesT.png");
    rockD = loadSOIL("textures/rockD.png");
    duckD = loadSOIL("textures/duck.png");

    textureflag = glGetUniformLocation(shaderProgram, "textureflag");
    diffuseColorSampler = glGetUniformLocation(shaderProgram, "diffuseColorSampler");
    diffuseColorSampler1 = glGetUniformLocation(shaderProgram, "diffuseColorSampler1");
    specularColorSampler = glGetUniformLocation(shaderProgram, "specularColorSampler");
    timeUniform = glGetUniformLocation(shaderProgram, "time0");
    wave0time = glGetUniformLocation(shaderProgram, "wave0time");
    wave0point = glGetUniformLocation(shaderProgram, "wave0point");
    wave1time = glGetUniformLocation(shaderProgram, "wave1time");
    wave1point = glGetUniformLocation(shaderProgram, "wave1point");
    planeLocation= glGetUniformLocation(shaderProgram, "plane");
    cameraPos = glGetUniformLocation(shaderProgram, "cameraPos");

    textureflag0 = glGetUniformLocation(shaderProgram0, "textureflag");
    diffuseColorSampler2 = glGetUniformLocation(shaderProgram0, "diffuseColorSampler");
    diffuseColorSampler3 = glGetUniformLocation(shaderProgram0, "diffuseColorSampler1");
    specularColorSampler2 = glGetUniformLocation(shaderProgram0, "specularColorSampler");
    planeLocation0 = glGetUniformLocation(shaderProgram0, "plane");

    
}


void free() {
    glDeleteBuffers(1, &modelVerticiesVBO);
    glDeleteVertexArrays(1, &modelVAO);


    glDeleteProgram(shaderProgram);
    glfwTerminate();
}

void renderReflection(){

    
    glBindFramebuffer(GL_FRAMEBUFFER, ReflectionBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, 1024, 768);
    glUseProgram(shaderProgram0);
    glUniform1f(timeUniform0, (float)glfwGetTime() / 20.0);

   
    

    //camera
    camera->update();
    mat4 projectionMatrix = camera->projectionMatrix;
    vec3 posReflect = camera->position;
    posReflect.y = -posReflect.y+0.71;
    float verticalAngle = camera->verticalAngle;
    float horizontalAngle = camera->horizontalAngle;
    vec3 dirReflect = vec3(
        cos(verticalAngle) * sin(horizontalAngle),
        -sin(verticalAngle),
        cos(verticalAngle) * cos(horizontalAngle)
    );
    vec3 right = vec3(sin(horizontalAngle - 3.14f / 2.0f),
        0,
        cos(horizontalAngle - 3.14f / 2.0f)
    );
    // Up vector
    vec3 up = cross(right, dirReflect);
    mat4 viewMatrix = lookAt(
        posReflect,
        posReflect + dirReflect,
        up);

    mat4 modelMatrix = mat4(1.0);
    glUniformMatrix4fv(VLocation0, 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(PLocation0, 1, GL_FALSE, &projectionMatrix[0][0]);
    vec3 lightpos = 100.0f * vec3(-5.0f, -3.0f, -5.0f);
    vec3 lightPos_cameraspace = vec3(viewMatrix * mat4(1) * vec4(lightpos, 1.0f));
    glUniform3f(LightPosLocation0, lightPos_cameraspace.x, lightPos_cameraspace.y, lightPos_cameraspace.z); // light


     // underlake chack
    vec3 planePosition(0, 0.35, 0);
    mat4 planeTranslation = translate(mat4(), planePosition);

    mat4 planeRotation = mat4(1);

    vec3 planeNormal = vec3(planeRotation * vec4(0, 1, 0, 0));
    float d = -dot(planeNormal, planePosition);
    vec4 planeCoeffs= vec4(planeNormal, d);

    glUniform4f(planeLocation0, planeCoeffs.x, planeCoeffs.y, planeCoeffs.z, planeCoeffs.w);

    //terrain

    modelMatrix = scale(mat4(1), vec3(0.1, 0.1, 0.1));
    modelMatrix = translate(modelMatrix, vec3(0, 0, 0));
    glUniformMatrix4fv(MLocation0, 1, GL_FALSE, &modelMatrix[0][0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, terrainD);
    glUniform1i(diffuseColorSampler2, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, terrainS);
    glUniform1i(specularColorSampler2, 1);
    glUniform1i(textureflag0, 0);
    terrain->bind();
    terrain->draw();

    //trees

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, barkD);
    glUniform1i(diffuseColorSampler2, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, leavesD);
    glUniform1i(specularColorSampler2, 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, leavesT);
    glUniform1i(diffuseColorSampler3, 2);

    modelMatrix = translate(mat4(1), vec3(-1.35, 0.4, -0.6));
    modelMatrix = scale(modelMatrix, vec3(0.02, 0.02, 0.02));
    glUniformMatrix4fv(MLocation0, 1, GL_FALSE, &modelMatrix[0][0]);
    glUniform1i(textureflag0, 1);
    bark->bind();
    bark->draw();
    glUniform1i(textureflag0, 2);
    leaves->bind();
    leaves->draw();
    glUniform1i(textureflag0, 5);
    leaves->bind();
    leaves->draw();
    modelMatrix = translate(mat4(1), vec3(-0.2, 1, -2.7));
    modelMatrix = scale(modelMatrix, vec3(0.02, 0.015, 0.02));
    glUniformMatrix4fv(MLocation0, 1, GL_FALSE, &modelMatrix[0][0]);
    glUniform1i(textureflag0, 1);
    bark->bind();
    bark->draw();
    glUniform1i(textureflag0, 2);
    leaves->bind();
    leaves->draw();
    glUniform1i(textureflag0, 5);
    leaves->bind();
    leaves->draw();
    modelMatrix = translate(mat4(1), vec3(0.2, 0.9, -2.7));
    modelMatrix = scale(modelMatrix, vec3(0.02, 0.02, 0.02));
    glUniformMatrix4fv(MLocation0, 1, GL_FALSE, &modelMatrix[0][0]);
    glUniform1i(textureflag0, 1);
    bark->bind();
    bark->draw();
    glUniform1i(textureflag0, 2);
    leaves->bind();
    leaves->draw();
    glUniform1i(textureflag0, 5);
    leaves->bind();
    leaves->draw();
    modelMatrix = translate(mat4(1), vec3(2.1, 0.6, -0.67));
    modelMatrix = scale(modelMatrix, vec3(0.02, 0.02, 0.02));
    glUniformMatrix4fv(MLocation0, 1, GL_FALSE, &modelMatrix[0][0]);
    glUniform1i(textureflag0, 1);
    bark->bind();
    bark->draw();
    glUniform1i(textureflag0, 2);
    leaves->bind();
    leaves->draw();
    glUniform1i(textureflag0, 5);
    leaves->bind();
    leaves->draw();
    modelMatrix = translate(mat4(1), vec3(2, 0.3, 0.87));
    modelMatrix = scale(modelMatrix, vec3(0.02, 0.02, 0.02));
    glUniformMatrix4fv(MLocation0, 1, GL_FALSE, &modelMatrix[0][0]);
    glUniform1i(textureflag0, 1);
    bark->bind();
    bark->draw();
    glUniform1i(textureflag0, 2);
    leaves->bind();
    leaves->draw();
    glUniform1i(textureflag0, 5);
    leaves->bind();
    leaves->draw();

    //rock
    modelMatrix = translate(mat4(1), vec3(-1.3, 0.3, 1));
    modelMatrix = scale(modelMatrix, vec3(0.3, 0.3, 0.3));
    glUniformMatrix4fv(MLocation0, 1, GL_FALSE, &modelMatrix[0][0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rockD);
    glUniform1i(diffuseColorSampler2, 0);
    glUniform1i(textureflag0, 3);
    rock->bind();
    rock->draw();


    //duck



       

    modelMatrix = scale(mat4(1), vec3(0.3, 0.3, 0.3));
    modelMatrix = translate(modelMatrix, vec3(2.1, 0.9, -1.95));
    modelMatrix = modelMatrix * translate(mat4(1), vec3(0, yH, 0));
    glUniformMatrix4fv(MLocation0, 1, GL_FALSE, &modelMatrix[0][0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, duckD);
    glUniform1i(diffuseColorSampler2, 0);
    glUniform1i(textureflag0, 6);

    duck->bind();
    duck->draw();


    //flyingrock
        
    modelMatrix = translate(mat4(1), vec3(-1.5, -4.3, -4.5));
    modelMatrix = scale(mat4(1), vec3(0.1, 0.1, 0.1)) * modelMatrix * translate(mat4(1), vec3(0, yF, 0));
    glUniformMatrix4fv(MLocation0, 1, GL_FALSE, &modelMatrix[0][0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rockD);
    glUniform1i(diffuseColorSampler2, 0);
    glUniform1i(textureflag0, 3);
    rock->bind();
    rock->draw();





}



void renderRefraction() {


    



}



void mainLoop() {
    do {

        renderReflection();


        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, 1024, 768);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);
        glUniform1f(timeUniform, (float)glfwGetTime() / 20.0);


        // underlake chack
        vec3 planePosition(0, 0.35, 0);
        mat4 planeTranslation = translate(mat4(), planePosition);

        mat4 planeRotation = mat4(1);

        vec3 planeNormal = vec3(planeRotation * vec4(0, 1, 0, 0));
        float d = -dot(planeNormal, planePosition);
        vec4 planeCoeffs = vec4(planeNormal, d);

        glUniform4f(planeLocation, planeCoeffs.x, planeCoeffs.y, planeCoeffs.z, planeCoeffs.w);

        //camera
        camera->update();
        mat4 projectionMatrix = camera->projectionMatrix;
        mat4 viewMatrix = camera->viewMatrix;
        mat4 modelMatrix = mat4(1.0);
        glUniformMatrix4fv(VLocation, 1, GL_FALSE, &viewMatrix[0][0]);
        glUniformMatrix4fv(PLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
        vec3 lightpos = 100.0f * vec3(-5.0f, -3.0f, -5.0f);
        vec3 lightPos_cameraspace = vec3(viewMatrix * mat4(1) * vec4(lightpos, 1.0f));
        glUniform3f(LightPosLocation, lightPos_cameraspace.x, lightPos_cameraspace.y, lightPos_cameraspace.z); // light
        glUniform3f(cameraPos,camera->position.x, camera->position.y, camera->position.z);


        //terrain

        modelMatrix =scale(mat4(1), vec3(0.1, 0.1, 0.1) );
        modelMatrix = translate(modelMatrix, vec3(0, 0, 0));
        glUniformMatrix4fv(MLocation, 1, GL_FALSE, &modelMatrix[0][0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, terrainD);
        glUniform1i(diffuseColorSampler, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, terrainS);
        glUniform1i(specularColorSampler, 1);
        glUniform1i(textureflag, 0);
        terrain->bind();
        terrain->draw();

        //trees

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, barkD);
        glUniform1i(diffuseColorSampler, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, leavesD);
        glUniform1i(specularColorSampler, 1);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, leavesT);
        glUniform1i(diffuseColorSampler1, 2);

        modelMatrix = translate(mat4(1), vec3(-1.35, 0.4, -0.6));
        modelMatrix = scale(modelMatrix, vec3(0.02, 0.02, 0.02));
        glUniformMatrix4fv(MLocation, 1, GL_FALSE, &modelMatrix[0][0]);
        glUniform1i(textureflag, 1);
        bark->bind();
        bark->draw();
        glUniform1i(textureflag, 2);
        leaves->bind();
        leaves->draw();
        glUniform1i(textureflag, 5);
        leaves->bind();
        leaves->draw();
        modelMatrix = translate(mat4(1), vec3(-0.2, 1, -2.7));
        modelMatrix = scale(modelMatrix, vec3(0.02, 0.015, 0.02));
        glUniformMatrix4fv(MLocation, 1, GL_FALSE, &modelMatrix[0][0]);
        glUniform1i(textureflag, 1);
        bark->bind();
        bark->draw();
        glUniform1i(textureflag, 2);
        leaves->bind();
        leaves->draw();
        glUniform1i(textureflag, 5);
        leaves->bind();
        leaves->draw();
        modelMatrix = translate(mat4(1), vec3(0.2, 0.9, -2.7));
        modelMatrix = scale(modelMatrix, vec3(0.02, 0.02, 0.02));
        glUniformMatrix4fv(MLocation, 1, GL_FALSE, &modelMatrix[0][0]);
        glUniform1i(textureflag, 1);
        bark->bind();
        bark->draw();
        glUniform1i(textureflag, 2);
        leaves->bind();
        leaves->draw();
        glUniform1i(textureflag, 5);
        leaves->bind();
        leaves->draw();
        modelMatrix = translate(mat4(1), vec3(2.1, 0.6, -0.67));
        modelMatrix = scale(modelMatrix, vec3(0.02, 0.02, 0.02));
        glUniformMatrix4fv(MLocation, 1, GL_FALSE, &modelMatrix[0][0]);
        glUniform1i(textureflag, 1);
        bark->bind();
        bark->draw();
        glUniform1i(textureflag, 2);
        leaves->bind();
        leaves->draw();
        glUniform1i(textureflag, 5);
        leaves->bind();
        leaves->draw();
        modelMatrix = translate(mat4(1), vec3(2, 0.3, 0.87));
        modelMatrix = scale(modelMatrix, vec3(0.02, 0.02, 0.02));
        glUniformMatrix4fv(MLocation, 1, GL_FALSE, &modelMatrix[0][0]);
        glUniform1i(textureflag, 1);
        bark->bind();
        bark->draw();
        glUniform1i(textureflag, 2);
        leaves->bind();
        leaves->draw();
        glUniform1i(textureflag, 5);
        leaves->bind();
        leaves->draw();

        //rock
        modelMatrix = translate(mat4(1), vec3(-1.3, 0.3, 1));
        modelMatrix = scale(modelMatrix, vec3(0.3, 0.3, 0.3));
        glUniformMatrix4fv(MLocation, 1, GL_FALSE, &modelMatrix[0][0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rockD);
        glUniform1i(diffuseColorSampler, 0);
        glUniform1i(textureflag, 3);
        rock->bind();
        rock->draw();
        

        //duck
        
       

        if ((yH > 0) && (timeH == 0)) {
            yH = yH - 0.07;
            
        }
        else if ((yH < 0) && (timeH == 0)) {
            yH = 0;
            timeH = (float)glfwGetTime();

        }
        else {
             
             float wave = (float)glfwGetTime() - timeH;
             yH =-(1.03 / pow(1.03, wave)) / 30 * sin(wave / 0.2); 
             glUniform1f(wave0time, wave);
             glUniform3f(wave0point, 0.2,0,-0.1);
        }
        
        
        modelMatrix = scale(mat4(1), vec3(0.3, 0.3, 0.3));
        modelMatrix = translate(modelMatrix, vec3(2.1, 0.9, -1.95));
        modelMatrix = modelMatrix * translate(mat4(1), vec3(0, yH, 0));
        glUniformMatrix4fv(MLocation, 1, GL_FALSE, &modelMatrix[0][0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, duckD);
        glUniform1i(diffuseColorSampler, 0);
        glUniform1i(textureflag, 6);

        duck->bind();
        duck->draw();


        //flyingrock
        if (yF > 7) {
            yF = yF - 0.2;

        }
        else if (yF <7 && timeF==0) {
            timeF = (float)glfwGetTime();

        }
        else if (yF > 0) {
            yF = yF - 0.01;

        }
        else if ((yF < 0) ) {
            yF = 0;

        }
        if (timeF>0) {
            float wave = (float)glfwGetTime() - timeF;
            glUniform1f(wave1time, wave);
            glUniform3f(wave1point, -0.25, 0, -0.1);
        }
        modelMatrix = translate(mat4(1), vec3(-1.5, -4.3, -4.5));
        modelMatrix = scale(mat4(1), vec3(0.1, 0.1, 0.1)) * modelMatrix * translate(mat4(1), vec3(0, yF, 0));
        glUniformMatrix4fv(MLocation, 1, GL_FALSE, &modelMatrix[0][0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rockD);
        glUniform1i(diffuseColorSampler, 0);
        glUniform1i(textureflag, 3);
        rock->bind();
        rock->draw();


        //lake
        modelMatrix = scale(mat4(1), vec3(2, 2, 2 ));
        modelMatrix = translate(modelMatrix, vec3(0.14, 0.16, -0.2));
        glUniformMatrix4fv(MLocation, 1, GL_FALSE, &modelMatrix[0][0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, lake0);
        glUniform1i(diffuseColorSampler, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, lake1);
        glUniform1i(specularColorSampler, 1);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, renderedTexture);
        glUniform1i(diffuseColorSampler1, 2);
        glUniform1i(textureflag, 4);
        lake->bind();
        lake->draw();

        



        glfwSwapBuffers(window);
        glfwPollEvents();
    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
             glfwWindowShouldClose(window) == 0);
}

void pollKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {     //mods = Shift or Control or etc., key=key pressed
   

    // Task 3.4: toggle polygon mode
    if (key == GLFW_KEY_T && action == GLFW_PRESS) { // We use GLFW_PRESS because if we don't the GLFW_KEY_T to be true for the next frame as well
        GLint polygonMode[2];
        glGetIntegerv(GL_POLYGON_MODE, &polygonMode[0]);

        // if GL_LINE, if GL_FILL check with polygonMode[0]
        if (polygonMode[0] == GL_LINE) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        else if (polygonMode[0] == GL_FILL) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    if (key == GLFW_KEY_H && action == GLFW_PRESS) { 
        timeH=0;
        yH = 2;
    }

    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        timeF = 0;
        yF = 20;
    }
}

void initialize() {
    // Initialize GLFW
    if (!glfwInit()) {
        throw runtime_error("Failed to initialize GLFW\n");
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(W_WIDTH, W_HEIGHT, TITLE, NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        throw runtime_error(string(string("Failed to open GLFW window.") +
                            " If you have an Intel GPU, they are not 3.3 compatible." +
                            "Try the 2.1 version.\n"));
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Start GLEW extension handler
    glewExperimental = GL_TRUE;

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        glfwTerminate();
        throw runtime_error("Failed to initialize GLEW\n");
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Hide the mouse and enable unlimited movement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, W_WIDTH / 2, W_HEIGHT / 2);

    // Gray background color
    glClearColor(0.5f, 0.5f, 0.5f, 0.0f);

    glfwSetKeyCallback(window, pollKeyboard);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);

    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //*/

    // Log
    logGLParameters();

    // Create camera
    camera = new Camera(window);

}

int main(void) {
    try {
        initialize();
        createContext();
        mainLoop();
        free();
    } catch (exception& ex) {
        cout << ex.what() << endl;
        getchar();
        free();
        return -1;
    }
    return 0;
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}