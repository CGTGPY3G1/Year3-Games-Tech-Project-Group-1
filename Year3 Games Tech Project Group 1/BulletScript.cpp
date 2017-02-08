#include "BulletScript.h"
#include "GameObject.h"
BulletScript::BulletScript() : ScriptableComponent() {
}
BulletScript::BulletScript(std::weak_ptr<GameObject> gameObject) : ScriptableComponent(gameObject) {

}
BulletScript::~BulletScript() {
}
void BulletScript::Start() {
	lifeTime = 3.5f;
}
void BulletScript::Update(const float & deltaTime) {
	if(lifeTime > 0.0f) lifeTime -= deltaTime;
	else {
		if(!GetComponent<AudioSource>().lock()->IsPlaying()) gameObject.lock()->Destroy();
		else if(alive) {
			GetComponent<RigidBody2D>().lock()->SetEnabled(false);
			GetComponent<SpriteRenderer>().lock()->SetEnabled(false);
			alive = false;
		}
	}
}

void BulletScript::OnCollisionEnter(const CollisionData & data) {
	lifeTime = -1.0f;
	std::shared_ptr<GameObject> g = data.gameObject.lock();
	if(g) {
		std::shared_ptr<HealthScript> hs = g->GetComponent<HealthScript>().lock();
		if(hs) {
			hs->AddToHealth(-Random::RandomFloat(30.0f, 60.0f));
		}
	}
}

int BulletScript::GetSortOrder() {
	const static int order = TypeInfo::ScriptSortOrder<BulletScript>();
	return order;
}
