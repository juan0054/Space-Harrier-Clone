#include "EnemyShipPrefab.h"

#include "Engine/GameObject.h"
#include "Engine/Transform.h"
#include "Engine/SpriteSheet.h"
#include "Engine/Sprite.h"
#include "Engine/RectangleCollider.h"
#include "Enemy.h"
#include "CollisionCallbackForwarder.h"
#include "ExplosiveObject.h"


void EnemyShipPrefab::configureGameObject(Reference<GameObject>& gameObject) const
{
	auto enemy = gameObject->addComponent<Enemy>();
	auto explosiveObject = gameObject->addComponent<ExplosiveObject>();
	if (enemy && explosiveObject)
	{
		auto characterGo = GameObject::createNew();
		if (characterGo)
		{
			characterGo->transform->setParent(gameObject->transform, false);

			auto rectColl = characterGo->addComponent<RectangleCollider>();
			if (rectColl)
			{
				rectColl->isTrigger = true;
				rectColl->size = Vector2(112, 48);
				rectColl->offset.y += rectColl->size.y / 2;
				rectColl->setCollisionLayer("Enemy");
			}

			auto spriteSheet = characterGo->addComponent<SpriteSheet>();
			if (spriteSheet)
			{
				spriteSheet->loadImage("assets/sprites/Enemies.png");
				spriteSheet->setAllPivots(Vector2(0.5f, 0));
				spriteSheet->setRenderLayer("Main");

				spriteSheet->addAnimation("fly");
				spriteSheet->addRectForAnimation("fly", Vector2(5, 150), 112, 48);
				spriteSheet->addRectForAnimation("fly", Vector2(120, 150), 112, 48);
				spriteSheet->addRectForAnimation("fly", Vector2(5, 205), 112, 48);
				spriteSheet->addRectForAnimation("fly", Vector2(120, 205), 112, 48);
			}

			auto ccf1 = characterGo->addComponent<CollisionCallbackForwarder>();
			if (ccf1)
			{
				ccf1->target = enemy;
			}

			auto ccf2 = characterGo->addComponent<CollisionCallbackForwarder>();
			if (ccf2)
			{
				ccf2->target = explosiveObject;
			}
		}	
	}

	auto shadowGo = GameObject::createNew();
	if (shadowGo)
	{
		shadowGo->transform->setParent(gameObject->transform, false);

		auto shadow = shadowGo->addComponent<Sprite>();
		if (shadow)
		{
			shadow->setRenderLayer("Shadows");
			shadow->setZIndex(0);

			shadow->loadImage("assets/sprites/Enemies.png");
			shadow->setClipRect(SDL_Rect{ 235, 150, 62, 18 });
			shadow->setAllPivots(Vector2(0.5f, 0));
		}
	}
}
