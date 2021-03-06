#include "cScrollContainer.h"
#include "cSlider.h"
#include "cGame.h"
#include "cGlobals.h"

cScrollContainer::cScrollContainer(cGame* game) : cRenderable(game)
{
	slider = new cSlider(game, true);
	slider->setValue(1.0f);
	crop = AARect(-100, -100, 100, 100);
	maxScroll = 100.0f;
	content = new cRenderableContainer(game);
}

cScrollContainer::~cScrollContainer()
{
	SAFE_DELETE(slider);
	SAFE_DELETE(content);
}

void cScrollContainer::render(bool isIdentity, const Mat3& mat, const AARect& crop)
{
	if (isVisible() == false)
	{
		return;
	}
	content->render(isIdentity, mat, this->crop);
	slider->setVisible(maxScroll > (this->crop.getMax().y - this->crop.getMin().y));
	slider->render(false, isIdentity ? worldMatrix : worldMatrix * mat, crop);
}

void cScrollContainer::setAlignment(RenderableAlignment alignment)
{
	cRenderable::setAlignment(alignment);
	content->setAlignment(alignment);
	slider->setAlignment(alignment);
}

void cScrollContainer::setColor(const Vec4& color)
{
	cRenderable::setColor(color);
	content->setColor(color);
	slider->setColor(color);
}

void cScrollContainer::check(const Vec2& mousePos)
{
	slider->check(mousePos);
	bool update = slider->isChanged();
	if (input.getMouseWheel() && isMouseInside(mousePos))
	{
		float shift = input.getMouseWheel() * 20.0f / maxScroll;
		slider->setValue(slider->getValue() + shift);
		update = true;
	}
	if (update)
	{
		float val = 1.0f - slider->getFloatValue();
		float t = (maxScroll - (crop.getMax().y - crop.getMin().y));
		if (t > 0.0f)
		{
			content->setWorldMatrix(Mat3::translationMatrix(0.0f, val * t));
		}
	}
}

void cScrollContainer::addRenderable(cRenderable *child)
{
	content->addRenderable(child);
	child->setShader(resources.getShader("resources/defaultWithCrop"));
}

void cScrollContainer::removeRenderable(cRenderable *child)
{
	content->removeRenderable(child);
}

void cScrollContainer::setRect(const AARect& rect)
{
	this->crop = rect;
	Vec2 pos, size;
	pos.x = rect.getMax().x;
	pos.y = rect.getMid().y;
	slider->setWorldMatrix(Mat3::translationMatrix(pos));
	slider->setLength((rect.getMax().y - rect.getMin().y) * 0.5f);
}

void cScrollContainer::setMaxScroll(float maxScroll)
{
	if (maxScroll < 0.0f)
	{
		maxScroll = 0.0f;
	}
	this->maxScroll = maxScroll;
}

const Mat3& cScrollContainer::getScrollMatrix() const
{
	return content->getWorldMatrix();
}

bool cScrollContainer::isMouseInside(const Vec2& mousePos) const
{
	return crop.isInside(game->getRelativeMousePos(mousePos, alignment));
}

void cScrollContainer::setScroll(float scroll)
{
	if (maxScroll > 0.0f)
	{
		float s = 1.0f - scroll / maxScroll;
		slider->setValue(s);
	}
}
