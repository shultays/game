#pragma once

#include "cGame.h"
#include "cResources.h"
#include "cSound.h"

enum Depths
{
	BACKGROUND = -1000,
	MONSTERS = 1000,
	PLAYER = 2000,
	GAME_OBJECTS = 3000,
	BULLETS = 4000,
	OBJECT_GUI = 5000,
	FOREGROUND = 6000,
	GUI = 7000
};

class cParticle;
class cParticleTemplate;
class cTexturedQuadRenderable;
class Player;
class Monster;
class Bullet;
class Gun;
class MissionController;
class BloodRenderable;
class Bonus;
class Perk;
class cPostProcess;
class cTextRenderable;
class LevelUpPopup;
class ExplosionController;
class DropController;
class MissionController;
class MonsterController;
class BulletController;
class BloodworksLuaWorld;
class LaserTemplate;
class MainMenu;
class OneShotSoundManager;
class OptionsPopup;
class cPersistent;
class ModWindow;
class cSlaveWork;
class BloodworksConfig;
class BloodworksCheats;

class Bloodworks : public cGame
{
	Player *player;

	MissionController *missionController;
	MonsterController *monsterController;
	BulletController *bulletController;
	ExplosionController *explosionController;
	DropController *dropController;
	BloodworksLuaWorld *luaWorld;
	OneShotSoundManager *oneShotSoundManager;

	BloodRenderable *bloodRenderable;
	cPostProcess *pausePostProcess;
	LevelUpPopup *levelUpPopup;
	OptionsPopup *optionsPopup;
	ModWindow *modWindow;

	std::vector<Gun*> guns;
	std::vector<Bonus*> bonuses;
	std::vector<Bonus*> activeBonuses;
	std::vector<Perk*> perks;
	std::vector<Perk*> usedPerks;

	cTexturedQuadRenderable *bg;
	std::vector<cTexturedQuadRenderable*> fgs;

	std::unordered_map<std::string, cParticleTemplate*> particles;
	std::unordered_map<std::string, LaserTemplate*> laserTemplates;

	BloodworksConfig* config;

	int nextUniqueId;
	int nextGlobalUniqueId;

	Vec2 mapSize;
	Vec2 mapBegin;
	Vec2 mapEnd;

	Vec2 cameraCenterPos;

	float pauseSlowdown;

	bool paused;

	MainMenu *mainMenu;

	float soundSpeed;
	bool soundPaused;

	std::vector<cSoundHandle> gameSounds;
	std::vector<cParticle*> orphanParticles;
protected:
	virtual void render() override;
	virtual void tick() override;
	virtual void init() override;

	void tickCamera();
	void tickGameSlowdown();
public:
	Bloodworks();
	virtual ~Bloodworks();

	MonsterController* getMonsterController() const
	{
		return monsterController;
	}

	DropController* getDropController() const
	{
		return dropController;
	}

	BulletController* getBulletController() const
	{
		return bulletController;
	}

	MissionController* getMissionController() const
	{
		return missionController;
	}

	const std::vector<Gun*>& Bloodworks::getGuns() const
	{
		return guns;
	}

	const std::vector<Bonus*>& Bloodworks::getBonuses() const
	{
		return bonuses;
	}

	const std::vector<Perk*>& Bloodworks::getPerks() const
	{
		return perks;
	}

	Player* getPlayer() const
	{
		return player;
	}

	BloodRenderable* getBloodRenderable();

	cParticleTemplate* getParticleTemplate(const std::string& name) const
	{
		return particles.at(name);
	}

	void addDrop(const Vec2& position);
	void addExplosion(const Vec2& pos, float maxScale, float scaleSpeed, int minDamage, int maxDamage);

	int getUniqueId()
	{
		if (nextUniqueId < 1000)
		{
			nextUniqueId = 1000;
		}
		return nextUniqueId++;
	}	
	
	int getGlobalUniqueId()
	{
		assert(nextGlobalUniqueId < 1000);
		return nextGlobalUniqueId++;
	}

	const Vec2& getMapSize() const
	{
		return mapSize;
	}
	const Vec2& getMapMin() const
	{
		return mapBegin;
	}
	const Vec2& getMapMax() const
	{
		return mapEnd;
	}
	bool isCoorOutside(const Vec2& pos, float radius) const;
	bool isCoorOutside(const Vec2& pos) const;

	void onAddedGunBullet(Gun *gun, Bullet *bullet);

	float getPauseSlowdown() const
	{
		return pauseSlowdown;
	}

	void openLevelupPopup();
	void doPause();
	void doUnpause();
	std::vector<Perk*> getAvailablePerks() const;
	void onPerkUsed(Perk *levelupPerks);
	int onPlayerDamaged(int damage, float dir, sol::table& params);
	void addLaserTemplate(LaserTemplate * laserTemplate);
	bool isLevelUpPopupVisible() const;
	bool isPaused() const
	{
		return paused;
	}

	BloodworksConfig* getConfig()
	{
		return config;
	}

	virtual void windowResized(int width, int height);
	void clearMission();
	bool gotoMainMenu();
	bool loadMission(const std::string& mission);
	void onPlayerDied();
	void playSoundAtMap(const Vec2& pos, cSoundSampleShr s, float volume = 1.0f);
	void playSoundAtMap(const Vec2& pos, cSoundSampleWithParams s);
	void playOneShotSound(sol::table& args);
	float getVolumeMultiplier(const Vec2& pos) const;
	void addGameSound(cSoundHandle& handle);
	void setSoundSpeed(float newSoundSpeed);
	float getSoundSpeed() const;
	bool isMissionLoaded() const;
	void onGunReloaded(Gun* gun);
	void onMonsterDied(Monster* monster, float dropChance);
	void showOptions();

	void addOrphanParticle(const std::vector<cParticle*> particles)
	{
		orphanParticles.insert(orphanParticles.end(), particles.begin(), particles.end());
	}	

	void addOrphanParticle(cParticle* particle)
	{
		orphanParticles.push_back(particle);
	}
	bool isOptionsVisible() const;
	void onLevelUp();
	void addSlaveWork(cSlaveWork* work);
	void showMods();
	void loadMod(const std::string& path);
	void updateVolume();
	void updateMusicVolume();
	void addToActiveBonuses(Bonus* bonus);
	void removeFromActiveBonuses(Bonus* bonus);
	void onPlayerPickedGun(Gun * gun);
	void onPlayerPickedBonus(Bonus * bonus, const Vec2& pos);
	void onMonsterDamaged(Monster* monster, int damage, const Vec2& dir, sol::table& args);
};