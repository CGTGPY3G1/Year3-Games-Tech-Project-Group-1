#pragma once
#ifndef WS_CAMERA_COMPONENT_H
#define WS_CAMERA_COMPONENT_H
#include "ScriptableComponent.h"
#include <SFML\Graphics\View.hpp>
class RigidBody2D;
class Transform2D;
class CameraFollowScript : public ScriptableComponent {
public:
	CameraFollowScript();
	CameraFollowScript(std::weak_ptr<GameObject> gameObject);
	~CameraFollowScript();
	const ComponentType Type() const override { return COMPONENT_CAMERA_FOLLOW; }
	void Start() override;
	void Update(const float & deltaTime) override;
	void SetZoom(const float & zoom);
	float GetZoom();
	void SetTarget(std::weak_ptr<RigidBody2D> target);
private:
	std::weak_ptr<RigidBody2D> myTarget;
	std::weak_ptr<Transform2D> myTransform;
	sf::View & view = sf::View();
	float zoomLevel = 1.0f;
};


#endif // !WS_CAMERA_COMPONENT_H



