#pragma once

#include <vector>

#include "cMat.h"
#include "cVec.h"
#include "cLuaWorld.h"
#include "cAnimatedRenderable.h"
#include "json.h"

class MonsterTemplate
{
	friend class BloodworksLuaWorld;
	friend class Monster;
	Vec2 size;
	Vec2 textureShift;
	std::string name;
	int hitPoint;
	int experience;
	float collisionRadius;
	float bulletRadius;
	bool hasBlood;
	std::string scriptPath;
	sol::table scriptTable;
	sol::table scriptArgs;

	std::vector<cAnimatedTexturedQuadRenderable::AnimationData> animationData;

	struct BodyPartData
	{
		cTextureShr texture;
		Vec2 shift;
	};
	std::vector<BodyPartData> bodyParts;
	std::vector<cTextureShr> bodyPartBits;

	std::vector<cSoundSampleShr> hitSounds;
	std::vector<cSoundSampleShr> killSounds;
public:
	MonsterTemplate(){}
	MonsterTemplate(nlohmann::json &j)
	{
		name = j["name"].get<std::string>();
		size = Vec2(j["size"].at(0).get<float>(), j["size"].at(1).get<float>());
		textureShift = Vec2(j["textureShift"].at(0).get<float>(), j["textureShift"].at(1).get<float>());
		hitPoint = j["hitPoint"].get<int>();
		experience = j["experience"].get<int>();
		bulletRadius = j["bulletRadius"].get<float>();
		collisionRadius = j["collisionRadius"].get<float>();

		if (j["hasBlood"])
		{
			hasBlood = j["hasBlood"].get<bool>();
		}
		else
		{
			hasBlood = true;
		}

		auto& animations = j["animations"];

		for (nlohmann::json::iterator it = animations.begin(); it != animations.end(); ++it)
		{
			cAnimatedTexturedQuadRenderable::AnimationData data = getAnimationData(it);
			animationData.push_back(data);
		}

		scriptTable = lua[j["scriptName"].get<std::string>()] = lua.create_table();
		scriptPath = j["scriptFile"].get<std::string>();
		lua.script_file(scriptPath);

		auto& parts = j["bodyParts"];

		for (nlohmann::json::iterator it = parts.begin(); it != parts.end(); ++it)
		{
			auto& val = it.value();
			BodyPartData data;
			if (val.is_array())
			{
				data.texture = resources.getTexture((val[0].get<std::string>()));
				data.shift = Vec2(val[1].get<float>(), val[2].get<float>());
			}
			else
			{
				data.texture = resources.getTexture((it.value().get<std::string>()));
				data.shift.setZero();
			}
			bodyParts.push_back(data);
		}

		auto& partBits = j["bodyPartBits"];

		for (nlohmann::json::iterator it = partBits.begin(); it != partBits.end(); ++it)
		{
			bodyPartBits.push_back(resources.getTexture(it.value().get<std::string>()));
		}


		if (j.count("hitSounds"))
		{
			auto& hitSoundsJson = j["hitSounds"];

			for (nlohmann::json::iterator it = hitSoundsJson.begin(); it != hitSoundsJson.end(); ++it)
			{
				hitSounds.push_back(resources.getSoundSample(it.value().get<std::string>()));
			}
		}

		if (j.count("killSounds"))
		{
			auto& killSoundsJson = j["killSounds"];

			for (nlohmann::json::iterator it = killSoundsJson.begin(); it != killSoundsJson.end(); ++it)
			{
				killSounds.push_back(resources.getSoundSample(it.value().get<std::string>()));
			}
		}


		scriptArgs = lua.create_table();
		if (j.count("scriptArgs"))
		{
			auto& scriptArgsJson = j["scriptArgs"];
			for (nlohmann::json::iterator it = scriptArgsJson.begin(); it != scriptArgsJson.end(); ++it)
			{
				switch (it.value().type())
				{
				case nlohmann::json::value_t::boolean:
					scriptArgs[it.key()] = it.value().get<bool>();
					break;
				case nlohmann::json::value_t::number_float:
					scriptArgs[it.key()] = it.value().get<float>();
					break;
				case nlohmann::json::value_t::number_integer:
					scriptArgs[it.key()] = it.value().get<int>();
					break;
				case nlohmann::json::value_t::number_unsigned:
					scriptArgs[it.key()] = it.value().get<unsigned>();
					break;
				case nlohmann::json::value_t::string:
					scriptArgs[it.key()] = it.value().get<std::string>();
					break;
				default:
					break;
				}
			}
		}

	}

	~MonsterTemplate()
	{
		animationData.clear();
		for (auto& bodyPart : bodyParts)
		{
			bodyPart.texture = nullptr;
		}
		bodyParts.clear();
		for (auto& bodyPart : bodyPartBits)
		{
			bodyPart = nullptr;
		}
		bodyPartBits.clear();
		for (auto& hitSound : hitSounds)
		{
			hitSound = nullptr;
		}
		hitSounds.clear();
	}

	const std::string& getName() const
	{
		return name;
	}
};
