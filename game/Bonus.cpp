#include "Bonus.h"

#include "cTools.h"
#include "cGlobals.h"

Bonus::Bonus(nlohmann::json& j)
{
	name = j["name"].get<std::string>();
	iconPath = j["iconFile"].get<std::string>();
	scriptName = j["scriptName"].get<std::string>();
	std::string scriptFile = j["scriptFile"].get<std::string>();

	lua[scriptName] = lua.create_table();
	lua.script_file(scriptFile);
}

void Bonus::spawnAt(const Vec2& pos)
{
	lua[scriptName]["spawn"](pos);
}

const std::string& Bonus::getIconPath() const
{
	return iconPath;
}

const std::string& Bonus::getName() const
{
	return name;
}
