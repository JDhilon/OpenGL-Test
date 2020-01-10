//
//  OpenGL-Test
//
//  Created by Jashanjot Dhilon on 2019-11-26.
//  Copyright © 2019 Jashanjot Dhilon. All rights reserved.
//

#define STB_IMAGE_IMPLEMENTATION

#ifndef _DEBUG
#define _DEBUG
#endif

// #define PHYSTESTING

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <chrono>

// TODO: Adjust these includes accordingly for various development enviroments
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "common_values.hpp"

#include "window.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "texture.hpp"
#include "directional_light.hpp"
#include "point_light.hpp"
#include "spot_light.hpp"
#include "material.hpp"
#include "model.hpp"

#include "PxPhysicsAPI.h"

/*
 PhysX Stuff
 */

physx::PxDefaultAllocator gAllocator;
physx::PxDefaultErrorCallback gErrorCallback;

physx::PxFoundation* gFoundation = NULL;
physx::PxPhysics* gPhysics = NULL;

physx::PxDefaultCpuDispatcher* gDispatcher = NULL;
physx::PxScene* gScene = NULL;

physx::PxCooking* gCooking = NULL;

physx::PxMaterial* gMaterial = NULL;

float accumulator = 0.0f;
float simulationStep = 1.0f / 60.0f;

struct Object {
    Model* model;
    physx::PxRigidDynamic* collision;
};

Object spaceshipObject;

/*
 PhysX Stuff
 */

const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;
Camera camera;

Texture brickTexture;
Texture dirtTexture;
Texture plainTexture;

Material shinyMaterial;
Material dullMaterial;

Model spaceship;

DirectionalLight mainLight;
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

// Vertex Shader
static const char* vShader = "Shaders/shader.vert";

// Fragment Shader
static const char* fShader = "Shaders/shader.frag";

void calcAverageNormals(unsigned int * indices, unsigned int indiceCount, GLfloat * vertices, unsigned int verticeCount,
                        unsigned int vLength, unsigned int normalOffset)
{
    for (size_t i = 0; i < indiceCount; i += 3)
    {
        unsigned int in0 = indices[i] * vLength;
        unsigned int in1 = indices[i + 1] * vLength;
        unsigned int in2 = indices[i + 2] * vLength;
        glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
        glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
        glm::vec3 normal = glm::cross(v1, v2);
        normal = glm::normalize(normal);
        
        in0 += normalOffset; in1 += normalOffset; in2 += normalOffset;
        vertices[in0] += normal.x; vertices[in0 + 1] += normal.y; vertices[in0 + 2] += normal.z;
        vertices[in1] += normal.x; vertices[in1 + 1] += normal.y; vertices[in1 + 2] += normal.z;
        vertices[in2] += normal.x; vertices[in2 + 1] += normal.y; vertices[in2 + 2] += normal.z;
    }
    
    for (size_t i = 0; i < verticeCount / vLength; i++)
    {
        unsigned int nOffset = i * vLength + normalOffset;
        glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
        vec = glm::normalize(vec);
        vertices[nOffset] = vec.x; vertices[nOffset + 1] = vec.y; vertices[nOffset + 2] = vec.z;
    }
}

void CreateObjects()
{
    unsigned int indices[] = {
        0, 3, 1,
        1, 3, 2,
        2, 3, 0,
        0, 1, 2
    };
    
    GLfloat vertices[] = {
        //    x      y      z            u      v            nx      ny    nz
        -1.0f, -1.0f, -0.6f,        0.0f, 0.0f,        0.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 1.0f,        0.5f, 0.0f,        0.0f, 0.0f, 0.0f,
        1.0f, -1.0f, -0.6f,        1.0f, 0.0f,        0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,        0.5f, 1.0f,        0.0f, 0.0f, 0.0f
    };
    
    unsigned int floorIndices[] = {
        0, 2, 1,
        1, 2, 3
    };
    
    GLfloat floorVertices[] = {
        -10.0f, 0.0f, -10.0f,    0.0f, 0.0f,        0.0f, -1.0f, 0.0f,
        10.0f, 0.0f, -10.0f,    10.0f, 0.0f,    0.0f, -1.0f, 0.0f,
        -10.0f, 0.0f, 10.0f,    0.0f, 10.0f,    0.0f, -1.0f, 0.0f,
        10.0f, 0.0f, 10.0f,        10.0f, 10.0f,    0.0f, -1.0f, 0.0f
    };
    
    calcAverageNormals(indices, 12, vertices, 32, 8, 5);
    
    Mesh *obj1 = new Mesh();
    obj1->CreateMesh(vertices, indices, 32, 12);
    meshList.push_back(obj1);
    
    Mesh *obj2 = new Mesh();
    obj2->CreateMesh(vertices, indices, 32, 12);
    meshList.push_back(obj2);
    
    Mesh *obj3 = new Mesh();
    obj3->CreateMesh(floorVertices, floorIndices, 32, 6);
    meshList.push_back(obj3);
}

void CreateShaders()
{
    Shader *shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*shader1);
}

physx::PxRigidDynamic* createDynamic(const physx::PxTransform& t, const physx::PxGeometry& geometry, const physx::PxVec3& velocity=physx::PxVec3(0))
{
    physx::PxRigidDynamic* dynamic = PxCreateDynamic(*gPhysics, t, geometry, *gMaterial, 10.0f);
    dynamic->setAngularDamping(0.5f);
    dynamic->setLinearVelocity(velocity);
    gScene->addActor(*dynamic);
    return dynamic;
}

physx::PxRigidStatic* createStatic(const physx::PxTransform& t, const physx::PxGeometry& geometry)
{
    physx::PxRigidStatic* stat = PxCreateStatic(*gPhysics, t, geometry, *gMaterial);
    gScene->addActor(*stat);
    return stat;
}

void SetupPhysX()
{
    gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator,
                                     gErrorCallback);
    if(!gFoundation) {
        std::cout << "PxCreateFoundation failed!";
    }
    
    bool recordMemoryAllocations = true;
    
    gPhysics = PxCreateBasePhysics(PX_PHYSICS_VERSION, *gFoundation,
                               physx::PxTolerancesScale());
    
    PxRegisterHeightFields(*gPhysics);
    
    if(!gPhysics) {
        std::cout << "PxCreatePhysics failed!";
    }
    
    physx::PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
    sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
    sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
    
    physx::PxU32 numWorkers = 1;
    gDispatcher = physx::PxDefaultCpuDispatcherCreate(numWorkers);
    sceneDesc.cpuDispatcher = gDispatcher;
    
    gScene = gPhysics->createScene(sceneDesc);
    
    gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);
}
void CreatePhysicsObjects()
{
    // Create ball
    auto col_box = createDynamic(physx::PxTransform(physx::PxVec3(0,5,-3)), physx::PxBoxGeometry(2, 2, 2.5), physx::PxVec3(0,0,0));
    spaceshipObject.collision = col_box;
    
    // Create floor
    auto floor_volume = createStatic(physx::PxTransform(physx::PxVec3(0,-4.5,0)), physx::PxBoxGeometry(8, 1.25, 8));
}

int main()
{
    SetupPhysX();
    CreatePhysicsObjects();
    
    mainWindow = Window(1366, 768); // 1280, 1024 or 1024, 768
    mainWindow.Initialise();
    
    CreateObjects();
    CreateShaders();
    
    camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 5.0f, 0.5f);
    
    brickTexture = Texture("Textures/brick.png");
    brickTexture.LoadTextureA();
    dirtTexture = Texture("Textures/dirt.png");
    dirtTexture.LoadTextureA();
    plainTexture = Texture("Textures/plain.png");
    plainTexture.LoadTextureA();
    
    shinyMaterial = Material(4.0f, 256);
    dullMaterial = Material(0.3f, 4);
    
    #ifndef PHYSTESTING
    spaceship = Model();
    spaceship.LoadModel("Models/E45_Aircraft_obj.obj");
    spaceship.SetTranslate(glm::vec3(0.0f, 0.0f, 0.0f));
    spaceship.SetScale(glm::vec3(0.2f, 0.2f, 0.2f));
    #endif
    
    spaceshipObject.model = &spaceship;
    
    mainLight = DirectionalLight(1.0f, 1.0f, 1.0f,
                                 0.3f, 0.3f,
                                 0.0f, 0.0f, -1.0f);
    
    unsigned int pointLightCount = 0;
    pointLights[0] = PointLight(1.0f, 1.0f, 1.0f,
                                0.0f, 0.1f,
                                0.0f, 0.0f, 0.0f,
                                0.3f, 0.2f, 0.1f);
    pointLightCount++;
    pointLights[1] = PointLight(1.0f, 1.0f, 1.0f,
                                0.0f, 0.1f,
                                -4.0f, 2.0f, 0.0f,
                                0.3f, 0.1f, 0.1f);
    pointLightCount++;
    
    unsigned int spotLightCount = 0;
    spotLights[0] = SpotLight(1.0f, 1.0f, 1.0f,
                              0.0f, 2.0f,
                              0.0f, 0.0f, 0.0f,
                              0.0f, -1.0f, 0.0f,
                              1.0f, 0.0f, 0.0f,
                              20.0f);
    spotLightCount++;
    spotLights[1] = SpotLight(1.0f, 1.0f, 1.0f,
                              0.0f, 1.0f,
                              0.0f, -1.5f, 0.0f,
                              -100.0f, -1.0f, 0.0f,
                              1.0f, 0.0f, 0.0f,
                              20.0f);
    spotLightCount++;
    
    GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
    uniformSpecularIntensity = 0, uniformShininess = 0;
    glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 100.0f);
    
    // Loop until window closed
    while (!mainWindow.getShouldClose())
    {
        GLfloat now = glfwGetTime(); // SDL_GetPerformanceCounter();
        deltaTime = now - lastTime; // (now - lastTime)*1000/SDL_GetPerformanceFrequency();
        lastTime = now;
        
        // Get + Handle User Input
        glfwPollEvents();
        
        // Add random force if p is pushed
        if(mainWindow.getsKeys()[GLFW_KEY_P]) {
            spaceshipObject.collision->addForce(physx::PxVec3(0, 10000, 0));
        }
        
        accumulator += deltaTime;
        if(accumulator > simulationStep) {
            accumulator -= simulationStep;
            gScene->simulate(simulationStep);
            gScene->fetchResults(true);
        }
        
        auto pos = spaceshipObject.collision->getGlobalPose().p;
        auto quat = spaceshipObject.collision->getGlobalPose().q;
        float pos_x = pos.x;
        float pos_y = pos.y;
        float pos_z = pos.z;
        /*
        std::cout << "X: " << pos_x << std::endl;
        std::cout << "Y: " << pos_y << std::endl;
        std::cout << "Z: " << pos_z << std::endl << std::endl;
         */
        spaceshipObject.model->SetTranslate(glm::vec3(pos_x, pos_y, pos_z));
        spaceshipObject.model->SetQuat(glm::quat(quat.w, quat.x, quat.y, quat.z));
        
        camera.keyControl(mainWindow.getsKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());
        // TODO: add mouse button control to handle mouse press
        
        // Clear the window
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        shaderList[0].UseShader();
        uniformModel = shaderList[0].GetModelLocation();
        uniformProjection = shaderList[0].GetProjectionLocation();
        uniformView = shaderList[0].GetViewLocation();
        uniformEyePosition = shaderList[0].GetEyePositionLocation();
        uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
        uniformShininess = shaderList[0].GetShininessLocation();
        
        glm::vec3 lowerLight = camera.getCameraPosition();
        lowerLight.y -= 0.3f;
        if(mainWindow.getsKeys()[GLFW_KEY_F]) {
            spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());
        }
        else {
            spotLights[0].SetFlash(glm::vec3(0,0,0), glm::vec3(0,0,0));
        }
        
        shaderList[0].SetDirectionalLight(&mainLight);
        shaderList[0].SetPointLights(pointLights, pointLightCount);
        shaderList[0].SetSpotLights(spotLights, spotLightCount);
        
        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
        glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);
        
        glm::mat4 model = glm::mat4(1.0f);
        
        /*
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -2.5f));
        //model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        brickTexture.UseTexture();
        shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshList[0]->RenderMesh();
        */
        
        /*
        model = glm::mat4() = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 4.0f, -2.5f));
        //model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        dirtTexture.UseTexture();
        dullMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshList[1]->RenderMesh();
         */
        
        model = glm::mat4() = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
        //model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        plainTexture.UseTexture();
        dullMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshList[2]->RenderMesh();
        
        #ifndef PHYSTESTING
        model = spaceship.GetModelMatrix();
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
        spaceship.RenderModel();
        #endif
        
        glUseProgram(0);
        
        mainWindow.swapBuffers();
    }
    
    gPhysics->release();
    gFoundation->release();
    
    return 0;
}
