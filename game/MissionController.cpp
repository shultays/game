#include "MissionController.h"
#include "json.h"
#include "cTools.h"
#include "cGlobals.h"
#include "cRenderable.h"
#include "cTexture.h"
#include "cFont.h"
#include "Bloodworks.h"
#include "Player.h"

using json = nlohmann::json;

int MissionController::nextGameObjectId = 0;

void MissionController::loadMissionController(const std::string& missionControllerData)
{
	lua["gameObjects"] = lua.create_table();

	this->bloodworks = bloodworks;
	std::string jsonFile;
	textFileRead(missionControllerData.c_str(), jsonFile);
	json j = json::parse(jsonFile.c_str());

	name = j["name"].get<std::string>();

	scriptTable = lua[j["scriptName"].get<std::string>()] = lua.create_table();
	scriptPath = j["scriptFile"].get<std::string>();
	lua.script_file(scriptPath);
	scriptTable["init"]();
}

void MissionController::clear()
{
	scriptTable["clear"]();


	for (auto& g : gameObjects)
	{
		auto& gameObject = g.second;

		for (auto& r : gameObject.renderables)
		{
			SAFE_DELETE(r.renderable);
		}
	}
	gameObjects.clear();

	lua["gameObjects"] = lua.create_table();
}

void MissionController::init(Bloodworks *bloodworks)
{
	this->bloodworks = bloodworks;

	lua.set_function("addGameObject",
		[&](const std::string& name, const std::string& script, const sol::table& initArgs) -> int
	{
		return addGameObject(name, script, initArgs);
	});

	lua.set_function("addTextureToGameObject",
		[&](int gameObject, const sol::table& params, const std::string& texture, const std::string& shader) -> int
	{
		return addTextureToGameObject(gameObject, params, texture, shader);
	});

	lua.set_function("addTextToGameObject",
		[&](int gameObject, const sol::table& params, const std::string& text, const std::string& font) -> int
	{
		return addTextToGameObject(gameObject, params, text, font);
	});

	lua.set_function("updateRenderableParams",
		[&](int gameObject, int renderableId, const sol::table& params)
	{
		updateRenderableParams(gameObject, renderableId, params);
	});

	lua.set_function("removeGameObject",
		[&](int gameObject)
	{
		removeGameObject(gameObject);
	});


	lua.set_function("addCustomBullet",
		[&](const sol::table& params) -> int
	{
		return addCustomBullet(params);
	});
}

void MissionController::tick(float dt)
{
	scriptTable["onTick"]();

	std::vector<int> toBeRemoved;
	for (auto& g : gameObjects)
	{
		auto& gameObject = g.second;
		if (gameObject.hasOnTick)
		{
			lua[gameObject.script]["onTick"](gameObject.id);
			if (g.second.gameObject["toBeRemoved"])
			{
				toBeRemoved.push_back(g.first);
			}
		}
	}

	for (auto& toRemove : toBeRemoved)
	{
		removeGameObject(toRemove);
	}
}

int MissionController::addGameObject(const std::string& name, const std::string& script, const sol::table& initArgs)
{
	int id = nextGameObjectId++;
	sol::table gameObject = lua["gameObjects"][id] = lua.create_table();
	gameObject["name"] = name;
	gameObject["args"] = initArgs;
	gameObject["script"] = script;
	gameObject["id"] = id;


	GameObject object;
	object.gameObject = gameObject;
	object.id = id;
	object.hasOnTick = lua[script]["onTick"];
	object.hasBulletCollision = lua[script]["onBulletCollision"];
	object.hasBulletCollision = lua[script]["onPlayerCollision"];
	object.hasBulletCollision = lua[script]["onMonsterCollision"];
	object.script = script;
	gameObjects[id] = object;

	lua[script]["init"](id);
	return id;
}

void MissionController::removeGameObject(int id)
{
	lua["gameObjects"][id] = nullptr;
	auto& gameObject = gameObjects[id];

	for (auto& r : gameObject.renderables)
	{
		SAFE_DELETE(r.renderable);
	}
	gameObjects.erase(id);
}

int MissionController::addTextureToGameObject(int gameObject, const sol::table& params, const std::string& texture, const std::string& shader)
{
	RenderableData renderableData;

	renderableData.type = RenderableDataType::texture;
	renderableData.pos = Vec2::zero();
	renderableData.scale = Vec2(1.0f);
	renderableData.rotation = 0.0f;

	cTexturedQuadRenderable *renderable = new cTexturedQuadRenderable((cGame*)bloodworks, texture.c_str(), shader.length() > 0 ? shader.c_str() : "resources/default");
	renderableData.renderable = renderable;
	bloodworks->addRenderable(renderable, 600);

	gameObjects[gameObject].renderables.push_back(renderableData);

	updateRenderableParams(gameObject, (int)gameObjects[gameObject].renderables.size() - 1, params);

	return (int)gameObjects[gameObject].renderables.size() - 1;
}

int MissionController::addTextToGameObject(int gameObject, const sol::table& params, const std::string& text, const std::string& font)
{
	RenderableData renderableData;

	renderableData.type = RenderableDataType::text;
	renderableData.pos = Vec2::zero();
	renderableData.scale = Vec2(1.0f);
	renderableData.rotation = 0.0f;

	cTextRenderable *renderable = new cTextRenderable((cGame*)bloodworks, resources.getFont(font.size() ? font.c_str() : "resources/fontData.txt"), text);
	renderableData.renderable = renderable;
	bloodworks->addRenderable(renderable, 600);

	gameObjects[gameObject].renderables.push_back(renderableData);

	updateRenderableParams(gameObject, (int)gameObjects[gameObject].renderables.size() - 1, params);

	return (int)gameObjects[gameObject].renderables.size() - 1;
}

void MissionController::updateRenderableParams(int gameObject, int renderableId, const sol::table& params)
{
	if (renderableId == -1)
	{
		auto& g = gameObjects[gameObject];
		for (int i = 0; i < g.renderables.size(); i++)
		{
			updateRenderableParams(gameObject, i, params);
		}
		return;
	}

	auto &renderableData = gameObjects[gameObject].renderables[renderableId];

	if (params["position"])
	{
		renderableData.pos = Vec2(params["position"][1].get<float>(), params["position"][2].get<float>());
	}

	if (renderableData.type == RenderableDataType::texture)
	{
		cTexturedQuadRenderable *quadRenderable = (cTexturedQuadRenderable*)renderableData.renderable;

		if (params["color"])
		{
			quadRenderable->setColor(Vec4::fromColor(params["color"].get<int>()));
		}
		if (params["rotation"])
		{
			renderableData.rotation = params["rotation"][1].get<float>();
		}
		if (params["size"])
		{
			renderableData.scale = Vec2(params["size"][1].get<float>(), params["size"][2].get<float>());
		}

		Mat3 mat = Mat3::scaleMatrix(renderableData.scale).rotateBy(renderableData.rotation).translateBy(renderableData.pos);
		renderableData.renderable->setWorldMatrix(mat);
	}
	else if (renderableData.type == RenderableDataType::text)
	{
		cTextRenderable *textRenderable = (cTextRenderable*)renderableData.renderable;
		textRenderable->setPosition(renderableData.pos);

		if (params["color"])
		{
			textRenderable->setTextColor(Vec4::fromColor(params["color"].get<int>()));
		}
		if (params["textSize"])
		{
			textRenderable->setTextSize(params["textSize"][1].get<float>());
		}
		if (params["alignment"])
		{
			cTextRenderable::Alignment alignment = cTextRenderable::left;
			std::string s = params["alignment"].get<std::string>();
			if (s == "center")
			{
				alignment = cTextRenderable::center;
			}
			else if (s == "right")
			{
				alignment = cTextRenderable::right;
			}
			textRenderable->setAlignment(alignment);
		}
	}
}

int MissionController::addCustomBullet(const sol::table& params)
{
	Vec2 dir = bloodworks->getPlayer()->getAimDir();
	Bullet *bullet = new Bullet(bloodworks, nullptr);
	Player *player = bloodworks->getPlayer();

	bullet->pos = player->getPos() + player->getAimDir() * 20;
	if (params["position"])
	{
		bullet->pos = Vec2(params["position"][1].get<float>(), params["position"][2].get<float>());
	}

	bullet->speed = player->getAimDir() * 20.0f;
	if (params["speed"])
	{
		bullet->speed = Vec2(params["speed"][1].get<float>(), params["speed"][2].get<float>());
	}

	bullet->rotation = bullet->speed.toAngle();
	if (params["rotation"])
	{
		bullet->rotation = params["rotation"].get<float>();
	}

	bullet->radius = 2.0f;
	if (params["radius"])
	{
		bullet->radius = params["radius"].get<float>();
	}

	bullet->damage = 10;
	if (params["damage"])
	{
		bullet->damage = params["damage"].get<int>();
	}

	std::string resource = "resources/basicgun/bullet.png";
	if (params["image"])
	{
		resource = params["image"].get<std::string>();
	}

	std::string shader = "resources/default";
	if (params["shader"])
	{
		shader = params["shader"].get<std::string>();
	}


	cTexturedQuadRenderable *renderable = new cTexturedQuadRenderable(bloodworks, resource.c_str(), shader.c_str());

	Vec2 bulletSize(10.0f);
	if (params["imageSize"])
	{
		bulletSize = Vec2(params["imageSize"][1].get<float>(), params["imageSize"][2].get<float>());
	}
	renderable->setSize(bulletSize);
	bullet->addRenderable(renderable);

	if (params["onHitCallback"])
	{
		bullet->onHitCallback = params["onHitCallback"].get<std::string>();
	}

	if (params["onTickCallback"])
	{
		bullet->onTickCallback = params["onTickCallback"].get<std::string>();
	}

	bullet->init();
	bloodworks->getBulletController()->addBullet(bullet);

	return bullet->getId();
}

