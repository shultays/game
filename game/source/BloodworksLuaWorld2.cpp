#include "BloodworksLuaWorld.h"

#include "Bloodworks.h"
#include "cGlobals.h"
#include "Bullet.h"
#include "Monster.h"
#include "Gun.h"
#include "MonsterController.h"
#include "BulletController.h"
#include "MissionController.h"
#include "cParticle.h"
#include "cRenderable.h"
#include "GameObject.h"
#include "Player.h"
#include "LaserRenderable.h"
#include "cShader.h"
#include "cRenderable.h"
#include "MonsterTemplate.h"
#include "cPostProcess.h"
#include "BuffFloat.h"
#include "LuaBuffController.h"
#include "Bonus.h"
#include "DropController.h"
#include "Bonus.h"
#include "cTimeProfiler.h"
#include "cAnimatedRenderable.h"
#include "cTextRenderable.h"
#include "cTexturedQuadRenderable.h"
#include "GameObjectTemplate.h"
#include "CollisionController.h"
#include "cCircle.h"
#include "cRect.h"
#include "cCapsule.h"
#include "Perk.h"
#include "StripLaserRenderable.h"
#include "BloodworksSteam.h"
#include "BloodRenderable.h"
#include "ExplosionController.h"

void BloodworksLuaWorld::init2()
{

	lua.new_usertype<Bullet>("Bullet",
		"id", sol::readonly(&Bullet::id),

		"position", sol::property(&Bullet::getPosition, &Bullet::setPosition),
		"moveSpeed", &Bullet::moveSpeed,
		"moveAngle", &Bullet::moveAngle,
		"lifeTime", &Bullet::lifeTime,
		"startTime", &Bullet::startTime,
		"meshRotation", &Bullet::meshRotation,

		"hasCollision", &Bullet::hasCollision,

		"meshScale", &Bullet::meshScale,
		"scale", sol::readonly(&Bullet::scale),
		"setScale", [](Bullet& b, float scale) { b.scale = scale; b.updateDrawable(); },

		"moveDir", sol::readonly(&Bullet::moveDir),
		"moveVelocity", sol::readonly(&Bullet::moveVelocity),

		"radius", &Bullet::radius,
		"damage", &Bullet::damage,

		"monsterBullet", sol::readonly(&Bullet::monsterBullet),

		"hitsMultipleTimes", &Bullet::hitsMultipleTimes,
		"penetrateCount", &Bullet::penetrateCount,
		"penetrateUsed", sol::readonly(&Bullet::penetrateUsed),

		"setScript", &Bullet::setScript,

		"isDead", sol::readonly(&Bullet::isDead),

		"data", &Bullet::data,
		"onDamageArgs", &Bullet::onDamageArgs,

		"setPosition", &Bullet::setPosition,

		"addRenderableTexture", &Bullet::addRenderableTexture,
		"addRenderableTextureWithSize", &Bullet::addRenderableTextureWithSize,
		"addRenderableTextureWithPosAndSize", &Bullet::addRenderableTextureWithPosAndSize,

		"setColor", &Bullet::setColor,

		"modifyDrawLevel", &Bullet::modifyDrawLevel,
		"addTrailParticle", &Bullet::addTrailParticle,

		"updateDrawable", &Bullet::updateDrawable,
		"removeSelf", &Bullet::removeSelf
		);


	lua.set_function("addPostProcess",
		[&](const std::string& postProcessShader) -> cPostProcess*
	{
		cPostProcess *postProcess = new cPostProcess();
		postProcess->init(bloodworks, resources.getShader("resources/post_process/default.vs", postProcessShader), 100);
		return postProcess;
	});

	lua.set_function("removePostProcess",
		[&](cPostProcess *postProcess)
	{
		bloodworks->removePostProcess(postProcess);
		SAFE_DELETE(postProcess);
	});

	lua.set_function("approachAngle",
		[&](float angle, float angleToApproach, float maxRotation) -> float
	{
		return approachAngle(angle, angleToApproach, maxRotation);
	});

	lua.set_function("isKeyReleased",
		[&](int key) -> bool
	{
		return input.isKeyReleased(key);
	});

	lua.set_function("getScore",
		[&]() -> int
	{
		return bloodworks->getPlayer()->getScore();
	});

	lua.set_function("addScore",
		[&](int score)
	{
		bloodworks->getPlayer()->addScore(score);
	});

	lua.set_function("setScore",
		[&](int score)
	{
		bloodworks->getPlayer()->setScore(score);
	});

	lua.set_function("isKeyPressed",
		[&](int key) -> bool
	{
		return input.isKeyPressed(key);
	});
	lua.set_function("isKeyUp",
		[&](int key) -> bool
	{
		return input.isKeyUp(key);
	});

	lua.set_function("isKeyDown",
		[&](int key) -> bool
	{
		return input.isKeyDown(key);
	});

	lua.set_function("log",
		[&](const std::string& str)
	{
		out << str << "\n";
	});

	lua.set_function("angleDiff",
		[&](float angle, float angleToApproach) -> float
	{
		return angleDiff(angle, angleToApproach);
	});

	lua.set_function("getUniqueId", [&]()
	{
		return bloodworks->getUniqueId();
	});

	lua.set_function("getGlobalUniqueId", [&]()
	{
		return bloodworks->getGlobalUniqueId();
	});

	lua.set_function("addLine",
		[&](const Vec2& pos0, const Vec2& pos1, float time, int color)
	{
		debugRenderer.addLine(pos0, pos1, time, color ? color : -1);
	});

	lua.set_function("addCircle",
		[&](const Vec2& pos0, float r, float time, int color)
	{
		debugRenderer.addCircle(pos0, r, time, color ? color : -1);
	});

	lua.set_function("addExplosion",
		[&](const Vec2& pos, float scale, float speed, int minDamage, int maxDamage, float startTime, bool damagePlayer, sol::function onHit, bool noParticle)
	{
		return &bloodworks->addExplosion(pos, scale, speed, minDamage, maxDamage, startTime, damagePlayer, onHit, noParticle);
	});

	lua.set_function("addCustomExplosion",
		[&](const Vec2& pos, bool particle)
	{
		return &bloodworks->addExplosion(pos, 100.0f, 100.0f, 50, 100, 0.0f, false, sol::function(), !particle);
	});

	lua.set_function("playSound",
		[&](sol::table& args)
	{
		bloodworks->playOneShotSound(args);
	});

	lua.set_function("getFreePosition",
		[&](float radius, unsigned ignoreFlags)
	{
		return bloodworks->getCollisionController()->getFreePosition(radius, ignoreFlags);
	});

	lua.set_function("hasCircleCollision",
		[&](const Vec2& pos, float radius, unsigned ignoreFlags)
	{
		return bloodworks->getCollisionController()->hasCollision(Circle(pos, radius), ignoreFlags);
	});

	lua.set_function("hasCapsuleCollision",
		[&](const Vec2& pos, const Vec2& pos2, float radius, unsigned ignoreFlags)
	{
		return bloodworks->getCollisionController()->hasCollision(Capsule(pos, pos2, radius), ignoreFlags);
	});

	lua.set_function("hasRectCollision",
		[&](const Vec2& pos, const Vec2& size, float rotation, float radius, unsigned ignoreFlags)
	{
		return bloodworks->getCollisionController()->hasCollision(Rect(pos, size, rotation, radius), ignoreFlags);
	});

	lua.set_function("getCollisionForRay",
		[&](const Vec2& pos, const Vec2& ray, float radius, unsigned ignoreFlags)
	{
		return bloodworks->getCollisionController()->getRayDistance(pos, ray, radius, ignoreFlags);
	});

	lua.new_usertype<Bonus>("Bonus",
		"name", sol::readonly(&Bonus::name),
		"scriptName", sol::readonly(&Bonus::scriptName),
		"data", &Bonus::data,
		"setActive", &Bonus::setActive,
		"isActive", &Bonus::isActive
		);

	lua.new_usertype<Perk>("Perk",
		"name", sol::readonly(&Perk::name),
		"scriptName", sol::readonly(&Perk::scriptName),

		"level", sol::readonly(&Perk::level),
		"maxLevel", sol::readonly(&Perk::maxLevel)
		);

	lua.new_usertype<StripLaserRenderable>("StripLaser",
		"setShooting", &StripLaserRenderable::setShooting,
		"setSpeed", &StripLaserRenderable::setSpeed,
		"setPos", &StripLaserRenderable::setPos,
		"stop", &StripLaserRenderable::stop
		);

	lua.new_usertype<Gun>("Gun",
		"id", sol::readonly(&Gun::id),
		"data", &Gun::data,
		"newBulletData", &Gun::newBulletData,
		"name", sol::readonly(&Gun::name),
		"scriptName", sol::readonly(&Gun::scriptName),
		"ultimate", sol::readonly(&Gun::ultimate),
		"bulletRadius", &Gun::bulletRadius,
		"bulletSpeed", &Gun::bulletSpeed,
		"scriptTable", &Gun::scriptTable,
		"bulletLifeTime", &Gun::bulletLifeTime,
		"showShootAnimation", &Gun::showShootAnimation,
		"shootParticleColor", &Gun::shootParticleColor,
		"firingSoundFadein", &Gun::gunShootSoundFadein,
		"firingSoundFadeout", &Gun::gunShootSoundFadeout,
		"firingCurVolume", &Gun::gunShootSoundCurVolume,
		"playFiringSound", &Gun::playGunShootSound,

		"spreadAngle", &Gun::spreadAngle,
		"crosshairDistance", &Gun::crosshairDistance,

		"isTriggered", &Gun::isTriggered,

		"stripLaser", &Gun::stripLaser,

		"getRandomDamage", &Gun::getRandomDamage,

		"hasAmmo", &Gun::hasAmmo,
		"addAmmo", &Gun::addAmmo,
		"consumeAmmo", &Gun::consumeAmmo,
		"reload", &Gun::reload,
		"isReloading", &Gun::isReloading,
		"getReloadPercentage", &Gun::getReloadPercentage,
		"getMaxAmmo", &Gun::getMaxAmmo,
		"getCurrentAmmo", &Gun::getCurrentAmmo,
		"reloadSpeedMultiplier", &Gun::reloadSpeedMultiplier,

		"addBullet", &Gun::addBullet,
		"laser", &Gun::laser
		);

	lua.set_function("convertToScreenPosition",
		[&](const Vec2& pos) -> Vec2
	{
		return (pos - bloodworks->getCameraPos()) / bloodworks->getCameraZoom() + bloodworks->getScreenDimensions().toVec() * 0.5f;
	});

	lua.set_function("getGunCount",
		[&]() -> int
	{
		return bloodworks->getGuns().size();
	});

	lua.set_function("getGunAtIndex",
		[&](int index) -> Gun*
	{
		return bloodworks->getGuns()[index];
	});

	lua.set_function("getGun",
		[&](const std::string& script) -> Gun*
	{
		for (auto& gun : bloodworks->getGuns())
		{
			if (gun->getScriptName() == script)
			{
				return gun;
			}
		}
		return nullptr;
	});

	lua.set_function("isPaused",
		[&]() -> bool
	{
		return bloodworks->isPaused();
	});

	lua.set_function("loadMission",
		[&](const std::string& mission)
	{
		bloodworks->loadMissionNextTick(mission);
	});

	lua.set_function("isGoodSpawnPos",
		[&](const Vec2& pos) 
	{
		return bloodworks->getMapLimits().isInside(pos, 30);
	});

	lua.set_function("addGameObject",
		[&](const std::string& script, const sol::table& params) -> GameObject*
	{
		return bloodworks->getMissionController()->addGameObject(script, nullptr, params);
	});

	lua.set_function("addGameObjectUsingTemplate",
		[&](const std::string& templateName, const sol::table& params) -> GameObject*
	{
		return bloodworks->getMissionController()->addGameObjectUsingTemplate(templateName, params);
	});

	lua.set_function("addCustomBullet",
		[&](const sol::table& params) -> Bullet*
	{
		return bloodworks->getBulletController()->addCustomBullet(params);
	});

	lua.set_function("getAllMonsterTypeCount",
		[&]() -> int
	{
		return bloodworks->getMonsterController()->getTemplates().size();
	});

	lua.set_function("getMonsterTypeAt",
		[&](int i) -> MonsterTemplate*
	{
		return bloodworks->getMonsterController()->getTemplates()[i];
	});

	lua.set_function("addMonster",
		[&](std::string monsterTemplate) -> Monster*
	{
		return bloodworks->getMonsterController()->addMonster(monsterTemplate);
	});

	lua.set_function("getMonsterCount",
		[&]() -> int
	{
		return bloodworks->getMonsterController()->getMonsterCount();
	});

	lua.set_function("getMonsterAtIndex",
		[&](int i) -> Monster*
	{
		return bloodworks->getMonsterController()->getMonsterAtIndex(i);
	});
	lua.set_function("getMonster",
		[&](int i) -> Monster*
	{
		return bloodworks->getMonsterController()->getMonster(i);
	});

	lua.set_function("getClosestMonster",
		[&](const Vec2& pos) -> Monster*
	{
		return bloodworks->getMonsterController()->getClosestMonster(pos);
	});

	lua.set_function("loadScript",
		[&](const std::string& path)
	{
		bloodworks->loadLuaFile(path);
	});

	lua.set_function("getClosestMonsterWithIgnoreId",
		[&](const Vec2& pos, const sol::table& ignoreIdList) -> Monster*
	{
		std::vector<int> ignoreIds;

		if (auto argIgnoreIds = ignoreIdList)
		{
			int t = 1;
			while (argIgnoreIds[t])
			{
				ignoreIds.push_back(argIgnoreIds[t].get<int>());
				t++;
			}
		}
		return bloodworks->getMonsterController()->getClosestMonsterWithIgnoreId(pos, ignoreIds);
	});

	lua.set_function("getClosestMonsterInRange",
		[&](const Vec2& pos, float range) -> Monster*
	{
		return bloodworks->getMonsterController()->getClosestMonsterInRange(pos, range);
	});

	lua.set_function("getRandomMapPosition",
		[&]() -> Vec2
	{
		return Vec2(randFloat(bloodworks->getMapLimits().getMin().x + 50, bloodworks->getMapLimits().getMax().x - 50.0f), randFloat(bloodworks->getMapLimits().getMin().y + 50.0f, bloodworks->getMapLimits().getMax().y - 50));
	});


	lua.set_function("getMapSize",
		[&](sol::table args) -> Vec2
	{
		return bloodworks->getMapSize();
	});

	lua.set_function("getRandomPosition",
		[&](sol::table args) -> Vec2
	{
		return bloodworks->getMonsterController()->getRandomPos(args);
	});

	lua.set_function("getClosestMonsterInRangeWithIgnoreId",
		[&](const Vec2& pos, float range, const sol::table& ignoreIdList) -> Monster*
	{
		std::vector<int> ignoreIds;
		if (auto argIgnoreIds = ignoreIdList)
		{
			int t = 1;
			while (argIgnoreIds[t])
			{
				ignoreIds.push_back(argIgnoreIds[t].get<int>());
				t++;
			}
		}
		return bloodworks->getMonsterController()->getClosestMonsterInRangeWithIgnoreId(pos, range, ignoreIds);
	});

	lua.set_function("damageMonstersInRangeWithIgnoreId",
		[&](const Vec2& pos, float range, int minRange, int maxRange, bool mark, int ignoreId)
	{
		return bloodworks->getMonsterController()->damageMonstersInRangeWithIgnoreId(pos, range, minRange, maxRange, mark, ignoreId);
	});

	lua.new_usertype<MonsterController::MonsterHitResult>("MonsterHitResult",
		"monster", &MonsterController::MonsterHitResult::monster,
		"distance", &MonsterController::MonsterHitResult::distance
		);

	lua.set_function("getClosestMonsterOnLine",
		[&](const Vec2& pos, const Vec2& ray, float radius, sol::table& args)
	{
		return bloodworks->getMonsterController()->getClosestMonsterOnLine(pos, ray, radius, args);
	});

	lua.set_function("runForEachMonsterInRadius",
		[&](const Vec2& pos, float radius, sol::table& args, sol::function& func)
	{
		std::function<bool(Monster*)> func2 = [&func](Monster* monster)
		{
			return func(monster);
		};
		bloodworks->getMonsterController()->runForRadius(pos, radius, args, func2);
	});

	lua.set_function("runForEachMonsterOnLine",
		[&](const Vec2& pos, const Vec2& ray, float radius, sol::table& args, sol::function& func)
	{
		std::function<bool(Monster*)> func2 = [&func](Monster* monster)
		{
			return func(monster);
		};
		bloodworks->getMonsterController()->runForRay(pos, ray, radius, args, func2);
	});

	lua.set_function("spawnRandomBonus",
		[&](const Vec2& pos)
	{
		return bloodworks->getDropController()->spawnDrop(pos);
	});

	lua.set_function("spawnRandomDrop",
		[&](const Vec2& pos, float time)
	{
		return bloodworks->getDropController()->spawnDrop(pos, time > 0.1f ? time : DEFAULT_DROP_DURATION);
	});

	lua.set_function("spawnRandomGun",
		[&](const Vec2& pos, float time)
	{
		return bloodworks->getDropController()->spawnGun(pos, -1, time > 0.1f ? time : DEFAULT_DROP_DURATION);
	});

	lua.set_function("spawnRandomBonus",
		[&](const Vec2& pos, float time)
	{
		return bloodworks->getDropController()->spawnBonus(pos, -1, time > 0.1f ? time : DEFAULT_DROP_DURATION);
	});


	lua.set_function("getCameraZoom",
		[&]()
	{
		return bloodworks->getCameraZoom();
	});
	lua.set_function("getCameraPos",
		[&]()
	{
		return bloodworks->getCameraPos();
	});
	lua.set_function("getCameraAngle",
		[&]()
	{
		return bloodworks->getCameraAngle();
	});

	lua.set_function("getLastBonusSpawnTime",
		[&]()
	{
		return bloodworks->getDropController()->getLastSpawnTime();
	});

	lua.set_function("getGameSpeedMultiplier",
		[&]() -> BuffFloat&
	{
		return bloodworks->getMissionController()->getGameSpeedMultiplierBuff();
	});

	lua.set_function("getRandomGun",
		[&]()
	{
		return bloodworks->getGuns()[randInt(bloodworks->getGuns().size())];
	}
	);
	lua.set_function("getRandomBonus",
		[&]()
	{
		return bloodworks->getBonuses()[randInt(bloodworks->getBonuses().size())];
	}
	);
	lua.set_function("spawnGun",
		[&](const Vec2& pos, const std::string& name, float time)
	{
		int index = 0;
		for (auto& gun : bloodworks->getGuns())
		{
			if (name == gun->getScriptName())
			{
				return bloodworks->getDropController()->spawnGun(pos, index, time > 0.1f ? time : DEFAULT_DROP_DURATION);
			}
			index++;
		}
		return -1;
	});

	lua.set_function("spawnBonus",
		[&](const Vec2& pos, const std::string& name, float time)
	{
		int index = 0;
		for (auto& bonus : bloodworks->getBonuses())
		{
			if (name == bonus->getScriptName())
			{
				return bloodworks->getDropController()->spawnBonus(pos, index, time > 0.1f ? time : DEFAULT_DROP_DURATION);
			}
			index++;
		}
		return -1;
	});

	lua.set_function("removeDrop",
		[&](int id)
	{
		bloodworks->getDropController()->removeDrop(id);
	});


	lua.new_usertype<GameObjectTemplate>("GameObjectTemplate",
		"name", sol::readonly(&GameObjectTemplate::name),
		"scriptName", sol::readonly(&GameObjectTemplate::scriptName),
		"basePath", sol::readonly(&GameObjectTemplate::basePath),
		"jsonTable", sol::readonly(&GameObjectTemplate::jsonTable)
		);


	lua.new_usertype<MonsterTemplate>("MonsterTemplate",
		"name", sol::readonly(&MonsterTemplate::name),
		"scriptName", sol::readonly(&MonsterTemplate::scriptName),
		"experience", sol::readonly(&MonsterTemplate::experience),
		"textureSize", sol::readonly(&MonsterTemplate::size),
		"textureShift", sol::readonly(&MonsterTemplate::textureShift),
		"hitPoint", sol::readonly(&MonsterTemplate::hitPoint),
		"hitpoint", sol::readonly(&MonsterTemplate::hitPoint),
		"collisionRadius", sol::readonly(&MonsterTemplate::collisionRadius),
		"bulletRadius", sol::readonly(&MonsterTemplate::bulletRadius),
		"hasBlood", sol::readonly(&MonsterTemplate::hasBlood),
		"scriptPath", sol::readonly(&MonsterTemplate::scriptPath),
		"scriptTable", sol::readonly(&MonsterTemplate::scriptTable),
		"scriptArgs", sol::readonly(&MonsterTemplate::scriptArgs),
		"basePath", sol::readonly(&MonsterTemplate::basePath)
		);

	lua.new_usertype<ExplosionData>("ExplosionData",
		"id", sol::readonly(&ExplosionData::id),

		"renderable", &ExplosionData::ringRenderable,
		"position", &ExplosionData::pos,
		"startTime", &ExplosionData::startTime,
		"maxScale", &ExplosionData::maxScale,
		"scaleSpeed", &ExplosionData::scaleSpeed,
		"minDamage", &ExplosionData::minDamage,
		"maxDamage", &ExplosionData::maxDamage,
		"damagePlayer", &ExplosionData::damagePlayer,
		"onHit", &ExplosionData::onHit
		);

#ifdef SHOW_TIMINGS
	lua.set_function("createSumProfiler",
		[&](const std::string& name) -> cAccumulatedTimeProfiler&
	{
		return Coral::createAccumulatedTimeProfile(name.c_str());
	});

	lua.set_function("createProfiler",
		[&](const std::string& name) -> cTimeProfiler
	{
		return cTimeProfiler(name.c_str());
	});
	lua.new_usertype<cAccumulatedTimeProfiler>("SumProfiler",
		"start", &cAccumulatedTimeProfiler::start,
		"stop", &cAccumulatedTimeProfiler::stop
		);

	lua.new_usertype<cTimeProfiler>("Profiler",
		"start", &cTimeProfiler::start,
		"stop", &cTimeProfiler::stop
		);
#endif

	lua.new_usertype<Monster>("Monster",
		"name", &Monster::name,
		"id", sol::readonly(&Monster::id),

		"debugVal", &Monster::debugVal,

		"scriptTable", &Monster::scriptTable,

		"animationSpeed", &Monster::animationSpeed,
		"pathCheckDistance", &Monster::pathCheckDistance,

		"setVisible", &Monster::setVisible,
		"isVisible", &Monster::isVisible,

		"position", sol::property(&Monster::getPosition, &Monster::setPosition),
		"moveSpeed", &Monster::moveSpeed,
		"moveAngle", &Monster::moveAngle,
		"spriteAngle", &Monster::spriteAngle,
		"moveVelocity", sol::readonly(&Monster::moveVelocity),
		"moveDir", sol::readonly(&Monster::moveDir),

		"getPathPos", &Monster::getPathPos,

		"textureShift", &Monster::textureShift,
		"textureSize", &Monster::textureSize,

		"hitPoint", &Monster::hitPoint,
		"hitpoint", &Monster::hitPoint,

		"collisionRadius", &Monster::collisionRadius,
		"bulletRadius", &Monster::bulletRadius,

		"data", &Monster::data,
		"experienceMultiplier", &Monster::experienceMultiplier,
		"scoreMultiplier", &Monster::scoreMultiplier,

		"scale", sol::readonly(&Monster::scale),

		"moveSpeedMultiplier", &Monster::moveSpeedMultiplier,
		"colorMultiplier", &Monster::colorMultiplier,

		"isDead", sol::readonly(&Monster::isDead),
		"removeOnDead", &Monster::removeOnDead,
		"hasBlood", &Monster::hasBlood,
		"hasGibs", &Monster::hasGibs,

		"playAnimation", &Monster::playAnimation,
		"addTimer", &Monster::addTimer,
		"setScale", &Monster::setScale,

		"knockbackResistance", &Monster::knockbackResistance,
		"addKnockback", &Monster::addKnockback,

		"dropChance", &Monster::dropChance,
		"hasCollision", &Monster::hasCollision,

		"canGetOneShooted", &Monster::canGetOneShooted,

		"killSelf", [&](Monster* monster)
	{
		monster->killSelf(Vec2::zero());
	},
		"killSelfWithDir", &Monster::killSelf,

		"doDamage", &Monster::doDamage,
		"doDamageWithArgs", &Monster::doDamageWithArgs,
		"addIgnoreId", &Monster::addIgnoreId,
		"hasIgnoreId", &Monster::hasIgnoreId,
		"copyIgnoreId", &Monster::copyIgnoreId,

		"addParticleSpawner", &Monster::addParticleSpawner,

		"spawnParticle", &Monster::spawnParticle,
		"spawnParticleShifted", &Monster::spawnParticleShifted,

		"spawnParticleWithoutArgs", &Monster::spawnParticleWithoutArgs,
		"spawnParticleShiftedWithoutArgs", &Monster::spawnParticleShiftedWithoutArgs,

		"modifyDrawLevel", &Monster::modifyDrawLevel,
		"setDrawLevel", &Monster::setDrawLevel,
		"getDrawLevel", &Monster::getDrawLevel,
		"monsterTemplate", sol::readonly(&Monster::monsterTemplate)
		);

	lua.new_usertype<GameObject>("GameObject",
		"id", sol::readonly(&GameObject::id),

		"toBeRemoved", &GameObject::toBeRemoved,
		"script", &GameObject::script,
		"data", &GameObject::data,
		"addText", &GameObject::addText,
		"addTexture", &GameObject::addTexture,
		"addParticle", &GameObject::addParticle,
		"addAnimation", &GameObject::addAnimation,

		"removeAnimation", &GameObject::removeAnimation,
		"removeParticle", &GameObject::removeParticle,
		"removeText", &GameObject::removeText,
		"removeTexture", &GameObject::removeTexture,


		"setPosition", &GameObject::setPosition,
		"setRotation", &GameObject::setRotation,
		"setScale", &GameObject::setScale,

		"getPosition", &GameObject::getPosition,
		"getRotation", &GameObject::getRotation,
		"getScale", &GameObject::getScale,

		"addCircleCollider", &GameObject::addCircleCollider,
		"addRectCollider", &GameObject::addRectCollider,
		"addCapsuleCollider", &GameObject::addCapsuleCollider,

		"setLevel", &GameObject::setLevel,
		"setAlignment", &GameObject::setAlignment

		);

	lua.new_usertype<Player>("Player",
		"position", sol::readonly(&Player::pos),
		"moveSpeed", sol::readonly(&Player::moveSpeed),
		"crosshairPos", sol::readonly(&Player::crosshairPos),
		"crosshairDistance", sol::readonly(&Player::crosshairDistance),
		"gunPos", sol::readonly(&Player::gunPos),
		"aimDir", sol::readonly(&Player::aimDir),
		"aimAngle", sol::readonly(&Player::aimAngle),
		"moveAngle", sol::readonly(&Player::moveAngle),
		"moveDir", sol::readonly(&Player::moveDir),
		"visible", sol::readonly(&Player::visible),
		"isDead", sol::readonly(&Player::isDead),
		"colorMultiplier", &Player::colorMultiplier,
		"moveVelocity", sol::readonly(&Player::moveVelocity),

		"hitPoints", sol::readonly(&Player::hitPoints),
		"maxHitPoints", sol::readonly(&Player::maxHitPoints),

		"hitpoints", sol::readonly(&Player::hitPoints),
		"maxHitpoints", sol::readonly(&Player::maxHitPoints),

		"maxSpeed", &Player::maxSpeed,
		"maxRotateSpeed", &Player::maxRotateSpeed,
		"doDamage", &Player::doDamage,
		"doDamageWithArgs", &Player::doDamageWithArgs,
		"doHeal", &Player::doHeal,
		"gainExperience", &Player::gainExperience,
		"bulletSpeedMultiplier", &Player::bulletSpeedMultiplier,
		"shootSpeedMultiplier", &Player::shootSpeedMultiplier,
		"monsterExperienceMultiplier", &Player::monsterExperienceMultiplier,
		"damageMultiplier", &Player::damageMultiplier,
		"reloadSpeedMultiplier", &Player::reloadSpeedMultiplier,
		"globalMonsterSpeedMultiplier", &Player::globalMonsterSpeedMultiplier,
		"clipCountMultiplier", &Player::clipCountMultiplier,
		"gunSpreadMultiplier", &Player::gunSpreadMultiplier,
		"accelerationMultiplier", &Player::accelerationMultiplier,
		"canFireNextFrame", &Player::canFireNextFrame,
		"gun", sol::readonly(&Player::gun),
		"setGun", &Player::setGun,
		"ultimate", sol::readonly(&Player::secondaryGun),
		"setUltimate", &Player::setSecondaryGun,
		"level", sol::readonly(&Player::level),
		"experience", sol::readonly(&Player::experience),
		"experienceForNextLevel", sol::readonly(&Player::experienceForNextLevel),
		"playShootAnimation", &Player::playShootAnimation,

		"getMaxHitpoints", &Player::getMaxHitpoints,
		"getHitpoints", &Player::getHitpoints,
		"setMaxHitpoints", &Player::setMaxHitpoints,
		"setHitpoints", &Player::setHitpoints,

		"addKnockback", &Player::addKnockback,
		"data", &Player::data
		);

	lua.new_usertype<BuffFloat::BuffInfo>("BuffFloatInfo",
		"buffType", sol::readonly(&BuffFloat::BuffInfo::buffType),

		"buffAmount", sol::readonly(&BuffFloat::BuffInfo::buffAmount),
		"duration", sol::readonly(&BuffFloat::BuffInfo::duration),
		"fadeInDuration", sol::readonly(&BuffFloat::BuffInfo::fadeInDuration),
		"fadeOutDuration", sol::readonly(&BuffFloat::BuffInfo::fadeOutDuration),

		"restart", &BuffFloat::BuffInfo::restart,

		"removeAfterEnds", &BuffFloat::BuffInfo::removeAfterEnds,
		"getRemainingTime", &BuffFloat::BuffInfo::getRemainingTime,

		"setBuffAmount", &BuffFloat::BuffInfo::setBuffAmount,
		"setBuffDuration", &BuffFloat::BuffInfo::setBuffDuration,
		"getCurrentBuffAmount", &BuffFloat::BuffInfo::getCurrentBuffAmount,
		"setBuffFadeInFadeOut", &BuffFloat::BuffInfo::setBuffFadeInFadeOut

		);

	lua.new_usertype<BuffFloat>("BuffFloat",

		"create", sol::initializers([&](BuffFloat& memory)
	{
		new (&memory) BuffFloat();
		buffController->addBuff(&memory);
	}),
		sol::meta_function::garbage_collect, sol::destructor([&](BuffFloat& memory)
	{
		buffController->removeBuff(&memory);
		memory.~BuffFloat();
	}),

		"removeBuff", &BuffFloat::removeBuff,
		"addBuffWithType", &BuffFloat::addBuffWithType,
		"addBuff", [&](BuffFloat& buff, float amount)
	{
		int id = bloodworks->getUniqueId();
		buff.addBuff(id, amount, BuffFloat::multiply_buff);
		return id;
	},

		"addBuffWithId", [&](BuffFloat& buff, int id, float amount)
	{
		buff.addBuff(id, amount, BuffFloat::multiply_buff);
		return id;
	},
		"setBaseValue", &BuffFloat::setBaseValue,
		"setBuffAmount", &BuffFloat::setBuffAmount,
		"setBuffDuration", &BuffFloat::setBuffDuration,
		"setBuffFadeInFadeOut", &BuffFloat::setBuffFadeInFadeOut,
		"getBuffRemainingTime", &BuffFloat::getBuffRemainingTime,

		"setUseRealtime", &BuffFloat::setUseRealtime,

		"getBuffInfo", &BuffFloat::getBuffInfo,
		"hasBuffInfo", &BuffFloat::hasBuffInfo,

		"getBaseValue", &BuffFloat::getBaseValue,
		"getBuffedValue", &BuffFloat::getBuffedValue,

		"clear", &BuffFloat::clear

		);


	lua.new_usertype<BuffVec4::BuffInfo>("BuffVec4Info",
		"buffType", sol::readonly(&BuffVec4::BuffInfo::buffType),

		"buffAmount", sol::readonly(&BuffVec4::BuffInfo::buffAmount),
		"duration", sol::readonly(&BuffVec4::BuffInfo::duration),
		"fadeInDuration", sol::readonly(&BuffVec4::BuffInfo::fadeInDuration),
		"fadeOutDuration", sol::readonly(&BuffVec4::BuffInfo::fadeOutDuration),

		"restart", &BuffVec4::BuffInfo::restart,

		"removeAfterEnds", &BuffVec4::BuffInfo::removeAfterEnds,

		"setBuffAmount", &BuffVec4::BuffInfo::setBuffAmount,
		"setBuffDuration", &BuffVec4::BuffInfo::setBuffDuration,
		"getCurrentBuffAmount", &BuffVec4::BuffInfo::getCurrentBuffAmount,
		"setBuffFadeInFadeOut", &BuffVec4::BuffInfo::setBuffFadeInFadeOut

		);

	lua.new_usertype<BuffVec4>("BuffVec4",
		"removeBuff", &BuffVec4::removeBuff,
		"addBuffWithType", &BuffVec4::addBuffWithType,
		"addBuff", [&](BuffVec4& buff, Vec4 amount)
	{
		int id = bloodworks->getUniqueId();
		buff.addBuff(id, amount, BuffVec4::multiply_buff);
		return id;
	},

		"addBuffWithId", [&](BuffVec4& buff, int id, Vec4 amount)
	{
		buff.addBuff(id, amount, BuffVec4::multiply_buff);
		return id;
	},
		"setBaseValue", &BuffVec4::setBaseValue,
		"setBuffAmount", &BuffVec4::setBuffAmount,
		"setBuffDuration", &BuffVec4::setBuffDuration,
		"setBuffFadeInFadeOut", &BuffVec4::setBuffFadeInFadeOut,
		"getBuffRemainingTime", &BuffVec4::getBuffRemainingTime,

		"setUseRealtime", &BuffVec4::setUseRealtime,

		"getBuffInfo", &BuffVec4::getBuffInfo,
		"hasBuffInfo", &BuffVec4::hasBuffInfo,

		"getBaseValue", &BuffVec4::getBaseValue,
		"getBuffedValue", &BuffVec4::getBuffedValue,

		"clear", &BuffVec4::clear

		);


	lua.set_function("hasCheats",
		[&]()
	{
		return bloodworks->hasCheats();
	});

	lua.set_function("addCircleCollider",
		[&](const Vec2& pos, float radius)
	{
		return bloodworks->getCollisionController()->addCollider(Circle(pos, radius), nullptr);
	});
	lua.set_function("addCapsuleCollider",
		[&](const Vec2& pos0, const Vec2& pos1, float radius)
	{
		return bloodworks->getCollisionController()->addCollider(Capsule(pos0, pos1, radius), nullptr);
	});
	lua.set_function("addRectCollider",
		[&](const Vec2& pos, const Vec2& size, float rotation, float radius)
	{
		return bloodworks->getCollisionController()->addCollider(Rect(pos, size, rotation, radius), nullptr);
	});

	lua.set_function("relocateCircleCollider",
		[&](int index, const Vec2& pos, float radius)
	{
		return bloodworks->getCollisionController()->relocateCollider(index, Circle(pos, radius));
	});
	lua.set_function("relocateCapsuleCollider",
		[&](int index, const Vec2& pos0, const Vec2& pos1, float radius)
	{
		return bloodworks->getCollisionController()->relocateCollider(index, Capsule(pos0, pos1, radius));
	});
	lua.set_function("relocateRectCollider",
		[&](int index, const Vec2& pos, const Vec2& size, float rotation, float radius)
	{
		return bloodworks->getCollisionController()->relocateCollider(index, Rect(pos, size, rotation, radius));
	});

	lua.set_function("removeCollider",
		[&](int index)
	{
		bloodworks->getCollisionController()->removeCollider(index);
	});

	lua.set_function("setColliderFlag",
		[&](int index, int flag, bool isSet)
	{
		bloodworks->getCollisionController()->setFlag(index, flag, true);
	});
	lua.set_function("setColliderFlags",
		[&](int index, int flags)
	{
		bloodworks->getCollisionController()->setFlags(index, flags);
	});
	lua.set_function("clearColliderFlag",
		[&](int index, int flag, bool isSet)
	{
		bloodworks->getCollisionController()->setFlag(index, flag, false);
	});
	lua.set_function("hasColliderFlag",
		[&](int index, int flag, bool isSet)
	{
		return bloodworks->getCollisionController()->hasFlag(index, flag);
	});
	lua.set_function("getColliderFlags",
		[&](int index, int flag, bool isSet)
	{
		return bloodworks->getCollisionController()->getFlags(index);
	});

	lua.set_function("hasAchievement",
		[&](const std::string& achievement)
	{
		return bloodworks->getSteam()->hasAchievement(achievement);
	});

	lua.set_function("addAchievement",
		[&](const std::string& achievement)
	{
		return bloodworks->getSteam()->addAchievement(achievement);
	});

	lua.set_function("clearBloods",
		[&]()
	{
		return bloodworks->getBloodRenderable()->reset();
	});

	lua.set_function("numActivePerks",
		[&]()
	{
		return (int)bloodworks->getActivePerks().size();
	});
	lua.set_function("getActivePerk",
		[&](int index)
	{
		return bloodworks->getActivePerks()[index];
	});

	lua.set_function("getActivePerk",
		[&](int index)
	{
		return bloodworks->getActivePerks()[index];
	});

	lua.set_function("setDropInterval",
		[&](float interval)
	{
		bloodworks->getDropController()->setDropInterval(interval);
	});

	lua.set_function("setMonsterDropInterval",
		[&](float interval)
	{
		bloodworks->getDropController()->setMonsterDropInterval(interval);
	});


	lua.set_function("addCustomTick",
		[&](sol::function& func)
	{
		bloodworks->getMissionController()->addCustomTick(func);
	});
}

