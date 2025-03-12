//
// Display a color cube
//
// Colors are assigned to each vertex and then the rasterizer interpolates
//   those colors across the triangles.  We us an orthographic projection
//   as the default projetion.

#include "cube.h"
#include "sphere.h" // 구 랜더링을 위한 헤더파일
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "texture.hpp"

glm::mat4 projectMat;
glm::mat4 viewMat;

GLuint pvmMatrixID;
GLuint projectMatrixID;
GLuint viewMatrixID;
GLuint modelMatrixID;
GLuint shadeModeID;
GLuint textureModeID;

// 각각의 도형을 담기 위한 VAO 정의
GLuint CubeVao,sphereVao;

// 구 객체 생성 및 초기화
Sphere sphere(50, 50);

GLuint waterTexture, earthTexture;


// 각도 관련 전역변수 정의
int animationMode = 0; // 몇번째 KeyFrame을 지났는지 표시
float rotAngle = 0.0f;

float leftLowerArmAngle = 0.0f;
float rightLowerArmAngle = 0.0f;
float leftLowerLegAngle = 0.0f;
float rightLowerLegAngle = 0.0f;


float KeyFrameAngle[8] = {
	0.0f,
	glm::radians(20.0f),
	glm::radians(70.0f),
	glm::radians(20.0f),
	0.0f,
	glm::radians(-20.0f),
	glm::radians(-70.0f),
	glm::radians(-20.0f)
}; // KeyFrame 기준이 되는 윗팔의 각도

float Angle[4][8] = {
	{0.0f, glm::radians(30.0f), glm::radians(90.0f), glm::radians(30.0f), 0.0f, glm::radians(-20.0f), glm::radians(-70.0f), glm::radians(-20.0f)}, // 왼쪽 팔 아래 각도
	{0.0f, glm::radians(-20.0f), glm::radians(-70.0f), glm::radians(-20.0f), 0.0f, glm::radians(30.0f), glm::radians(90.0f), glm::radians(30.0f)}, // 오른쪽 팔 아래 각도
	{0.0f, glm::radians(70.0f), glm::radians(120.0f), glm::radians(70.0f), 0.0f, glm::radians(-30.0f), glm::radians(0.0f), glm::radians(-30.0f)}, // 왼쪽 다리 아래 각도
	{0.0f, glm::radians(-30.0f), glm::radians(-0.0f), glm::radians(-30.0f), 0.0f, glm::radians(70.0f), glm::radians(120.0f), glm::radians(70.0f)}  // 오른쪽 다리 아래 각도
};

// 몸 움직임 관련 전역변수 정의
float bodyPostionMove = 0.0f; // 몸을 Z축 방향 움직인 위치
int bodyMoveDirection = 0; // 0이면 위쪽으로 이동 중, 1이면 아래쪽으로 이동중
float MaxbodyPosition = 0.25; // 최대 z값
float LowBodyPosition = 0.0; // 최소 z값

int isDrawingCar = false;
int cameraMode = 1; //카메라 모드 정의

typedef glm::vec4  color4;
typedef glm::vec4  point4;
typedef glm::vec4  normal4;
typedef glm::vec2  texCoord2;


const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

point4 points[NumVertices];
color4 colors[NumVertices];
normal4 normals[NumVertices];
texCoord2 texCoords[NumVertices];

// Vertices of a unit cube centered at origin, sides aligned with axes
point4 vertices[8] = {
	point4(-0.5, -0.5, 0.5, 1.0),
	point4(-0.5, 0.5, 0.5, 1.0),
	point4(0.5, 0.5, 0.5, 1.0),
	point4(0.5, -0.5, 0.5, 1.0),
	point4(-0.5, -0.5, -0.5, 1.0),
	point4(-0.5, 0.5, -0.5, 1.0),
	point4(0.5, 0.5, -0.5, 1.0),
	point4(0.5, -0.5, -0.5, 1.0)
};

// RGBA colors
color4 vertex_colors[8] = {
	color4(0.0, 0.0, 0.0, 1.0),  // black
	color4(0.0, 1.0, 1.0, 1.0),   // cyan
	color4(1.0, 0.0, 1.0, 1.0),  // magenta
	color4(1.0, 1.0, 0.0, 1.0),  // yellow
	color4(1.0, 0.0, 0.0, 1.0),  // red
	color4(0.0, 1.0, 0.0, 1.0),  // green
	color4(0.0, 0.0, 1.0, 1.0),  // blue
	color4(1.0, 1.0, 1.0, 1.0)  // white
};

//----------------------------------------------------------------------------

// quad generates two triangles for each face and assigns colors
//    to the vertices
int Index = 0;
void
quad(int a, int b, int c, int d)
{
	//lighting을 위한 Normal Vecter 정의
	glm::vec3 normal = glm::normalize(glm::cross(
		glm::vec3(vertices[b]) - glm::vec3(vertices[a]),
		glm::vec3(vertices[c]) - glm::vec3(vertices[a])
	));

	// vec4 타입으로 변환
	glm::vec4 normalVec4 = glm::vec4(normal, 0.0f);

	// texture 좌표 정의
	glm::vec2 quadTexCoords[4] = {
		glm::vec2(0.0f, 0.0f),
		glm::vec2(1.0f, 0.0f),
		glm::vec2(1.0f, 1.0f),
		glm::vec2(0.0f, 1.0f)
	};

	colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; normals[Index] = normalVec4; texCoords[Index] = quadTexCoords[0]; Index++;
	colors[Index] = vertex_colors[b]; points[Index] = vertices[b]; normals[Index] = normalVec4; texCoords[Index] = quadTexCoords[1]; Index++;
	colors[Index] = vertex_colors[c]; points[Index] = vertices[c]; normals[Index] = normalVec4; texCoords[Index] = quadTexCoords[2]; Index++;
	colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; normals[Index] = normalVec4; texCoords[Index] = quadTexCoords[0]; Index++;
	colors[Index] = vertex_colors[c]; points[Index] = vertices[c]; normals[Index] = normalVec4; texCoords[Index] = quadTexCoords[2]; Index++;
	colors[Index] = vertex_colors[d]; points[Index] = vertices[d]; normals[Index] = normalVec4; texCoords[Index] = quadTexCoords[3]; Index++;

}

//----------------------------------------------------------------------------

// generate 12 triangles: 36 vertices and 36 colors
void
colorcube()
{
	quad(1, 0, 3, 2);
	quad(2, 3, 7, 6);
	quad(3, 0, 4, 7);
	quad(6, 5, 1, 2);
	quad(4, 5, 6, 7);
	quad(5, 4, 0, 1);
}

//----------------------------------------------------------------------------

// OpenGL initialization
void
init()
{
	colorcube();

	// Create a vertex array object

	glGenVertexArrays(1, &CubeVao);
	glBindVertexArray(CubeVao);

	// Create and initialize a buffer object
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	//vertex 데이터 정의
	glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors) + sizeof(normals) + sizeof(texCoords),
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors), sizeof(normals), normals);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors) + sizeof(normals), sizeof(texCoords), texCoords);

	// Load shaders and use the resulting shader program
	GLuint program = InitShader("src/vshader.glsl", "src/fshader.glsl");
	glUseProgram(program);

	// set up vertex arrays
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(0));

	GLuint vColor = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(points)));

	GLuint vNormal = glGetAttribLocation(program, "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(points) + sizeof(colors)));

	GLuint vTexCoord = glGetAttribLocation(program, "vTexCoord");
	glEnableVertexAttribArray(vTexCoord);
	glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(points) + sizeof(colors) + sizeof(normals)));

	projectMatrixID = glGetUniformLocation(program, "mProject");
	projectMat = glm::perspective(glm::radians(65.0f), 1.0f, 0.1f, 100.0f);
	glUniformMatrix4fv(projectMatrixID, 1, GL_FALSE, &projectMat[0][0]);

	viewMatrixID = glGetUniformLocation(program, "mView");
	viewMat = glm::lookAt(glm::vec3(2, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &viewMat[0][0]);

	modelMatrixID = glGetUniformLocation(program, "mModel");
	glm::mat4 modelMat = glm::mat4(1.0f);
	glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMat[0][0]);

	pvmMatrixID = glGetUniformLocation(program, "mPVM");


	//구를 위한 CubeVao
	glGenVertexArrays(1, &sphereVao);
	glBindVertexArray(sphereVao);

	// 구의 버퍼 데이터 설정
	GLuint sphereBuffer;
	glGenBuffers(1, &sphereBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, sphereBuffer);

	int vertSize = sizeof(sphere.verts[0]) * sphere.verts.size();
	int normalSize = sizeof(sphere.normals[0]) * sphere.normals.size();
	int texSize = sizeof(sphere.texCoords[0]) * sphere.texCoords.size();

	glBufferData(GL_ARRAY_BUFFER, vertSize + normalSize + texSize, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertSize, sphere.verts.data());
	glBufferSubData(GL_ARRAY_BUFFER, vertSize, normalSize, sphere.normals.data());
	glBufferSubData(GL_ARRAY_BUFFER, vertSize + normalSize, texSize, sphere.texCoords.data());

	// 구의 셰이더 프로그램 설정
	GLuint sphereProgram = InitShader("src/vshader.glsl", "src/fshader.glsl");
	glUseProgram(sphereProgram);

	// 구의 텍스처 및 조명 설정
	GLuint sphereVPosition = glGetAttribLocation(sphereProgram, "vPosition");
	glEnableVertexAttribArray(sphereVPosition);
	glVertexAttribPointer(sphereVPosition, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(0));

	GLuint sphereVNormal = glGetAttribLocation(sphereProgram, "vNormal");
	glEnableVertexAttribArray(sphereVNormal);
	glVertexAttribPointer(sphereVNormal, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(vertSize));

	GLuint sphereVTexCoord = glGetAttribLocation(sphereProgram, "vTexCoord");
	glEnableVertexAttribArray(sphereVTexCoord);
	glVertexAttribPointer(sphereVTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(vertSize + normalSize));

	GLuint sphereProjectMatrixID = glGetUniformLocation(sphereProgram, "mProject");
	glUniformMatrix4fv(sphereProjectMatrixID, 1, GL_FALSE, &projectMat[0][0]);

	GLuint sphereViewMatrixID = glGetUniformLocation(sphereProgram, "mView");
	glUniformMatrix4fv(sphereViewMatrixID, 1, GL_FALSE, &viewMat[0][0]);

	GLuint sphereModelMatrixID = glGetUniformLocation(sphereProgram, "mModel");
	glUniformMatrix4fv(sphereModelMatrixID, 1, GL_FALSE, &modelMat[0][0]);

	GLuint spherePvmMatrixID = glGetUniformLocation(sphereProgram, "mPVM");


	// Load the texture using any two methods
	waterTexture = loadBMP_custom("water.bmp");
	earthTexture = loadBMP_custom("earth.bmp");
	//GLuint waterTexture = loadDDS("uvtemplate.DDS");

	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID = glGetUniformLocation(program, "texture");

	// Bind our texture in waterTexture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, waterTexture);

	// Set our "myTextureSampler" sampler to use waterTexture Unit 0
	glUniform1i(TextureID, 0);


	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0, 0.0, 1.0);
}

// 제자리에서 달리는 인간을 그리는 함수
void drawHuman(glm::mat4 humanMat)
{
    glm::mat4 bodyMat, pvmMat;
    glm::vec3 humanPos[10];

    // 인간의 몸을 10부위로 나눠서 각각의 상대 위치 정의
    humanPos[0] = glm::vec3(0.0, bodyPostionMove, 0.0); // 몸
    humanPos[1] = glm::vec3(0.0, 0.8, 0.0); // 머리
    humanPos[2] = glm::vec3(-0.65, 0.2, 0.0); // 왼쪽 위쪽 팔
    humanPos[3] = glm::vec3(0.0, -0.875, 0.0); // 왼쪽 아래쪽 팔
    humanPos[4] = glm::vec3(0.65, 0.2, 0.0); // 오른쪽 위쪽 팔
    humanPos[5] = glm::vec3(0.0, -0.875, 0.0); // 오른쪽 아래쪽 팔
    humanPos[6] = glm::vec3(-0.2, -0.5, 0.0); // 왼쪽 위쪽 다리
    humanPos[7] = glm::vec3(0.0, -1, 0.0); // 왼쪽 아래쪽 다리
    humanPos[8] = glm::vec3(0.2, -0.5, 0.0); // 오른쪽 위쪽 다리
    humanPos[9] = glm::vec3(0.0, -1, 0.0); // 오른쪽 아래쪽 다리

    // 몸
	glBindTexture(GL_TEXTURE_2D, waterTexture);
    bodyMat = glm::translate(humanMat, humanPos[0]);
    bodyMat = glm::scale(bodyMat, glm::vec3(0.4, 0.6, 0.3));
    pvmMat = projectMat * viewMat * bodyMat;
    glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &bodyMat[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);

    // 머리

	glBindVertexArray(sphereVao);
	glBindTexture(GL_TEXTURE_2D, earthTexture);
    glm::mat4 headMat = glm::translate(bodyMat, humanPos[1]);
    headMat = glm::scale(headMat, glm::vec3(0.5, 0.5, 0.5));
    pvmMat = projectMat * viewMat * headMat;
    glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &headMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, sphere.verts.size());

    // 왼쪽 팔 위쪽
	glBindTexture(GL_TEXTURE_2D, waterTexture);
	glBindVertexArray(CubeVao);
    glm::mat4 leftUpperArmMat = glm::translate(bodyMat, humanPos[2]);
    leftUpperArmMat = glm::rotate(leftUpperArmMat, glm::clamp(-glm::sin(rotAngle) , glm::radians(-70.0f), glm::radians(70.0f)), glm::vec3(1, 0, 0));
    leftUpperArmMat = glm::scale(leftUpperArmMat, glm::vec3(0.25, 0.33, 0.33));
    pvmMat = projectMat * viewMat * leftUpperArmMat;
    glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &leftUpperArmMat[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);

    // 왼쪽 팔 아래쪽 
    glm::mat4 leftLowerArmMat = glm::translate(leftUpperArmMat, humanPos[3]);
    leftLowerArmMat = glm::rotate(leftLowerArmMat, leftLowerArmAngle, glm::vec3(1, 0, 0));
    leftLowerArmMat = glm::scale(leftLowerArmMat, glm::vec3(1, 0.75, 1));
    pvmMat = projectMat * viewMat * leftLowerArmMat;
    glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &leftLowerArmMat[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);

    // 오른쪽 팔 위쪽
    glm::mat4 rightUpperArmMat = glm::translate(bodyMat, humanPos[4]);
    rightUpperArmMat = glm::rotate(rightUpperArmMat, glm::clamp(glm::sin(rotAngle), glm::radians(-70.0f), glm::radians(70.0f)), glm::vec3(1, 0, 0));
    rightUpperArmMat = glm::scale(rightUpperArmMat, glm::vec3(0.25, 0.33, 0.33));
    pvmMat = projectMat * viewMat * rightUpperArmMat;
    glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &rightUpperArmMat[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);

    // 오른쪽 팔 아래쪽 
    glm::mat4 rightLowerArmMat = glm::translate(rightUpperArmMat, humanPos[5]);
    rightLowerArmMat = glm::rotate(rightLowerArmMat, rightLowerArmAngle, glm::vec3(1, 0, 0));
    rightLowerArmMat = glm::scale(rightLowerArmMat, glm::vec3(1, 0.75, 1));
    pvmMat = projectMat * viewMat * rightLowerArmMat;
    glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &rightLowerArmMat[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);

    // 왼쪽 다리 위쪽
    glm::mat4 leftUpperLegMat = glm::translate(bodyMat, humanPos[6]);
    leftUpperLegMat = glm::rotate(leftUpperLegMat, glm::clamp(glm::sin(rotAngle), glm::radians(-70.0f), glm::radians(70.0f)), glm::vec3(1, 0, 0));
    leftUpperLegMat = glm::scale(leftUpperLegMat, glm::vec3(0.25, 0.5, 0.3));
    pvmMat = projectMat * viewMat * leftUpperLegMat;
    glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &leftUpperLegMat[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);

    // 왼쪽 다리 아래쪽 
    glm::mat4 leftLowerLegMat = glm::translate(leftUpperLegMat, humanPos[7]);
    leftLowerLegMat = glm::rotate(leftLowerLegMat, leftLowerLegAngle, glm::vec3(1, 0, 0));
    leftLowerLegMat = glm::scale(leftLowerLegMat, glm::vec3(1, 1, 1));
    pvmMat = projectMat * viewMat * leftLowerLegMat;
    glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &leftLowerLegMat[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);

    // 오른쪽 다리 위쪽
    glm::mat4 rightUpperLegMat = glm::translate(bodyMat, humanPos[8]);
    rightUpperLegMat = glm::rotate(rightUpperLegMat, glm::clamp(-glm::sin(rotAngle), glm::radians(-70.0f), glm::radians(70.0f)), glm::vec3(1, 0, 0));
    rightUpperLegMat = glm::scale(rightUpperLegMat, glm::vec3(0.25, 0.5, 0.3));
    pvmMat = projectMat * viewMat * rightUpperLegMat;
    glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &rightUpperLegMat[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);

    // 오른쪽 다리 아래쪽 
    glm::mat4 rightLowerLegMat = glm::translate(rightUpperLegMat, humanPos[9]);
    rightLowerLegMat = glm::rotate(rightLowerLegMat, rightLowerLegAngle, glm::vec3(1, 0, 0));
    rightLowerLegMat = glm::scale(rightLowerLegMat, glm::vec3(1, 1, 1));
    pvmMat = projectMat * viewMat * rightLowerLegMat;
    glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &rightLowerLegMat[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);
}

// 제자리에서 달리는 인간 display
void display(void)
{
	glm::mat4 worldMat, pvmMat;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	worldMat = glm::mat4(1.0f);

	drawHuman(worldMat);

	glutSwapBuffers();
}

void idle()
{
	static int prevTime = glutGet(GLUT_ELAPSED_TIME);
	int currTime = glutGet(GLUT_ELAPSED_TIME);

	if (abs(currTime - prevTime) >= 20)
	{
		float t = abs(currTime - prevTime);
		float angleIncrement = glm::radians(t * 360.0f / 1000.0f); // 라디안 단위로 변경

		// 윗팔의 회전 각도 업데이트
		if (animationMode % 8 < 2 || animationMode % 8 > 5) {
			rotAngle += angleIncrement;
			if (rotAngle >= KeyFrameAngle[(animationMode + 1) % 8]) {
				rotAngle = KeyFrameAngle[(animationMode + 1) % 8];
				animationMode += 1;
			}
		}
		else {
			rotAngle -= angleIncrement;
			if (rotAngle <= KeyFrameAngle[(animationMode + 1) % 8]) {
				rotAngle = KeyFrameAngle[(animationMode + 1) % 8];
				animationMode += 1;
			}
		}
		animationMode = animationMode % 8;

		//printf("%d", animationMode); //디버깅용

		// KeyFrame 계산
		int keyFrame = animationMode;
		float keyFrameProgress = (rotAngle - KeyFrameAngle[keyFrame]) / (KeyFrameAngle[(keyFrame + 1) % 8] - KeyFrameAngle[keyFrame]);

		// 선형 보간
		leftLowerArmAngle = glm::mix(Angle[0][keyFrame], Angle[0][(keyFrame + 1) % 8], keyFrameProgress); // glm::mix는 선형보간에 사용하는 라이브러리 함수
		rightLowerArmAngle = glm::mix(Angle[1][keyFrame], Angle[1][(keyFrame + 1) % 8], keyFrameProgress);
		leftLowerLegAngle = glm::mix(Angle[2][keyFrame], Angle[2][(keyFrame + 1) % 8], keyFrameProgress);
		rightLowerLegAngle = glm::mix(Angle[3][keyFrame], Angle[3][(keyFrame + 1) % 8], keyFrameProgress);

		//body 움직임 애니메이션
		if (bodyMoveDirection == 0) { // 위로 움직이는 중
			bodyPostionMove += t / 1300.0f;
			if (bodyPostionMove > MaxbodyPosition) {
				bodyPostionMove = bodyPostionMove - 2 * (bodyPostionMove - MaxbodyPosition);
				bodyMoveDirection = 1;
			}
		}
		else { // 아래로 움직이는 중
			bodyPostionMove -= t / 1300.0f;
			if (bodyPostionMove < LowBodyPosition) {
				bodyPostionMove = bodyPostionMove + 2 * (LowBodyPosition - bodyPostionMove);
				bodyMoveDirection = 0;
			}
		}

		prevTime = currTime;
		glutPostRedisplay();
	}
}
//----------------------------------------------------------------------------

//keyBoard 숫자 입력에 따라 시점 방향 변경
void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 033:  // Escape key
	case 'q': case 'Q':
		exit(EXIT_SUCCESS);
		break;
	case '1':
		cameraMode = 1; //side 
		viewMat = glm::lookAt(glm::vec3(2, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		break;
	case '2':
		cameraMode = 2; //over-the-shoulder 
		viewMat = glm::lookAt(glm::vec3(0, 1.5, -1.5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		break;
	case '3':
		cameraMode = 3; //front
		viewMat = glm::lookAt(glm::vec3(0.5, 0.5, 2), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		break;
	}
	glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &viewMat[0][0]);
}


void resize(int w, int h)
{
	float ratio = (float)w / (float)h;
	glViewport(0, 0, w, h);

	projectMat = glm::perspective(glm::radians(65.0f), ratio, 0.1f, 100.0f);
	glUniformMatrix4fv(projectMatrixID, 1, GL_FALSE, &projectMat[0][0]);
	glutPostRedisplay();
}

//----------------------------------------------------------------------------

int
main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(512, 512);
	glutInitContextVersion(3, 2);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow("Running Human");

	glewInit();

	init();

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(resize);
	glutIdleFunc(idle);

	glutMainLoop();
	return 0;
}
