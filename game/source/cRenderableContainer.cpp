#include "cRenderableContainer.h"

cRenderableContainer::cRenderableContainer(cGame *game, int initialCapacity /*= 2*/) : cRenderable(game)
{
	renderables.reserve(initialCapacity);
}

void cRenderableContainer::render(bool isIdentity, const Mat3& mat, const AARect& crop)
{
	if (isVisible() == false)
	{
		return;
	}
	for (auto& childData : renderables)
	{
		if (childData.child->isVisible())
		{
			childData.child->render(false, isIdentity ? worldMatrix : worldMatrix * mat, crop);
		}
	}
}

void cRenderableContainer::addRenderable(cRenderable *child)
{
	ChildData childData;
	childData.child = child;
	child->setAlignment(alignment);
	renderables.push_back(childData);
}

void cRenderableContainer::removeRenderable(cRenderable *child)
{
	for (int i = 0; i < renderables.size(); i++)
	{
		if (renderables[i].child == child)
		{
			renderables.erase(renderables.begin() + i);
			return;
		}
	}
}
