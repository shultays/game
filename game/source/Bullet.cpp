#include "Bullet.h"
#include "cRenderable.h"
#include "cRenderableContainer.h"
#include "cTexturedQuadRenderable.h"
#include "Monster.h"
#include "Bloodworks.h"
#include "cTexture.h"
#include "Gun.h"
#include "cParticle.h"
#include "MonsterController.h"
#include "BulletController.h"
#include "Player.h"
#include "CollisionController.h"
#include "cCircle.h"

Bullet::Bullet(Bloodworks *bloodworks, Gun *gun)
{
	this->bloodworks = bloodworks;
	this->gun = gun;
	renderable = new cRenderableContainer(bloodworks);
	bloodworks->addRenderable(renderable, BULLETS);
	isDead = false;
	id = bloodworks->getUniqueId();
	damage = 10;
	penetrateCount = 0;
	penetrateUsed = 0;
	scale = 1.0f;
	meshScale = Vec2(1.0f);
	data = lua.create_table();
	onDamageArgs = lua.create_table();
	onDamageArgs["bullet"] = this;
	onDamageArgs["gun"] = gun;
	meshRotation = -1500;
	monsterBullet = false;
	hasCollision = true;
	if (gun)
	{
		lifeTime = gun->getBulletLifeTime();
	}
	else
	{
		lifeTime = -1.0f;
	}
	startTime = timer.getTime();

	particleArgs = lua.create_table();
	particleArgs["bullet"] = this;
}

Bullet::~Bullet()
{
	SAFE_DELETE(renderable);
	for (int i = 0; i < particles.size(); i++)
	{
		bloodworks->addOrphanParticle(particles[i].particle);
	}
	particles.clear();
}

void Bullet::tick()
{
	float dt = timer.getDt();
	Vec2 oldPos = pos;
	if (onTickCallback)
	{
		onTickCallback(this);
	}

	if (gun != nullptr)
	{
		if (gun->getScriptTable()["onBulletTick"])
		{
			gun->getScriptTable()["onBulletTick"](gun, this);
		}
	}
	moveDir = Vec2::fromAngle(moveAngle);
	moveVelocity = moveDir * moveSpeed * bloodworks->getPlayer()->getBulletSpeedMultiplier();

	pos += moveVelocity * dt;
	clampPos();
	updateDrawable();

	if (bloodworks->isCoorOutside(pos, -20.0f) || (lifeTime > 0.0f && timer.getTime() - startTime > lifeTime))
	{
		onHit(nullptr);
		removeSelf();
	}

	if (hasCollision && bloodworks->getCollisionController()->hasCollision(Circle(pos, radius), CollisionController::no_bullet_collision))
	{
		onHit(nullptr);
		removeSelf();
	}

	if (isDead)
	{
		return;
	}

	for (auto& particleData : particles)
	{
		Vec2 finalPos = pos + (particleData.spawnShift * Mat2::rotation(-getMeshRotation() + pi_d2));
		Vec2 toFinalPos = finalPos - particleData.lastSpawnPos;
		float distanceToFinal = toFinalPos.normalize();
		while (distanceToFinal > particleData.spawnDistance)
		{
			distanceToFinal -= particleData.spawnDistance;

			particleData.lastSpawnPos += toFinalPos * particleData.spawnDistance;

			if (particleData.particle->getParticleTemplate()->needsLuaCall())
			{
				particleData.particle->addParticle(particleData.lastSpawnPos, particleArgs);
			}
			else
			{
				particleData.particle->addParticleInternal(particleData.lastSpawnPos, nullptr, nullptr);
			}
		}

	}
	
	if (monsterBullet)
	{
		float checkRadius = radius + bloodworks->getPlayer()->getBulletRadius();
		if (bloodworks->getPlayer()->getPosition().distanceSquared(pos) < checkRadius * checkRadius)
		{
			bloodworks->getPlayer()->doDamageWithArgs(damage, moveAngle, onDamageArgs);
			onHit(nullptr);
			removeSelf();
		}
	}
	else
	{
		std::function<bool(Monster *monster)> func = [&](Monster *monster) -> bool
		{
			Vec2 monsterPos = monster->getPosition();
			float radiusToCheck = monster->getRadius() + radius * scale;

			if (monster->isRemoved() == false && pos.distanceSquared(monsterPos) < radiusToCheck * radiusToCheck && (penetrateCount == 0 || monster->hasIgnoreId(id) == false))
			{
				if (monster->shouldHit(this) == false)
				{
					return true;
				}
				if (shouldHitMonsterTest)
				{
					if (shouldHitMonsterTest(this, monster))
					{
						return true;
					}
				}

				penetrateUsed++;

				if (penetrateUsed > penetrateCount)
				{
					removeSelf();
				}
				else
				{
					monster->addIgnoreId(id);
				}

				monster->doDamageWithArgs(damage, moveDir, onDamageArgs);
				onHit(monster);
				return false;
			}
			return true;
		};
		bloodworks->getMonsterController()->runForEachMonsterInRadius(pos, radius * scale, func);
	}
}

void Bullet::addRenderable(cRenderable *renderable)
{
	this->renderable->addRenderable(renderable);
}

void Bullet::updateDrawable()
{
	Mat3 mat = Mat3::identity();
	mat.scaleBy(meshScale * scale);
	mat.rotateBy(-getMeshRotation());
	mat.translateBy(pos);
	renderable->setWorldMatrix(mat);
}


void Bullet::removeSelf()
{
	isDead = true;
	renderable->setVisible(false);
}

float Bullet::getMeshRotation()
{
	return (meshRotation > -1000 ? meshRotation : moveAngle);
}

void Bullet::setColor(const Vec4& color)
{
	renderable->setColor(color);
}

Gun* Bullet::getGun() const
{
	return gun;
}

void Bullet::onHit(Monster* monster)
{
	if (onHitCallback)
	{
		onHitCallback(this, monster);
	}
	if (gun)
	{
		gun->onBulletHit(this, monster);
	}
}

void Bullet::clampPos()
{
	AARect rect = bloodworks->getMapLimits();
	rect.addThreshold(100.0f);
	rect.clampPos(pos);
}

void Bullet::addRenderableTextureWithPosAndSize(const std::string& texture, const Vec2& pos, const Vec2& dimensions)
{
	cTexturedQuadRenderable* quad = new cTexturedQuadRenderable(bloodworks, texture, "resources/default");
	quad->setWorldMatrix(Mat3::scaleMatrix(dimensions.isNonZero() ? dimensions : quad->getTexture()->getDimensions().toVec()).translateBy(pos));
	addRenderable(quad);
	updateDrawable();
}

cParticle* Bullet::addTrailParticle(const std::string& name, const Vec2& shift, float spawnDistance, const sol::table& args)
{
	Particledata particleData;
	particleData.particle = new cParticle(bloodworks, bloodworks->getParticleTemplate(name), args);
	particleData.spawnShift = shift;
	particleData.lastSpawnPos = pos;
	particleData.spawnDistance = spawnDistance;
	bloodworks->addRenderable(particleData.particle, renderable->getLevel() - 1);
	particles.push_back(particleData);
	return particleData.particle;
}

void Bullet::modifyDrawLevel(int level)
{
	renderable->setLevel(renderable->getLevel() + level);
}

void Bullet::setPosition(const Vec2& pos)
{
	if (this->pos.distanceSquared(pos) > 10.0f * 10.0f)
	{
		for (auto& particle : particles)
		{
			particle.lastSpawnPos = pos;
		}
	}

	this->pos = pos;
	bloodworks->getBulletController()->relocateBullet(this);

	clampPos();
}

void Bullet::setScript(sol::table& script)
{
	this->script = script;

	onHitCallback = script["onHit"];
	onTickCallback = script["onTick"];
	shouldHitMonsterTest = script["shouldHitMonsterTest"];
}

void Bullet::addRenderableTextureWithSize(const std::string& texture, const Vec2& dimensions)
{
	addRenderableTextureWithPosAndSize(texture, Vec2::zero(), dimensions);
}

void Bullet::addRenderableTexture(const std::string& texture)
{
	addRenderableTextureWithPosAndSize(texture, Vec2::zero(), Vec2::zero());
}