#include "GLDisplay.h"

GLDisplay::GLDisplay(){}
GLDisplay::~GLDisplay(){}

void GLDisplay::initialize()
{
	//Initializers
	this->initializeGL();
	this->initializeCamera();
	this->initializeFramebuffers();

	//Event handlers
	Singleton<EventHandler>::Instance()->addEnterFrameEventListener(EnterFrameReceiver::from_method<GLDisplay,&GLDisplay::update>(this));
	Singleton<EventHandler>::Instance()->addEnterFrameEventListener(EnterFrameReceiver::from_method<GLDisplay,&GLDisplay::checkKeyPress>(this));
	Singleton<EventHandler>::Instance()->addInputEventListener(sf::Event::Resized,InputReceiver::from_method<GLDisplay,&GLDisplay::resize>(this));
	Singleton<EventHandler>::Instance()->addInputEventListener(sf::Event::MouseButtonPressed,InputReceiver::from_method<GLDisplay,&GLDisplay::mouseButtonPressed>(this));
	Singleton<EventHandler>::Instance()->addInputEventListener(sf::Event::MouseMoved,InputReceiver::from_method<GLDisplay,&GLDisplay::mouseMoved>(this));
	Singleton<EventHandler>::Instance()->addInputEventListener(sf::Event::MouseWheelMoved,InputReceiver::from_method<GLDisplay,&GLDisplay::mouseWheelMoved>(this));
}
void GLDisplay::initializeGL()
{
	gl3wInit();
	Singleton<GLUniformBlockHelper>::Instance()->initialize();

    //Set GL global vars
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRange(0.0f, 1.0f);
    //glPointSize(10);
	//glLineWidth(2);
	//glEnable (GL_BLEND); 
	//glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//glPolygonOffset(10.0f,10.0f);
}
void GLDisplay::initializeCamera()
{
	//Camera to Clip
	float fov = 45.0f;
	float nearPlane = 0.1f;
	float farPlane = 100.0f;
	Singleton<GLCamera>::Instance()->calcCameraToClipMatrix(fov,nearPlane,farPlane);
}
void GLDisplay::initializeFramebuffers()
{
	this->reflectionBufferFront = new GLFramebuffer_Reflection();
	this->reflectionBufferFront->initialize();

	this->reflectionBufferBack = new GLFramebuffer_Reflection();
	this->reflectionBufferBack->initialize();

	this->shadowMapBuffer = new GLFramebuffer_ShadowMap();
	this->shadowMapBuffer->initialize();
}
void GLDisplay::initializePhysics()
{
	//Singleton<PhysicsSceneDefault>::Instance()->makeDefaultScene(this->world);
	//this->physicsWorld = Singleton<PhysicsSceneDefault>::Instance()->getScene();
	//Singleton<PhysicsIO>::Instance()->initialize();
}
void GLDisplay::update()
{ 
	if(this->world != 0)
	{
		GLState* glState = Singleton<GLState>::Instance();
		GLCamera* glCamera = Singleton<GLCamera>::Instance();
		GLUniformBlockHelper* uniformBlockHelper = Singleton<GLUniformBlockHelper>::Instance();

		//Texture stuff
		glState->setReflectionTextures(0,1,2,3);
		glState->depthTextureShadow = 4;
		int textureGroup0 = GLFramebuffer_Reflection::TEXTURE_GROUP0;
		int textureGroup1 = GLFramebuffer_Reflection::TEXTURE_GROUP1;
		
		//Update everything
		//physicsWorld->update();
		world->update();
		glm::mat4 worldToCameraMatrix = this->camera->getWorldToCameraMatrix();
		//glm::mat4 worldToCameraMatrix = ((ShadowLight*)(this->world->getObjectsByType("ShadowLight").at(0)))->lightCamera->getWorldToCameraMatrix();
		glCamera->setWorldToCameraMatrix(worldToCameraMatrix);
		uniformBlockHelper->updateAll();

		//Diffuse render to texture (front and back faces)
		glState->effectType = GLUniformBlockHelper::DIFFUSE;
		uniformBlockHelper->update(GLUniformBlockHelper::TYPE_EFFECT_TYPE);
		//Front face diffuse render
		this->reflectionBufferFront->bindForWriting(textureGroup0);
		glCullFace(GL_BACK);
		this->clearGL();
		world->render();
		//this->reflectionBufferFront->bindForReading(GL_TEXTURE0,GL_TEXTURE1,textureGroup0);
		//Back face diffuse render
		//this->reflectionBufferBack->bindForWriting(textureGroup1);
		//glCullFace(GL_FRONT);
		//this->clearGL();
		//world->render();

		//Reflections render to texture (front faces)
		glState->effectType = GLUniformBlockHelper::REFLECTION;
		uniformBlockHelper->update(GLUniformBlockHelper::TYPE_EFFECT_TYPE);
		glCullFace(GL_BACK);
		this->reflectionBufferFront->bindForReadingAndWriting(GL_TEXTURE0,GL_TEXTURE1,textureGroup0,textureGroup1);
		//this->reflectionBufferBack->bindForReadingAndWriting(GL_TEXTURE2,GL_TEXTURE3,textureGroup0,textureGroup1);
		//this->reflectionBufferFront->bindForReading(GL_TEXTURE0,GL_TEXTURE1,textureGroup1);
		//this->reflectionBufferBack->bindForReading(GL_TEXTURE2,GL_TEXTURE3,textureGroup1);
		this->clearGL();
		world->render();

		//Refractions render to texture (front faces)
		glState->effectType = GLUniformBlockHelper::REFRACTION;
		uniformBlockHelper->update(GLUniformBlockHelper::TYPE_EFFECT_TYPE);
		glCullFace(GL_BACK);
		this->reflectionBufferFront->bindForReadingAndWriting(GL_TEXTURE0,GL_TEXTURE1,textureGroup1,textureGroup0);
		//this->reflectionBufferFront->bindForReading(GL_TEXTURE0,GL_TEXTURE1,textureGroup1);
		//this->reflectionBufferBack->bindForReading(GL_TEXTURE2,GL_TEXTURE3,textureGroup1);
		this->clearGL();
		world->render();
		
		this->reflectionBufferFront->bindForReading(GL_TEXTURE0,GL_TEXTURE1,textureGroup0);
		//this->clearGL();
		//world->render();
		
		//Shadow map from light
		glState->effectType = GLUniformBlockHelper::SHADOW_BEGIN;
		uniformBlockHelper->update(GLUniformBlockHelper::TYPE_EFFECT_TYPE);
		glCullFace(GL_BACK);
		this->shadowMapBuffer->bindForWriting();
		this->clearGL();
		world->render();

		//Final
		glState->effectType = GLUniformBlockHelper::SHADOW_END;
		uniformBlockHelper->update(GLUniformBlockHelper::TYPE_EFFECT_TYPE);
		glCullFace(GL_BACK);
		this->shadowMapBuffer->bindForReading(GL_TEXTURE4);
		this->clearGL();
		world->render();
	}
}
void GLDisplay::clearGL()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void GLDisplay::resize(sf::Event sfEvent)
{
	int width = sfEvent.Size.Width;
	int height = sfEvent.Size.Height;
	this->resize(width,height);
}

//IO Events
void GLDisplay::resize(int width, int height)
{
	Singleton<GLCamera>::Instance()->setWindowDimensions(width,height);
}
void GLDisplay::mouseButtonPressed(sf::Event sfEvent)
{
	EventHandler* eventHandler = Singleton<EventHandler>::Instance();
	glm::ivec2 mousePos = eventHandler->getMousePos();
	if(sfEvent.MouseButton.Button == sf::Mouse::Left)
	{
		if(!eventHandler->isAltDown())
		{
			RenderObject* oldSelectedObject = this->selectedObject;
			this->selectedObject = 0;
			float closestDistance = FLT_MAX;

			Ray clickRay = Singleton<GLCamera>::Instance()->getPickingRay(mousePos.x,mousePos.y);
			std::vector<Object*> renderObjects = this->world->getObjectsByType("RenderObject");
			for(unsigned int i = 0; i < renderObjects.size(); i++)
			{
				RenderObject* renderObject = (RenderObject*)renderObjects.at(i);
				glm::mat4 transformationMatrix = renderObject->getTransformationMatrix();
				BoundingBox* boundingBox = renderObject->getMesh()->getGLMeshData()->boundingBox;
				IntersectionData intersectionData = IntersectionAlgorithms::RayBoxIntersect(clickRay,transformationMatrix, boundingBox);
				if(intersectionData.valid && intersectionData.distanceAlongRay < closestDistance)
				{
					closestDistance = intersectionData.distanceAlongRay;
					this->selectedObject = renderObject;	
				}
			}
			if(selectedObject != 0)
				this->selectedObject->getMaterial()->diffuseColor += 1.0f;
			if(oldSelectedObject != 0)
				oldSelectedObject->getMaterial()->diffuseColor -= 1.0f;
		}
	}
}
void GLDisplay::mouseMoved(sf::Event sfEvent)
{
	EventHandler* eventHandler = Singleton<EventHandler>::Instance();
	glm::ivec2 mousePos = eventHandler->getMousePos();
	glm::ivec2 prevMousePos = eventHandler->getPrevMousePos();
	int mouseXDiff = (mousePos.x - prevMousePos.x);
	int mouseYDiff = (mousePos.y - prevMousePos.y);

	bool altIsDown = eventHandler->isAltDown();
	if(eventHandler->isLeftMouseDown() && altIsDown)
	{	
		float scaleFactor = .008f;
		float mouseXDifference = -(float)mouseXDiff * scaleFactor;
		float mouseYDifference = (float)mouseYDiff * scaleFactor;
		this->camera->rotateRad(mouseXDifference,mouseYDifference);
	}
	else if(eventHandler->isMiddleMouseDown() && altIsDown)
	{
		float scaleFactor = .01f;
		float mouseXDifference = -(float)mouseXDiff * scaleFactor;
		float mouseYDifference = (float)mouseYDiff * scaleFactor;
		this->camera->pan(mouseXDifference,mouseYDifference);
	}
	else if(eventHandler->isRightMouseDown() && altIsDown)
	{
		float scaleFactor = .05f;
		float mouseYDifference = -(float)mouseYDiff * scaleFactor;
		this->camera->zoom(mouseYDifference);
	}
}
void GLDisplay::mouseWheelMoved(sf::Event sfEvent)
{
	int delta = sfEvent.MouseWheel.Delta;
	float scaleFactor = 1.0f;
	this->camera->zoom(delta*scaleFactor);
}
void GLDisplay::checkKeyPress()
{
	EventHandler* eventHandler = Singleton<EventHandler>::Instance();
	if(this->selectedObject != 0)
	{
		float translationAmount = .03f;
		glm::vec3 camUp = glm::normalize(this->camera->getUpDir());
		glm::vec3 camLook = glm::normalize(this->camera->getLookDir());
		glm::vec3 camRight = glm::normalize(glm::cross(camLook,camUp));
		glm::vec3 moveX = /*glm::vec3(translationAmount,0,0);*/translationAmount*camRight;
		glm::vec3 moveY = /*glm::vec3(0,translationAmount,0);*/translationAmount*camUp;
		glm::vec3 moveZ = /*glm::vec3(0,0,translationAmount);*/translationAmount*camLook;

		if(eventHandler->isKeyDown(sf::Keyboard::A))
			selectedObject->translate(-moveX);
		if(eventHandler->isKeyDown(sf::Keyboard::D))
			selectedObject->translate(moveX);
		if(eventHandler->isKeyDown(sf::Keyboard::W))
			selectedObject->translate(moveY);
		if(eventHandler->isKeyDown(sf::Keyboard::S))
			selectedObject->translate(-moveY);
		if(eventHandler->isKeyDown(sf::Keyboard::Q))
			selectedObject->translate(moveZ);
		if(eventHandler->isKeyDown(sf::Keyboard::E))
			selectedObject->translate(-moveZ);
	}
}

//World
void GLDisplay::setWorld(World* world)
{
	this->world = world;
	this->camera = (Camera3rdPerson*)this->world->getObjectsByType("Camera3rdPerson",true).at(0);
	this->selectedObject = 0;
}
World* GLDisplay::getWorld()
{
	return this->world;
}

//Camera
void GLDisplay::setCamera(Camera* camera)
{
	this->camera = camera;
}
Camera* GLDisplay::getCamera()
{
	return this->camera;
}