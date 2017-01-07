#include "LevelUpPopup.h"
#include "Bloodworks.h"
#include "cRenderable.h"
#include "cFont.h"
#include "cTexture.h"
#include "cButton.h"
#include "Player.h"
#include "Perk.h"
#include "MissionController.h"
#include "Bloodworks.h"
#include <sstream>

LevelUpPopup::LevelUpPopup(Bloodworks *bloodworks)
{
	this->bloodworks = bloodworks;
	levelupGroup = new cRenderableGroup(bloodworks);
	levelupGroup->setAlignment(RenderableAlignment::center);

	cTexturedQuadRenderable *t = new cTexturedQuadRenderable(bloodworks, "resources/level_up_bg.png", "resources/default");
	t->setWorldMatrix(Mat3::scaleMatrix(t->getTexture()->getDimensions().toVec() * 0.55f));
	levelupGroup->addRenderable(t);

	levelupGroupTitle = new cTextRenderable(bloodworks, resources.getFont("resources/fontData.txt"), "", 24.0f);
	levelupGroupTitle->setWorldMatrix(Mat3::translationMatrix(Vec2(0.0f, 110.0f)));
	levelupGroupTitle->setTextAllignment(TextAlignment::center);
	levelupGroup->addRenderable(levelupGroupTitle);


	currentPerkName = new cTextRenderable(bloodworks, resources.getFont("resources/fontData.txt"), "", 18.0f);
	currentPerkName->setWorldMatrix(Mat3::translationMatrix(Vec2(0.0f, -80.0f)));
	currentPerkName->setTextAllignment(TextAlignment::center);
	levelupGroup->addRenderable(currentPerkName);

	currentPerkExplanation = new cTextRenderable(bloodworks, resources.getFont("resources/fontData.txt"), "", 14.0f, Vec4::fromColor(0xFFAAAAAA));
	currentPerkExplanation->setWorldMatrix(Mat3::translationMatrix(Vec2(0.0f, -110.0f)));
	currentPerkExplanation->setTextAllignment(TextAlignment::center);
	levelupGroup->addRenderable(currentPerkExplanation);

	levelupGroup->setVisible(false);

	bloodworks->addRenderable(levelupGroup, GUI + 150);

	levelUpSound = resources.getSoundSample("resources/sounds/level_up.ogg");
}

LevelUpPopup::~LevelUpPopup()
{
	SAFE_DELETE(levelupGroup);

	for (auto& t : levelupPerksRenderables)
	{
		SAFE_DELETE(t);
	}
	levelupPerksRenderables.clear();
	levelUpSound = nullptr;
}

bool LevelUpPopup::isVisible() const
{
	return levelupGroup->isVisible();
}

void LevelUpPopup::show()
{
	lastMouseMoveTimer = 0.01f;
	input.showMouse();
	input.setMousePosition(bloodworks->getScreenDimensions().w / 2, bloodworks->getScreenDimensions().h / 2 + 70);
	bloodworks->doPause();
	levelupGroup->setVisible(true);
	std::stringstream ss;

	ss << "You are now level " << bloodworks->getPlayer()->getLevel() << "!";

	levelupGroupTitle->setText(ss.str());
	currentPerkName->setText("");
	currentPerkExplanation->setText("");
	hoverLevelupPerkIndex = -1;

	levelupGroup->setColor(Vec4(1.0f, 1.0f, 1.0f, 0.0f));

	auto availablePerks = bloodworks->getAvailablePerks();

	auto& missionData = bloodworks->getMissionController()->getMissionData();
	int selectCount = missionData["perkPerLevel"] ? missionData["perkPerLevel"].get<int>() : 3;

	selectCount = min(selectCount, (int)availablePerks.size());
	levelupPerks.clear();
	while (selectCount-- > 0)
	{
		int r = randInt((int)availablePerks.size());
		levelupPerks.push_back(availablePerks[r]);
		availablePerks[r] = availablePerks[availablePerks.size() - 1];
		availablePerks.resize(availablePerks.size() - 1);
	}
	assert(levelupPerksRenderables.size() == 0);

	float perkDistance = levelupPerks.size() < 5 ? 140.0f : 110.0f;
	for (int i = 0; i < levelupPerks.size(); i++)
	{
		cButton *t = new cButton(bloodworks);
		Vec2 pos = Vec2(-i * perkDistance + (levelupPerks.size() - 1) * perkDistance * 0.5f, 20.0f);
		t->setDefaultMatrix(pos, 1.0f, 0.0f);
		t->setHoverMatrix(pos, 1.25f, 0.0f);
		t->setAlignment(RenderableAlignment::center);
		t->setColor(Vec4(1.0f, 1.0f, 1.0f, 0.0f));
		t->setHitArea(Vec2(-50.0f), Vec2(50.0f));
		t->setHoverSpeed(10.0f);
		t->setSounds(resources.getSoundSample("resources/sounds/click.ogg"), resources.getSoundSample("resources/sounds/hover.ogg"));

		cTexturedQuadRenderable *quad = new cTexturedQuadRenderable(bloodworks, levelupPerks[i]->getIconPath(), "resources/default");
		quad->setWorldMatrix(Mat3::scaleMatrix(40.0f));
		t->addRenderable(quad);
		bloodworks->addRenderable(t, GUI + 151);
		levelupPerksRenderables.push_back(t);
	}

	levelUpSound->play();
}

void LevelUpPopup::tick()
{
	if (!isVisible())
	{
		return;
	}
	float alpha = levelupGroup->getColor().a;
	if (alpha < 1.0f)
	{
		alpha += timer.getNonSlowedDt() * 4.0f;
		alpha = min(1.0f, alpha);
		Vec4 color = Vec4(1.0f, 1.0f, 1.0f, alpha);
		levelupGroup->setColor(color);
		for (int i = 0; i < levelupPerks.size(); i++)
		{
			levelupPerksRenderables[i]->setColor(color);
		}
	}

	if (input.hasJoyStick() == false)
	{
		lastMouseMoveTimer = 1.0f;
	}
	else
	{
		if (input.getDeltaMousePos().lengthSquared() > 0.0f)
		{
			lastMouseMoveTimer = 1.0f;
			for (int i = 0; i < levelupPerks.size(); i++)
			{
				levelupPerksRenderables[i]->setEnforcedHovering(cButton::no_enforce);
			}
		}

		lastMouseMoveTimer -= timer.getNonSlowedDt();

		if (lastMouseMoveTimer < 0.0f && hoverLevelupPerkIndex == -1)
		{
			joyPadFree = false;
			for (int i = 0; i < levelupPerks.size(); i++)
			{
				levelupPerksRenderables[i]->setEnforcedHovering(1 == i ? cButton::enforce_hovering : cButton::enforce_not_hovering);
			}
		}


		if (joyPadFree)
		{
			int indexToSet = hoverLevelupPerkIndex;
			if (input.getJoystickAxisPos().x > 0.5f)
			{
				indexToSet--;
				if (indexToSet < 0)
				{
					indexToSet += (int)levelupPerks.size();
				}
				joyPadFree = false;
			}
			else if (input.getJoystickAxisPos().x < -0.5f)
			{
				indexToSet++;
				if (indexToSet >= levelupPerks.size())
				{
					indexToSet -= (int)levelupPerks.size();
				}
				joyPadFree = false;
			}
			
			if (joyPadFree == false)
			{
				for (int i = 0; i < levelupPerks.size(); i++)
				{
					levelupPerksRenderables[i]->setEnforcedHovering(i == indexToSet ? cButton::enforce_hovering : cButton::enforce_not_hovering);
				}
			}
		}
		else
		{
			if (fabs(input.getJoystickAxisPos().x) < 0.4f)
			{
				joyPadFree = true;
			}
		}
	}
	
	Vec2 mouseScreenPos = input.getMousePos() - bloodworks->getScreenDimensions().toVec() * 0.5f;
	//debugRenderer.addText("O", mouseScreenPos.x, mouseScreenPos.y, 0.0f, Vec4(1.0f), 12.0f, TextAlignment::center, RenderableAlignment::center);
	for (int i = 0; i < levelupPerks.size(); i++)
	{
		levelupPerksRenderables[i]->check(mouseScreenPos);
		if (hoverLevelupPerkIndex != i)
		{
			if (levelupPerksRenderables[i]->isHovering())
			{
				hoverLevelupPerkIndex = i;
				std::stringstream name;
				name << levelupPerks[i]->getName();
				if (levelupPerks[i]->isLevelVisible())
				{
					name << " (Level : " << (levelupPerks[i]->getLevel() + 1) << ")";
				}
				currentPerkName->setText(name.str());
				currentPerkExplanation->setText(levelupPerks[i]->getDescription());
				hoverLevelupPerkIndex = i;
			}
		}
		else if (hoverLevelupPerkIndex == i)
		{
			if (!levelupPerksRenderables[i]->isHovering())
			{
				hoverLevelupPerkIndex = -1;
				currentPerkName->setText("");
				currentPerkExplanation->setText("");
			}
		}

		if (levelupPerksRenderables[i]->isClicked() || input.isKeyPressed(joystick_0_button_a))
		{
			levelupPerks[i]->takeLevel();
			bloodworks->onPerkUsed(levelupPerks[i]);
			input.hideMouse();
			bloodworks->doUnpause();
			levelupGroup->setVisible(false);
			levelupPerks.clear();
			for (auto& t : levelupPerksRenderables)
			{
				SAFE_DELETE(t);
			}
			levelupPerksRenderables.clear();
			return;
		}
	}
}
