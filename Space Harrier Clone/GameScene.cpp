#include "GameScene.h"

#include "Engine/API.h"
#include "Engine/GameObject.h"
#include "Engine/Transform.h"
#include "Engine/Behaviour.h"
#include "Engine/Sprite.h"
#include "Engine/Vector2.h"
#include "gameData.h"
#include "FloorManager.h"
#include "DarkLineinfo.h"
#include "FloorObjectsFactory.h"
#include "BackgroundScroller.h"
#include "PlayerPrefab.h"
#include "Player.h"
#include "EnemiesFactory.h"
#include "EnemySpawnInfo.h"
#include "MotionPattern.h"
#include "ObstacleSpawnInfo.h"
#include "UIManager.h"
#include "Ranking.h"
#include "SceneFader.h"
#include "GameSceneMusicManager.h"


bool GameScene::load()
{
	Audio::setSFXVolume(0.25f);

	{
		auto go = Prefabs::instantiate(Prefabs::getPrefab(SCENE_FADER_PREFAB));
		assert(go);
		auto sceneFader = go->getComponent<SceneFader>();
		assert(sceneFader);
		sceneFader->init(0, SDL_Color{ 0, 0, 0, 255 }, 200, 100, 0, true);
	}

	auto worldGO = GameObject::createNew();
	if (worldGO)
	{
		auto musicManager = worldGO->addComponent<GameSceneMusicManager>();
		assert(musicManager);

		worldGO->transform->setWorldPosition(Vector2(SCREEN_WIDTH / 2.0f, 0));

		auto floorManagerGo = GameObject::createNew();
		if (floorManagerGo)
		{
			floorManagerGo->transform->setParent(worldGO->transform, false);

			auto floorManager = floorManagerGo->addComponent<FloorManager>();

			if (floorManager)
			{
				floorManager->init(ASSET_IMG_FLOOR_GREEN);
			}

			auto playerGo = Prefabs::instantiate(Prefabs::getPrefab(PLAYER_PREFAB));
			if (playerGo)
			{
				playerGo->transform->setParent(worldGO->transform, false);
				Reference<Player>& player = playerGo->getComponent<Player>();
				if (player)
				{
					player->floorManager = floorManager;
				}

				auto enemiesFactoryGo = GameObject::createNew();
				if (enemiesFactoryGo)
				{
					enemiesFactoryGo->transform->setParent(worldGO->transform, false);
					auto enemiesFactory = enemiesFactoryGo->addComponent<EnemiesFactory>();
					if (enemiesFactory)
					{
						enemiesFactory->init(player->getCharacterTransform(), floorManager, getEnemiesSpawnInfo(), getMotionPatterns());
					}
				}
			}

			auto objectsFactoryGo = GameObject::createNew();
			if (objectsFactoryGo)
			{
				objectsFactoryGo->transform->setParent(worldGO->transform, false);
				auto floorObjectsFactory = objectsFactoryGo->addComponent<FloorObjectsFactory>();
				if (floorObjectsFactory)
				{
					floorObjectsFactory->init(floorManager, getObstaclesSpawnInfo());
				}
			}

			

			// True background
			auto backgroundScrollerGo = GameObject::createNew();
			if (backgroundScrollerGo)
			{
				auto backgroundScroller = backgroundScrollerGo->addComponent<BackgroundScroller>();
				if (backgroundScroller)
				{
					backgroundScroller->init(floorManager, ASSET_IMG_BACKGROUND, 0, -3);
				}
			}

			// Mountains
			auto backgroundScrollerGoMountains = GameObject::createNew();
			if (backgroundScrollerGoMountains)
			{
				auto backgroundScroller = backgroundScrollerGoMountains->addComponent<BackgroundScroller>();
				if (backgroundScroller)
				{
					backgroundScroller->init(floorManager, ASSET_IMG_BG_MOUNTAINS, 0.3f, -2);
				}
			}

			// Grass
			auto backgroundScrollerGoTrees = GameObject::createNew();
			if (backgroundScrollerGoTrees)
			{
				auto backgroundScroller = backgroundScrollerGoTrees->addComponent<BackgroundScroller>();
				if (backgroundScroller)
				{
					backgroundScroller->init(floorManager, ASSET_IMG_BG_TREES, 0.6f, -1);
				}
			}
		}

		auto uiGo = GameObject::createNew();
		if (uiGo)
		{
			uiGo->transform->setWorldPosition(Vector2(0, 0));
			auto uiManager = uiGo->addComponent<UIManager>();
			auto ranking = uiGo->addComponent<Ranking>();
			assert(uiManager && ranking);
			uiManager->init(ranking);
		}
	}

	return true;
}


void GameScene::unload()
{
}
