#pragma once
#ifndef WS_ATTACKER_IDENTITY_H
#define WS_ATTACKER_IDENTITY_H
#include "ScriptableComponent.h"
#include "AttackTypes.h"
namespace WeiSungEngine {
	class GameObject;
	class RigidBody2D;
	class TileMapper;
	class Transform2D;

	struct AttackerInfo {
		AttackerInfo(std::weak_ptr<GameObject> attacker = std::weak_ptr<GameObject>(), const std::string & attackerName = "", const unsigned int & attackerID = 0, const AttackType & type = AttackType::NullHit);
		const bool IsAlive() const;
		const Vector2 GetPosition();

		friend bool operator == (const AttackerInfo & lhs, const AttackerInfo & rhs);
		friend bool operator == (const AttackerInfo & lhs, const std::string & rhs);
		friend bool operator == (const AttackerInfo & lhs, const unsigned int & rhs);
		std::string name;
		AttackType attackType;
		std::weak_ptr<GameObject> gameObject;
		unsigned int iD = 0;
	};

	class AttackerIdentityScript : public ScriptableComponent {
	public:
		AttackerIdentityScript();
		AttackerIdentityScript(std::weak_ptr<GameObject> gameObject);
		~AttackerIdentityScript();
		const AttackerInfo GetInfo() const;
		void Start() override;
		const ComponentType Type() const override { return COMPONENT_ATTACKER_IDENTITY_SCRIPT; }
		const std::string GetName() const override { return "AttackerIdentityScript"; }
		int GetSortOrder() override;
		void Reset();
		void SetAttackerName(const std::string & name);
		const std::string GetAttackerName() const;
		void SetAttacker(std::weak_ptr<GameObject> attacker = std::weak_ptr<GameObject>(), const std::string & name = "", const AttackType attackType = AttackType::NullHit);
		const std::weak_ptr<GameObject> GetAttacker() const;
		void SetAttackerID(const unsigned int & newId);
		const unsigned int GetAttackerID() const;
		const bool IsValid() const;
	private:
		AttackerInfo info;
		bool valid = false;
	};

}
#endif // !WS_ATTACKER_IDENTITY_H
