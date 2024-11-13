//
// Display a color cube
//
// Colors are assigned to each vertex and then the rasterizer interpolates
//   those colors across the triangles.  We us an orthographic projection
//   as the default projetion.

#include "cube.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"

glm::mat4 projectMat;
glm::mat4 viewMat;

GLuint pvmMatrixID;

// ���� ���� �������� ����
int animationMode = 0; // ���° KeyFrame�� �������� ǥ��
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
}; // KeyFrame ������ �Ǵ� ������ ����

float Angle[4][8] = {
	{0.0f, glm::radians(30.0f), glm::radians(90.0f), glm::radians(30.0f), 0.0f, glm::radians(-20.0f), glm::radians(-70.0f), glm::radians(-20.0f)}, // ���� �� �Ʒ� ����
	{0.0f, glm::radians(-20.0f), glm::radians(-70.0f), glm::radians(-20.0f), 0.0f, glm::radians(30.0f), glm::radians(90.0f), glm::radians(30.0f)}, // ������ �� �Ʒ� ����
	{0.0f, glm::radians(70.0f), glm::radians(120.0f), glm::radians(70.0f), 0.0f, glm::radians(-30.0f), glm::radians(0.0f), glm::radians(-30.0f)}, // ���� �ٸ� �Ʒ� ����
	{0.0f, glm::radians(-30.0f), glm::radians(-0.0f), glm::radians(-30.0f), 0.0f, glm::radians(70.0f), glm::radians(120.0f), glm::radians(70.0f)}  // ������ �ٸ� �Ʒ� ����
};

// �� ������ ���� �������� ����
float bodyPostionMove = 0.0f; // ���� Z�� ���� ������ ��ġ
int bodyMoveDirection = 0; // 0�̸� �������� �̵� ��, 1�̸� �Ʒ������� �̵���
float MaxbodyPosition = 0.25; // �ִ� z��
float LowBodyPosition = 0.0; // �ּ� z��

int isDrawingCar = false;
int cameraMode = 1; //ī�޶� ��� ����

typedef glm::vec4  color4;
typedef glm::vec4  point4;

const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

point4 points[NumVertices];
color4 colors[NumVertices];

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
	colors[Index] = vertex_colors[a]; points[Index] = vertices[a];  Index++;
	colors[Index] = vertex_colors[b]; points[Index] = vertices[b];  Index++;
	colors[Index] = vertex_colors[c]; points[Index] = vertices[c];  Index++;
	colors[Index] = vertex_colors[a]; points[Index] = vertices[a];  Index++;
	colors[Index] = vertex_colors[c]; points[Index] = vertices[c];  Index++;
	colors[Index] = vertex_colors[d]; points[Index] = vertices[d];  Index++;
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
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create and initialize a buffer object
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors),
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors);

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

	pvmMatrixID = glGetUniformLocation(program, "mPVM");

	projectMat = glm::perspective(glm::radians(65.0f), 1.0f, 0.1f, 100.0f);
	viewMat = glm::lookAt(glm::vec3(2, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0, 0.0, 1.0);
}

// ���ڸ����� �޸��� �ΰ��� �׸��� �Լ�
void drawHuman(glm::mat4 humanMat)
{
    glm::mat4 bodyMat, pvmMat;
    glm::vec3 humanPos[10];

    // �ΰ��� ���� 10������ ������ ������ ��� ��ġ ����
    humanPos[0] = glm::vec3(0.0, bodyPostionMove, 0.0); // ��
    humanPos[1] = glm::vec3(0.0, 0.625, 0.0); // �Ӹ�
    humanPos[2] = glm::vec3(-0.65, 0.2, 0.0); // ���� ���� ��
    humanPos[3] = glm::vec3(0.0, -0.875, 0.0); // ���� �Ʒ��� ��
    humanPos[4] = glm::vec3(0.65, 0.2, 0.0); // ������ ���� ��
    humanPos[5] = glm::vec3(0.0, -0.875, 0.0); // ������ �Ʒ��� ��
    humanPos[6] = glm::vec3(-0.2, -0.5, 0.0); // ���� ���� �ٸ�
    humanPos[7] = glm::vec3(0.0, -1, 0.0); // ���� �Ʒ��� �ٸ�
    humanPos[8] = glm::vec3(0.2, -0.5, 0.0); // ������ ���� �ٸ�
    humanPos[9] = glm::vec3(0.0, -1, 0.0); // ������ �Ʒ��� �ٸ�

    // ��
    bodyMat = glm::translate(humanMat, humanPos[0]);
    bodyMat = glm::scale(bodyMat, glm::vec3(0.4, 0.6, 0.3));
    pvmMat = projectMat * viewMat * bodyMat;
    glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);

    // �Ӹ�
    glm::mat4 headMat = glm::translate(bodyMat, humanPos[1]);
    headMat = glm::scale(headMat, glm::vec3(0.5, 0.5, 0.5));
    pvmMat = projectMat * viewMat * headMat;
    glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);

    // ���� �� ����
    glm::mat4 leftUpperArmMat = glm::translate(bodyMat, humanPos[2]);
    leftUpperArmMat = glm::rotate(leftUpperArmMat, glm::clamp(-glm::sin(rotAngle) , glm::radians(-70.0f), glm::radians(70.0f)), glm::vec3(1, 0, 0));
    leftUpperArmMat = glm::scale(leftUpperArmMat, glm::vec3(0.25, 0.33, 0.33));
    pvmMat = projectMat * viewMat * leftUpperArmMat;
    glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);

    // ���� �� �Ʒ��� 
    glm::mat4 leftLowerArmMat = glm::translate(leftUpperArmMat, humanPos[3]);
    leftLowerArmMat = glm::rotate(leftLowerArmMat, leftLowerArmAngle, glm::vec3(1, 0, 0));
    leftLowerArmMat = glm::scale(leftLowerArmMat, glm::vec3(1, 0.75, 1));
    pvmMat = projectMat * viewMat * leftLowerArmMat;
    glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);

    // ������ �� ����
    glm::mat4 rightUpperArmMat = glm::translate(bodyMat, humanPos[4]);
    rightUpperArmMat = glm::rotate(rightUpperArmMat, glm::clamp(glm::sin(rotAngle), glm::radians(-70.0f), glm::radians(70.0f)), glm::vec3(1, 0, 0));
    rightUpperArmMat = glm::scale(rightUpperArmMat, glm::vec3(0.25, 0.33, 0.33));
    pvmMat = projectMat * viewMat * rightUpperArmMat;
    glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);

    // ������ �� �Ʒ��� 
    glm::mat4 rightLowerArmMat = glm::translate(rightUpperArmMat, humanPos[5]);
    rightLowerArmMat = glm::rotate(rightLowerArmMat, rightLowerArmAngle, glm::vec3(1, 0, 0));
    rightLowerArmMat = glm::scale(rightLowerArmMat, glm::vec3(1, 0.75, 1));
    pvmMat = projectMat * viewMat * rightLowerArmMat;
    glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);

    // ���� �ٸ� ����
    glm::mat4 leftUpperLegMat = glm::translate(bodyMat, humanPos[6]);
    leftUpperLegMat = glm::rotate(leftUpperLegMat, glm::clamp(glm::sin(rotAngle), glm::radians(-70.0f), glm::radians(70.0f)), glm::vec3(1, 0, 0));
    leftUpperLegMat = glm::scale(leftUpperLegMat, glm::vec3(0.25, 0.5, 0.3));
    pvmMat = projectMat * viewMat * leftUpperLegMat;
    glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);

    // ���� �ٸ� �Ʒ��� 
    glm::mat4 leftLowerLegMat = glm::translate(leftUpperLegMat, humanPos[7]);
    leftLowerLegMat = glm::rotate(leftLowerLegMat, leftLowerLegAngle, glm::vec3(1, 0, 0));
    leftLowerLegMat = glm::scale(leftLowerLegMat, glm::vec3(1, 1, 1));
    pvmMat = projectMat * viewMat * leftLowerLegMat;
    glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);

    // ������ �ٸ� ����
    glm::mat4 rightUpperLegMat = glm::translate(bodyMat, humanPos[8]);
    rightUpperLegMat = glm::rotate(rightUpperLegMat, glm::clamp(-glm::sin(rotAngle), glm::radians(-70.0f), glm::radians(70.0f)), glm::vec3(1, 0, 0));
    rightUpperLegMat = glm::scale(rightUpperLegMat, glm::vec3(0.25, 0.5, 0.3));
    pvmMat = projectMat * viewMat * rightUpperLegMat;
    glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);

    // ������ �ٸ� �Ʒ��� 
    glm::mat4 rightLowerLegMat = glm::translate(rightUpperLegMat, humanPos[9]);
    rightLowerLegMat = glm::rotate(rightLowerLegMat, rightLowerLegAngle, glm::vec3(1, 0, 0));
    rightLowerLegMat = glm::scale(rightLowerLegMat, glm::vec3(1, 1, 1));
    pvmMat = projectMat * viewMat * rightLowerLegMat;
    glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);
}

// ���ڸ����� �޸��� �ΰ� display
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
		float angleIncrement = glm::radians(t * 360.0f / 1000.0f); // ���� ������ ����

		// ������ ȸ�� ���� ������Ʈ
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

		//printf("%d", animationMode); //������

		// KeyFrame ���
		int keyFrame = animationMode;
		float keyFrameProgress = (rotAngle - KeyFrameAngle[keyFrame]) / (KeyFrameAngle[(keyFrame + 1) % 8] - KeyFrameAngle[keyFrame]);

		// ���� ����
		leftLowerArmAngle = glm::mix(Angle[0][keyFrame], Angle[0][(keyFrame + 1) % 8], keyFrameProgress); // glm::mix�� ���������� ����ϴ� ���̺귯�� �Լ�
		rightLowerArmAngle = glm::mix(Angle[1][keyFrame], Angle[1][(keyFrame + 1) % 8], keyFrameProgress);
		leftLowerLegAngle = glm::mix(Angle[2][keyFrame], Angle[2][(keyFrame + 1) % 8], keyFrameProgress);
		rightLowerLegAngle = glm::mix(Angle[3][keyFrame], Angle[3][(keyFrame + 1) % 8], keyFrameProgress);

		//body ������ �ִϸ��̼�
		if (bodyMoveDirection == 0) { // ���� �����̴� ��
			bodyPostionMove += t / 1300.0f;
			if (bodyPostionMove > MaxbodyPosition) {
				bodyPostionMove = bodyPostionMove - 2 * (bodyPostionMove - MaxbodyPosition);
				bodyMoveDirection = 1;
			}
		}
		else { // �Ʒ��� �����̴� ��
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

//keyBoard ���� �Է¿� ���� ���� ���� ����
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
}


void resize(int w, int h)
{
	float ratio = (float)w / (float)h;
	glViewport(0, 0, w, h);

	projectMat = glm::perspective(glm::radians(65.0f), ratio, 0.1f, 100.0f);

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
