#include "TreePrefab.h"

#include "Engine/Reference.h"
#include "Engine/GameObject.h"
#include "Engine/Sprite.h"
#include "Engine/RectangleCollider.h"
#include "FloorObjectMover.h"


void TreePrefab::configureGameObject(Reference<GameObject>& gameObject) const
{
	auto rectColl = gameObject->addComponent<RectangleCollider>();
	if (rectColl)
	{
		rectColl->isTrigger = true;
		rectColl->size = Vector2(40, 158);
		rectColl->offset.y += rectColl->size.y / 2;
		rectColl->setCollisionLayer("Obstacle");
	}

	auto sprite = gameObject->addComponent<Sprite>();
	if (sprite)
	{
		sprite->setActive(false);
		// Tree is 40x158 pixels
		sprite->loadImage("assets/Tree.png");
		sprite->setAllPivots(Vector2(0.5f, 0));
		sprite->setRenderLayer("Main");
	}
	gameObject->addComponent<FloorObjectMover>();
}
