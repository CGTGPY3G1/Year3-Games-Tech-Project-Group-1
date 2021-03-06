#include "AttackerIdentityScript.h"
#include "GameObject.h"
#include "Transform2D.h"
namespace WeiSungEngine {
	AttackerIdentityScript::AttackerIdentityScript() : ScriptableComponent() {
	}

	AttackerIdentityScript::AttackerIdentityScript(std::weak_ptr<GameObject> gameObject) : ScriptableComponent(gameObject) {
	}

	AttackerIdentityScript::~AttackerIdentityScript() {
	}

	const AttackerInfo AttackerIdentityScript::GetInfo() const {
		return info;
	}

	void AttackerIdentityScript::Start() {
	}

	void AttackerIdentityScript::Reset() {
		SetAttacker(std::weak_ptr<GameObject>(), "");
	}

	void AttackerIdentityScript::SetAttackerName(const std::string & name) {
		info.name = name;
	}

	const std::string AttackerIdentityScript::GetAttackerName() const {
		return info.name;
	}

	void AttackerIdentityScript::SetAttacker(std::weak_ptr<GameObject> attacker, const std::string & name, const AttackType attackType) {
		valid = (attacker.use_count() > 0);
		info.iD = valid ? attacker.lock()->GetCombinedObjectID() : 0;
		info.gameObject = attacker;
		info.name = name;
		info.attackType = attackType;
	}

	const std::weak_ptr<GameObject> AttackerIdentityScript::GetAttacker() const {
		return info.gameObject;
	}

	void AttackerIdentityScript::SetAttackerID(const unsigned int & newId) {
		info.iD = newId;
	}

	const unsigned int AttackerIdentityScript::GetAttackerID() const {
		return info.iD;
	}

	const bool AttackerIdentityScript::IsValid() const {
		return valid && info.iD != 0;
	}

	int AttackerIdentityScript::GetSortOrder() {
		const static int order = TypeInfo::ScriptSortOrder<AttackerIdentityScript>();
		return order;
	}

	AttackerInfo::AttackerInfo(std::weak_ptr<GameObject> attacker, const std::string & attackerName, const unsigned int & attackerID, const AttackType & type) : gameObject(attacker), name(attackerName), iD(attackerID), attackType(type) {
	}

	const bool AttackerInfo::IsAlive() const {
		return gameObject.use_count() > 0 && gameObject.lock()->GetComponent<Transform2D>().use_count() > 0;
	}

	const Vector2 AttackerInfo::GetPosition() {
		return gameObject.use_count() > 0 ? gameObject.lock()->GetComponent<Transform2D>().use_count() > 0 ? gameObject.lock()->GetComponent<Transform2D>().lock()->GetPosition() : Vector2() : Vector2();
	}

	bool operator==(const AttackerInfo & lhs, const AttackerInfo & rhs) {
		return lhs.iD == rhs.iD && lhs.name.compare(rhs.name) == 0;
	}

	bool operator==(const AttackerInfo & lhs, const std::string & rhs) {
		return lhs.name.compare(rhs) == 0;
	}

	bool operator==(const AttackerInfo & lhs, const unsigned int & rhs) {
		return lhs.iD == rhs;
	}
}
