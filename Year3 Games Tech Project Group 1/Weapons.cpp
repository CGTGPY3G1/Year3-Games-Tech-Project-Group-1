#include "Weapons.h"

Weapon::Weapon() {
}

Weapon::~Weapon() {
}

void Weapon::UpdateTimers(const float & delta) {
	if(reloading) {
		if(ammoInCache > 0) {
			reloadTime -= delta;
			if(reloadTime <= 0.0f) Reload();
		}
	}
	else if (ammoInClip > 0) {
		if(timeTilFire >= -1.0f) timeTilFire -= delta;
	}
}

void Weapon::Fire() {
	if(enabled) {
		if(ammoInClip) {
			ammoInClip--;
			if(ammoInClip <= 0) {
				if(ammoInCache > 0) {
					reloadTime = GetReloadTime();

				}
				reloading = true;
			}
			timeTilFire = GetFireRate();
		}
		else {
			Reload();
		}
	}
	
}

void Weapon::Reload() {
	if(enabled) {
		if(ammoInCache > 0) {
			const unsigned int rpc = GetRoundsPerClip();
			if(ammoInCache >= rpc) {
				ammoInClip = rpc;
				ammoInCache -= rpc;
			}
			else {
				ammoInClip = ammoInCache;
				ammoInCache = 0;
			}
			reloading = false;
		}
	}
}

void Weapon::AddAmmo(const int & amount) {
	if(enabled) {
		if(amount == 0) return;
		ammoInCache += amount;
		if(ammoInCache < 0) ammoInCache = 0;
	}
}

const bool Weapon::IsReloading() const {
	return enabled && reloading;
}

const bool Weapon::NeedsReloading() const {
	return enabled && !reloading && ammoInClip == 0 && ammoInCache > 0;
}

const bool Weapon::CanFire() const {
	return enabled && !reloading && ammoInClip > 0 && timeTilFire <= 0;
}

const unsigned int Weapon::GetAmmoInCache() const {
	return ammoInCache;
}

const unsigned int Weapon::GetAmmoInClip() const {
	return ammoInClip;
}

void Weapon::Init() {
	reloading = false;

}

bool Weapon::operator==(const Weapon & other) {
	return GetType() == other.GetType();
}

Pistol::Pistol() : Weapon(){
	enabled = true;
}

Pistol::~Pistol() {
}

const sf::IntRect Pistol::GetTextureRect() {
	static const sf::IntRect toReturn = sf::IntRect(38, 57, 24, 11);
	return toReturn;
}

const WeaponType Pistol::GetType() const {
	static const WeaponType type = WeaponType::WeaponTypePistol;
	return type;
}

const float Pistol::GetFireRate() const {
	return 0.4f;
}

const float Pistol::GetReloadTime() const {
	return 3.0f;
}

const Vector2 Pistol::GetImageOffset() const {
	return Vector2(5.0f, 0.0f);
}

const float Pistol::DistanceToFirePoint() const {
	return 7.0f;
}

const unsigned int Pistol::GetRoundsPerClip() const {
	return 12;
}

Uzi::Uzi() : Weapon(){
	enabled  = true;
}

Uzi::~Uzi() {
}

const sf::IntRect Uzi::GetTextureRect() {
	static const sf::IntRect toReturn = sf::IntRect(2, 58, 32, 11);
	return toReturn;
}

const WeaponType Uzi::GetType() const {
	static const WeaponType type = WeaponType::WeaponTypeUzi;
	return type;
}

const float Uzi::GetFireRate() const {
	return 0.075f;
}

const float Uzi::GetReloadTime() const {
	return 4.0f;
}

const Vector2 Uzi::GetImageOffset() const {
	return Vector2(6.0f, 0.0f);
}

const float Uzi::DistanceToFirePoint() const {
	return 25.0f;
}

const unsigned int Uzi::GetRoundsPerClip() const {
	return 50;
}

NullWeapon::NullWeapon() : Weapon() {
	enabled = false;
}

NullWeapon::~NullWeapon() {
}

const sf::IntRect NullWeapon::GetTextureRect() {
	return sf::IntRect(98, 54, 1, 1);
}

const WeaponType NullWeapon::GetType() const {
	return WeaponType::WeaponTypeNull;
}

const float NullWeapon::GetFireRate() const {
	return 1000.0f;
}

const float NullWeapon::GetReloadTime() const {
	return 1000.0f;
}

const Vector2 NullWeapon::GetImageOffset() const {
	return Vector2(0.0f, 0.0f);
}

const float NullWeapon::DistanceToFirePoint() const {
	return 0.0f;
}

const unsigned int NullWeapon::GetRoundsPerClip() const {
	return 0;
}
