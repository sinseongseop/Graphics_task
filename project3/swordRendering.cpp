#define _CRT_SECURE_NO_WARNINGS
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>
#undef STB_IMAGE_IMPLEMENTATION

#include <iostream>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "glfw3.lib")

// 모델을 읽기 위해 assimp 라이브러리 추가
#pragma comment(lib, "assimp-vc143-mt.lib")

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
void renderSphere();
void renderOBJ(Mesh &mesh, int i);

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// 그림자를 표시하기 위한 바닥 VAO
unsigned int planeVAO;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    glfwMakeContextCurrent(window);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glew: load all OpenGL function pointers
    glewInit();

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader physicalShader("src/1.2.pbr.vs", "src/1.2.pbr.fs");
    Shader shadowShader("src/3.1.3.shadow_mapping.vs", "src/3.1.3.shadow_mapping.fs");
    Shader simpleDepthShader("src/3.1.3.shadow_mapping_depth.vs", "src/3.1.3.shadow_mapping_depth.fs");
    Shader debugDepthQuad("src/3.1.3.debug_quad.vs", "src/3.1.3.debug_quad_depth.fs");

    //바닥의 위치 정의
// ------------------------------------------------------------------
    float planeVertices[] = {
        // positions            // normals         // texcoords
        -25.0f, -25.0f,  -20.0f,  0.0f, 0.0f, 1.0f,  25.0f,  0.0f,
         25.0f, -25.0f,  -20.0f,  0.0f, 0.0f, 1.0f,   0.0f,  0.0f,
         25.0f,  25.0f,  -20.0f,  0.0f, 0.0f, 1.0f,   0.0f, 25.0f,

        -25.0f, -25.0f,  -20.0f,  0.0f, 0.0f, 1.0f,  25.0f,  0.0f,
         25.0f,  25.0f,  -20.0f,  0.0f, 0.0f, 1.0f,   0.0f, 25.0f,
        -25.0f,  25.0f,  -20.0f,  0.0f, 0.0f, 1.0f,  25.0f, 25.0f
    };

    //바닥 VBO 정의
    unsigned int planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    // load Plane textures
    unsigned int woodTexture = loadTexture(FileSystem::getPath("../resources/textures/wood.png").c_str());

    // configure depth map FBO
    // -----------------------
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);

    // create depth texture
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //shader configuration
    // --------------------
    shadowShader.use();
    shadowShader.setInt("diffuseTexture", 0);
    shadowShader.setInt("shadowMap", 1);
    debugDepthQuad.use();
    debugDepthQuad.setInt("depthMap", 0);

    // 필요한 모델 로딩
    Model ourModel(FileSystem::getPath("../resources/sword/sword.obj"));

    physicalShader.use();
    physicalShader.setInt("albedoMap", 0);
    physicalShader.setInt("normalMap", 1);
    physicalShader.setInt("metallicMap", 2);
    physicalShader.setInt("roughnessMap", 3);
    physicalShader.setInt("aoMap", 4);

    unsigned int albedo = loadTexture(FileSystem::getPath("../resources/sword/baseColor.png").c_str());
    unsigned int normal = loadTexture(FileSystem::getPath("../resources/sword/normal.png").c_str());
    unsigned int metallic = loadTexture(FileSystem::getPath("../resources/sword/metalic.png").c_str());
    unsigned int roughness = loadTexture(FileSystem::getPath("../resources/sword/roughness.png").c_str());
    unsigned int ao = loadTexture(FileSystem::getPath("../resources/sword/ao.png").c_str());

    // lights
    // ------
    glm::vec3 lightPositions[] = {
        glm::vec3(0.0f, 0.0f, 10.0f),
    };

    glm::vec3 lightColors[] = {
        glm::vec3(150.0f, 150.0f, 150.0f),
    };

    int nrRows = 7;
    int nrColumns = 7;
    float spacing = 2.5;

    // initialize static physicalShader uniforms before rendering
    // --------------------------------------------------
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    physicalShader.use();
    physicalShader.setMat4("projection", projection);

    // render loop
    // -----------

    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);

    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 1. render depth of scene to texture (from light's perspective)
// --------------------------------------------------------------
        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        float near_plane = 0.0f, far_plane = 100.0f;
        //lightProjection = glm::perspective(glm::radians(45.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane); // note that if you use a perspective projection matrix you'll have to change the light position as the current light position isn't enough to reflect the whole scene
        lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        lightView = glm::lookAt(lightPositions[0], glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;
        // render scene from light's point of view
        simpleDepthShader.use();
        simpleDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTexture);

        //floor rendering
        model = glm::mat4(1.0f);
        simpleDepthShader.setMat4("model", model);
        glBindVertexArray(planeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //obj 파일 랜더링.
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0, 0, -10));
        model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
        simpleDepthShader.setMat4("model", model);

        vector<Mesh> triangleMeshes = ourModel.meshes;

        // 각 트라이앵글 메쉬를 렌더링
        for (std::size_t i = 0; i < triangleMeshes.size(); ++i) {
            renderOBJ(triangleMeshes[i], static_cast<int>(i));
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // reset viewport
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 2. render scene as normal using the generated depth/shadow map  
        // --------------------------------------------------------------
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shadowShader.use();
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view = camera.GetViewMatrix();
        shadowShader.setMat4("projection", projection);
        shadowShader.setMat4("view", view);
        // set light uniforms
        shadowShader.setVec3("viewPos", camera.Position);
        shadowShader.setVec3("lightPos", lightPositions[0]);
        shadowShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);

        //floor rendering
        model = glm::mat4(1.0f);
        shadowShader.setMat4("model", model);
        glBindVertexArray(planeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // render Depth map to quad for visual debugging
        // ---------------------------------------------
        debugDepthQuad.use();
        debugDepthQuad.setFloat("near_plane", near_plane);
        debugDepthQuad.setFloat("far_plane", far_plane);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        //renderQuad();

		// 3. 칼을 물리 기반 랜더링
        physicalShader.use();
        glm::mat4 view = camera.GetViewMatrix();
        physicalShader.setMat4("view", view);
        physicalShader.setVec3("camPos", camera.Position);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, albedo);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, metallic);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, roughness);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, ao);

        //빛 랜더링
        for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
        {
            glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
            newPos = lightPositions[i];
            physicalShader.setVec3("lightPositions[" + std::to_string(i) + "]", newPos);
            physicalShader.setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);

            model = glm::mat4(1.0f);
            model = glm::translate(model, newPos);
            model = glm::scale(model, glm::vec3(0.5f));
            physicalShader.setMat4("model", model);
            //renderSphere();
        }

        //obj 파일 랜더링.
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0, 0, -10));
        model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
        physicalShader.setMat4("model", model);

        triangleMeshes = ourModel.meshes;
        // 각 트라이앵글 메쉬를 렌더링
        for (std::size_t i = 0; i < triangleMeshes.size(); ++i) {
            renderOBJ(triangleMeshes[i], static_cast<int>(i));
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
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

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
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

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

// renders (and builds at first invocation) a sphere
// -------------------------------------------------

// load triangleMesh to buffer
// -------------------------------------------------
unsigned int sphereVAO = 0;
unsigned int indexCount = 0;

//트라이앵글 Mesh들 관련 정보를 모두 저장
vector<unsigned int> OBJVaos;
vector<unsigned int> OBJVbos;
vector<unsigned int> OBJEbos;

//Vertex 정보와 인덱스 정보를 저장
vector<std::vector<float>> vertexInfos;
vector<std::vector<unsigned int>> indicesInfos;

//renderSphere 함수를 참고하여 OBJ 파일을 렌더링하는 함수
void renderOBJ(Mesh& triangleMesh, int index) {

    //VAO가 덜 생성된 경우
    if (OBJVaos.size() <= index ) {
        unsigned int vao, vbo, ebo;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        OBJVaos.push_back(vao);
        OBJVbos.push_back(vbo);
        OBJEbos.push_back(ebo);

        std::vector<glm::vec3> positions; //정점 위치
        std::vector<glm::vec2> uvs; // 텍스쳐 좌표 위치
        std::vector<glm::vec3> normals; // 법선 벡터 
        std::vector<unsigned int> indices; //인덱스

        // 메쉬의 각 버텍스 정보를 positions, uvs, normals 벡터에 저장
        for (const auto& vertex : triangleMesh.vertices) {
            positions.push_back(vertex.Position);
            uvs.push_back(vertex.TexCoords);
            normals.push_back(vertex.Normal);
        }

        // 매쉬의 인덱스 정보를 indices 벡터에 저장
        indices.assign(triangleMesh.indices.begin(), triangleMesh.indices.end());

        std::vector<float> vertexInfo;
        for (std::size_t i = 0; i < positions.size(); ++i) {
            vertexInfo.push_back(positions[i].x);
            vertexInfo.push_back(positions[i].y);
            vertexInfo.push_back(positions[i].z);
            if (!uvs.empty()) {
                vertexInfo.push_back(uvs[i].x);
                vertexInfo.push_back(uvs[i].y);
            }
            if (!normals.empty()) {
                vertexInfo.push_back(normals[i].x);
                vertexInfo.push_back(normals[i].y);
                vertexInfo.push_back(normals[i].z);
            }
        }

        vertexInfos.push_back(vertexInfo);
        indicesInfos.push_back(indices);

        // VAO, VBO, EBO 바인딩 및 데이터 설정
        glBindVertexArray(OBJVaos.at(index));
        glBindBuffer(GL_ARRAY_BUFFER, OBJVbos.at(index));
        glBufferData(GL_ARRAY_BUFFER, vertexInfos.at(index).size() * sizeof(float), vertexInfos[index].data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OBJEbos.at(index));
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesInfos.at(index).size() * sizeof(unsigned int), indicesInfos[index].data(), GL_STATIC_DRAW);

        // 버텍스 속성 포인터 설정
        float stride = (3 + 2 + 3) * sizeof(float);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
    }

    // 트라이앵글 메쉬 렌더링
    glBindVertexArray(OBJVaos[index]);
    glDrawElements(GL_TRIANGLES, indicesInfos[index].size(), GL_UNSIGNED_INT, 0);
}

// 구 렌더링 함수
void renderSphere()
{
    // sphereVAO가 0이면 VAO, VBO, EBO를 생성하고 데이터를 설정
    if (sphereVAO == 0)
    {
        glGenVertexArrays(1, &sphereVAO);

        unsigned int vbo, ebo;
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uv;
        std::vector<glm::vec3> normals;
        std::vector<unsigned int> indices;

        const unsigned int X_SEGMENTS = 64;
        const unsigned int Y_SEGMENTS = 64;
        const float PI = 3.14159265359;

        // 구의 각 정점의 위치, UV 좌표, 법선 벡터를 계산하여 저장
        for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
        {
            for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
            {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                positions.push_back(glm::vec3(xPos, yPos, zPos));
                uv.push_back(glm::vec2(xSegment, ySegment));
                normals.push_back(glm::vec3(xPos, yPos, zPos));
            }
        }

        bool oddRow = false;
        // 인덱스를 계산하여 저장
        for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
        {
            if (!oddRow) // 짝수 행: y == 0, y == 2 등
            {
                for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
                {
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                }
            }
            else // 홀수 행: y == 1, y == 3 등
            {
                for (int x = X_SEGMENTS; x >= 0; --x)
                {
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                }
            }
            oddRow = !oddRow;
        }
        indexCount = indices.size();

        std::vector<float> data;
        // 정점 데이터를 하나의 배열로 결합
        for (std::size_t i = 0; i < positions.size(); ++i)
        {
            data.push_back(positions[i].x);
            data.push_back(positions[i].y);
            data.push_back(positions[i].z);
            if (uv.size() > 0)
            {
                data.push_back(uv[i].x);
                data.push_back(uv[i].y);
            }
            if (normals.size() > 0)
            {
                data.push_back(normals[i].x);
                data.push_back(normals[i].y);
                data.push_back(normals[i].z);
            }
        }

        // VAO, VBO, EBO에 데이터 바인딩
        glBindVertexArray(sphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
        float stride = (3 + 2 + 3) * sizeof(float);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
    }

    // 구를 렌더링
    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
