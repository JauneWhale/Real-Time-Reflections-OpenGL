#include "ShadowLight.h"

//Static vars
std::string ShadowLight::className = "ShadowLight";

ShadowLight::ShadowLight() : Light() {}
ShadowLight::~ShadowLight(){}

//Initialize
void ShadowLight::initialize(TiXmlElement* element)
{
	Light::initialize(element);
	this->initialize();
}
void ShadowLight::initialize(std::string name, glm::vec4 intensity)
{
	Light::initialize(name,intensity);
	this->initialize();
}
void ShadowLight::initialize()
{
	this->lightCamera = new Camera3rdPerson();
}

//Type
std::string ShadowLight::getClassname()
{
	return ShadowLight::className;
}

//Transformations
void ShadowLight::updateTransformationMatrix()
{
	Light::updateTransformationMatrix();
	this->lightCamera->setCameraPos(this->getTranslation());
	glm::mat4 worldToCameraMatrix = this->lightCamera->getWorldToCameraMatrix();
	//Singleton<GLState>::Instance()->shadowLightWorldToCameraMatrix = worldToCameraMatrix;
}