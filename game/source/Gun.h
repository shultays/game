#pragma once

#include <string>
#include "cMat.h"
#include "cVec.h"
#include "cResources.h"
#include "sol.h"
#include "json.h"
#include "cSound.h"	
#include "BuffFloat.h"	
#include "DirentHelper.h"	

class Bloodworks;
class Bullet;
class LaserRenderable;
class Monster;
class StripLaserRenderable;

class Gun
{
	friend class BloodworksLuaWorld;

	Bloodworks *bloodworks;
	std::string name;
	std::string bulletTexturePath;
	std::string iconPath;
	cTextureShr bulletTexture;
	Vec2 bulletSize;
	Vec2 bulletMeshShift;
	float bulletRadius;
	float bulletSpeed;

	int bulletLevelModifier;

	std::string path;

	std::string scriptName;
	sol::table scriptTable;
	int id;

	sol::table data;
	sol::table newBulletData;

	Bullet* addBullet();

	float spreadAngle;
	float crosshairDistance;
	IntVec2 damage;

	bool hideSpread;

	bool isTriggered;

	Vec4 shootParticleColor;
	LaserRenderable *laser;

	StripLaserRenderable *stripLaser;

	cSoundSampleWithParams gunShootSound;
	cSoundHandle gunShootSoundHandle;

	cSoundSampleWithParams bulletHitSound;

	cSoundSampleWithParams reloadBeginSound;
	cSoundSampleWithParams reloadEndSound;

	float lastShootSoundTime;

	float lastAmmoReset;
	int currentAmmo;
	int maxAmmo;
	float reloadTime;
	float remainingReload;
	bool reloading;
	bool reloadEnding;

	int buffedMaxAmmo;

	BuffFloat reloadSpeedMultiplier;

	sol::function dynamicSpawnChanceCall;
	sol::function onTickCall;
	sol::function onBulletTickCall;
	sol::function onBulletHitCall;
	sol::function onReloadStartCall;
	sol::function onReloadEndCall;
	sol::function onPlayerDamagedCall;

	float spawnChance;
	float bulletLifeTime;

	float maxSoundPlayInterval;
	float gunShootSoundFadein;
	float gunShootSoundFadeout;
	bool playGunShootSound;
	float gunShootSoundCurVolume;
	bool gunShootSoundContinuous;
	bool showShootAnimation;
	bool ultimate;
	int index;
public:
	Gun(Bloodworks *bloodworks, nlohmann::json& j, const DirentHelper::File& file);
	~Gun();
	void stop();
	void start();
	void tick(float dt);

	float getMaxCrosshairDistance() const;
	float getSpreadAngle() const;
	int getId() const;
	sol::table& getScriptTable();
	const std::string& getName() const
	{
		return name;
	}
	const std::string& getIconPath() const;
	bool spreadVisible() const;
	void setTriggered(bool param1);
	const Vec4& getShootingParticleColor() const;

	bool isLaser() const
	{
		return laser != nullptr;
	}

	void updateLaser(const Vec2& pos, float angle);

	int getRandomDamage() const
	{
		return randInt(damage[0], damage[1]);
	}
	void reset();
	void onBulletHit(Bullet *bullet, Monster* monster);

	bool hasAmmo() const
	{
		return reloading == false && (maxAmmo == 0 || currentAmmo > 0);
	}

	void addAmmo();
	void consumeAmmo();
	void reload();
	int getCurrentAmmo() const;
	bool isReloading() const;
	float getReloadPercentage() const;

	int getMaxAmmo() const;
	float getSpawnChance();
	float getBulletLifeTime() const;
	const std::string& getScriptName() const;
	bool isUltimate() const
	{
		return ultimate;
	}
	int onPlayerDamaged(int damage, float dir, sol::table& params);
	bool onBulletTick(Bullet* bullet);

	void setIndex(int index)
	{
		this->index = index;
	}
	int getIndex() const
	{
		return index;
	}
};