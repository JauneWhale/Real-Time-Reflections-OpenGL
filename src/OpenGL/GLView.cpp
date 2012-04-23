#include "GLView.h"

GLView::GLView(){}
GLView::~GLView(){}

//CameraToClip
void GLView::calcCameraToClipMatrix(float viewAngle, float frustumNear, float frustumFar)
{
	//Set variables
	this->viewAngle = viewAngle;
	this->frustumNear = frustumNear;
	this->frustumFar = frustumFar;
	this->cameraToClipMatrix = glm::mat4(0.0f);

	//Calc frustum scale
	const float degToRad = 3.14159f / 180.0f;
	float viewAngleRad = this->viewAngle * degToRad;
	this->frustumScale = 1.0f / tan(viewAngleRad / 2.0f);

	//Create matrix
	this->cameraToClipMatrix[0].x = frustumScale;
	this->cameraToClipMatrix[1].y = frustumScale;
	this->cameraToClipMatrix[2].z = (this->frustumFar + this->frustumNear)/(this->frustumNear - this->frustumFar);
	this->cameraToClipMatrix[2].w = -1.0f;
	this->cameraToClipMatrix[3].z = (2.0f*this->frustumFar*this->frustumNear)/(this->frustumNear - this->frustumFar);
}
glm::mat4 GLView::getCameraToClipMatrix()
{
	return this->cameraToClipMatrix;
}

//Other
void GLView::setWindowDimensions(int width, int height)
{
	glViewport(0, 0, (GLsizei) width, (GLsizei) height);
	this->cameraToClipMatrix[0].x = frustumScale * (height/ (float)width);
	this->cameraToClipMatrix[1].y = frustumScale;
}
glm::ivec2 GLView::getWindowDimensions()
{
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT,viewport);
	return glm::ivec2((int)viewport[2],(int)viewport[3]);
}
Ray GLView::getPickingRay(int x, int y)
{
	GLState* glState = Singleton<GLState>::Instance();
	glm::mat4 worldToCameraMatrix = glState->worldToCameraMatrix;

	glm::ivec2 windowDimensions = GLView::getWindowDimensions();
	float winWidth = (float)windowDimensions.x;
	float winHeight = (float)windowDimensions.y;
	float winX = (float)x;
	float winY = (float)y;
	float winZClose = 0.0f;
	float winZFar = 1.0f;

	//Window to NDC
	glm::vec4 closePoint;
	closePoint.x = 2.0f*(winX/winWidth) - 1.0f;
	closePoint.y = 2.0f*((winHeight - winY)/winHeight) - 1.0f;
	closePoint.z = 2.0f*(winZClose) - 1.0f;
	closePoint.w = 1.0f;

	glm::vec4 farPoint;
	farPoint.x = 2.0f*(winX/winWidth) - 1.0f;
	farPoint.y = 2.0f*((winHeight - winY)/winHeight) - 1.0f;
	farPoint.z = 2.0f*(winZFar) - 1.0f;
	farPoint.w = 1.0f;

	//NDC to clip
	closePoint *= this->frustumNear;
	farPoint *= this->frustumFar;
	glm::mat4 clipToCamera = glm::inverse(this->cameraToClipMatrix);

	closePoint = clipToCamera * closePoint;
	farPoint = clipToCamera * farPoint;

	glm::mat4 cameraToWorld = glm::inverse(worldToCameraMatrix);
	closePoint = cameraToWorld * closePoint;
	farPoint = cameraToWorld * farPoint;

	Ray r = Ray(glm::vec3(closePoint), glm::normalize(glm::vec3(farPoint-closePoint)));
	return r;
}
float GLView::getViewAngle()
{
	return this->viewAngle;
}
float GLView::getFrustumNear()
{
	return this->frustumNear;
}
float GLView::getFrustumFar()
{
	return this->frustumFar;
}