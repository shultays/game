#include "BloodworksCheats.h"
#include "cGlobals.h"
#include "Bloodworks.h"
#include "MonsterController.h"
#include "cFont.h"
#include "Coral.h"
#include "BloodworksConfig.h"
#include "cSoundManager.h"
#include "Perk.h"
#include "Bonus.h"
#include "Monster.h"
#include "Player.h"
#include "DropController.h"
#include "Bloodrenderable.h"
#include "BulletController.h"
#include "Gun.h"
#include "MissionController.h"
#include <sstream>

#ifdef HAS_BLOODWORKS_CHEATS

BloodworksCheats* BloodworksCheats::instance = nullptr;

BloodworksCheats::BloodworksCheats(Bloodworks *bloodworks)
{
	this->bloodworks = bloodworks;
	instance = this;
	tickCount = 0;

	lastSetTickTime = lastSetRenderTime = 0.0f;
	tickCount = renderCount = 0;
	moveMonsters = true;
	slowdownBuff = bloodworks->getGlobalUniqueId();
#ifdef DEBUG
	showFPS = true;
#else
	showFPS = coral.isDebuggerPresent();
#endif
}

void BloodworksCheats::onTick()
{
	const std::vector<Perk*>& perks = bloodworks->getPerks();
	const std::vector<Bonus*>& bonuses = bloodworks->getBonuses();
	const std::vector<Gun*>& guns = bloodworks->getGuns();
	Player *player = bloodworks->getPlayer();
	DropController *dropController = bloodworks->getDropController();
	MonsterController *monsterController = bloodworks->getMonsterController();
	BulletController *bulletController = bloodworks->getBulletController();
	MissionController *missionController = bloodworks->getMissionController();

	if (input.isKeyPressed(key_n))
	{
		showFPS = !showFPS;
		if (showFPS == false)
		{
			debugRenderer.removeText(0);
			debugRenderer.removeText(1);
		}
	}

	if (showFPS)
	{
		tickCount++;
		if (timer.getRealTime() - lastSetTickTime > 1.0f)
		{
			lastSetTickTime += 1.0f;
			std::stringstream ss;
			ss << "FPS " << tickCount << " Monster " << bloodworks->getMonsterController()->getMonsterCount();
			debugRenderer.addText(0, ss.str(), 5.0f, -24.0f, FLT_MAX, Vec4(1.0f), 24.0f, TextAlignment::left, RenderableAlignment::topLeft);

			tickCount = 0;
		}
	}

	if (input.isKeyDown(key_num_minus) || input.isKeyDown(key_num_plus))
	{
		float globalVolume = coral.getSoundManager()->getGlobalVolume();
		if (input.isKeyDown(key_num_plus))
		{
			globalVolume += timer.getNonSlowedDt();
		}
		else
		{
			globalVolume -= timer.getNonSlowedDt();
		}
		clamp(globalVolume, 0.0f, 1.0f);
		coral.getSoundManager()->setGlobalVolume(globalVolume);
		bloodworks->getConfig()->setVolume(globalVolume);
	}

	if (input.isKeyPressed(key_1))
	{
		moveMonsters = !moveMonsters;
	}

	if (input.isKeyPressed(key_2))
	{
		for (int i = 0; i < perks.size(); i++)
		{
			if (perks[i]->isTakenFully() == false && perks[i]->getScriptName() == "Scavenger")
			{
				perks[i]->takeLevel();
				bloodworks->onPerkUsed(perks[i]);
				break;
			}
		}
	}

	if (input.isKeyPressed(key_3))
	{
		std::string s = "CircleFire";

		for (int i = 0; i < bonuses.size(); i++)
		{
			if (bonuses[i]->getScriptName() == s)
			{
				bonuses[i]->spawnAt(player->getPosition());
				break;
			}
		}
	}

	if (input.isKeyPressed(key_4))
	{
		for (int i = 0; i < guns.size(); i++)
		{
			dropController->spawnGun(player->getPosition() + Vec2(-100, i * 50.0f - guns.size() * 25.0f), i);
		}

		for (int i = 0; i < bonuses.size(); i++)
		{
			dropController->spawnBonus(player->getPosition() + Vec2(100, i * 50.0f - bonuses.size() * 25.0f), i);
		}
	}

	if (input.isKeyPressed(key_5) && bloodworks->isLevelUpPopupVisible() == false)
	{
		player->doLevelup();
	}

	if (input.isKeyPressed(key_6))
	{
		sol::table t = lua.create_table();
		t["notNearPlayer"] = true;
		for (int i = 0; i < 100; i++)
		{
			Vec2 v;
			do
			{
				v.x = bloodworks->getMapMin().x + randFloat(bloodworks->getMapSize().x);
				v.y = bloodworks->getMapMin().y + randFloat(bloodworks->getMapSize().y);
			} while (v.distanceSquared(bloodworks->getPlayer()->getPosition()) < 20.0f);
			dropController->spawnDrop(v);
		}
	}

	if (input.isKeyPressed(key_7))
	{
		Vec2 dir = Vec2::fromAngle(randFloat(pi_2));
		float damage = randFloat(10);
		bloodworks->getBloodRenderable()->addBlood(player->getPosition(), dir * clamped(damage * 2.7f, 0.0f, 20.0f), 10.0f + randFloat(10.0f));

	}
	if (input.isKeyPressed(key_8) || input.isKeyPressed(key_9))
	{
		Vec2 dir = Vec2::fromAngle(randFloat(pi_2));
		dir = dir * randFloat(5.0f, 20.0f);

		for (int i=0; i<monsterController->getMonsterCount(); i++)
		{
			Monster *m = monsterController->getMonsterAtIndex(i);
			if (m->getDebug())
			{
				input.isKeyPressed(key_8) ? m->spawnBits(dir, 3) : m->spawnBodyParts(dir);
			}
		}
	}

	if (input.isKeyPressed(key_z))
	{
		bloodworks->getMissionController()->getGameSpeedMultiplierBuff().addBuff(slowdownBuff, 0.1f, BuffFloat::multiply_buff);
	}
	if (input.isKeyPressed(key_x))
	{
		bloodworks->getMissionController()->getGameSpeedMultiplierBuff().removeBuff(slowdownBuff);
	}
	if (input.isKeyPressed(key_c))
	{
		bloodworks->getBloodRenderable()->reset();
	}


	if (input.isKeyDown(key_f1))
	{
		bulletController->drawDebug();
	}
	if (input.isKeyDown(key_f2))
	{
		monsterController->drawDebug();
	}

	if (input.isKeyPressed(key_g))
	{
		if (player->getMaxHitpoints() < 100000000)
		{
			player->setMaxHitpoints(100000000);
			player->setHitpoints(100000000);
		}
		else
		{
			player->setMaxHitpoints(100);
			player->setHitpoints(100);
		}
	}

	if (input.isKeyPressed(key_q))
	{
		player->killSelf();
	}

	missionController->onDebugTick();
}

void BloodworksCheats::onMonsterTick(Monster *monster)
{
	if (moveMonsters == false)
	{
		monster->setPosition(monster->getPreviousPosition());
	}

	if (monster->getDebug())
	{
		if (input.isKeyDown(key_f2) || moveMonsters == false || monster->getDebug() != -1)
		{
			debugRenderer.addCircle(monster->getPosition(), monster->getRadius(), 0.0f, Vec4::fromColor(monster->getDebug() == -1 ? 0xFFFFFF00 : 0xFF00FFFF));
		}
	}
}

void BloodworksCheats::onRender()
{
	if (showFPS)
	{
		renderCount++;
		if (timer.getRealTime() - lastSetRenderTime > 1.0f)
		{
			lastSetRenderTime += 1.0f;
			std::stringstream ss;
			ss << "Render " << renderCount;
			debugRenderer.addText(1, ss.str(), 5.0f, -24.0f * 2.0f, FLT_MAX, Vec4(1.0f), 24.0f, TextAlignment::left, RenderableAlignment::topLeft);

			renderCount = 0;
		}
	}
}

void BloodworksCheats::onLoadMission()
{
	const std::vector<Gun*>& guns = bloodworks->getGuns();
	Player *player = bloodworks->getPlayer();

	for (auto& gun : guns)
	{
		if (gun->getScriptName() == "Laser")
		{
			//player->setGun(gun);
		}

		if (gun->getScriptName() == "RocketBarrage")
		{
			player->setSecondaryGun(gun);
		}
		if (gun->getScriptName() == "BigLaser")
		{
			//player->setSecondaryGun(gun);
		}
	}
}

void BloodworksCheats::onMonsterPreTick(Monster* monster)
{
	if (monster->getDebug())
	{
		if (input.isKeyPressed(key_f3))
		{
			monster->setDebug(1);
		}
		if (input.isKeyPressed(key_f4))
		{
			monster->setDebug(0);
		}
		if (input.isKeyPressed(key_f5))
		{
			doBreak();
		}
	}
}

void BloodworksCheats::onInit()
{
	const bool testGame = true;
	if (coral.isDebuggerPresent())
	{
		coral.setFullScreen(false);
	}
	if (coral.isDebuggerPresent() && testGame)
	{
		coral.getSoundManager()->setGlobalVolume(0.0f);
		bloodworks->loadMission("Survival");
	}
}

#endif