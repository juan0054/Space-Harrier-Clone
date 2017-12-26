#include "FloorObjectsFactory.h"

#include <random>
#include <assert.h>
#include "Engine/API.h"
#include "Engine/PrefabsFactory.h"
#include "Engine/GameObject.h"
#include "Engine/Transform.h"
#include "FloorManager.h"
#include "TreePrefab.h"
#include "FloorObjectMover.h"
#include "ObjectEffectType.h"
#include "GameObjectPool.h"
#include "ExplosionPrefab.h"
#include "ExplosiveObject.h"


FloorObjectsFactory::~FloorObjectsFactory()
{
	for (GameObjectPool* gop : m_prefabPools)
	{
		delete gop;
	}
	m_prefabPools.clear();

	delete m_explosionsPool;
	m_explosionsPool = nullptr;
}


void FloorObjectsFactory::init(const Reference<FloorManager>& floorManager)
{
	m_floorManager = floorManager;
	assert(m_floorManager);
	m_sfxExplosion = Audio::LoadSFX("assets/audio/sfx/SFX - Explosion.wav");
}

void FloorObjectsFactory::start()
{
	assert(m_floorManager);
	m_prefabs.reserve(4);
	m_prefabs.push_back(Prefabs::getPrefab("TreePrefab"));
	m_prefabs.push_back(Prefabs::getPrefab("SmallBushPrefab"));
	m_prefabs.push_back(Prefabs::getPrefab("BigBushPrefab"));
	m_prefabs.push_back(Prefabs::getPrefab("RockPrefab"));
	for (Reference<Prefab>& prefab : m_prefabs)
	{
		assert(prefab);
		m_prefabPools.push_back(new GameObjectPool(prefab, 2));
	}

	m_explosionsPool = new GameObjectPool(Prefabs::getPrefab("ExplosionPrefab"), 3);
	assert(m_explosionsPool);

	m_spawnWaitTime = 2500;
	m_elapsedTime = 0;

	m_normalizedSpawnY = 1.0f;
	m_normalizedDespawnY = 0.05f;
	m_spawnMinX = -20;
	m_spawnMaxX = 20;
	m_distribution = std::uniform_int_distribution<int>(m_spawnMinX, m_spawnMaxX);
}


void FloorObjectsFactory::update()
{
	if (m_floorManager->freezeAtBottom)
	{
		return;
	}
	if (m_elapsedTime > m_spawnWaitTime)
	{
		m_elapsedTime -= m_spawnWaitTime;
		spawnObject();
	}
	m_elapsedTime += Time::deltaTime();
}

void FloorObjectsFactory::spawnObject()
{
	int currentFloorHeight = m_floorManager->getCurrentFloorHeight();
	float spawnX = (float)m_distribution(m_generator);

	auto go = m_prefabPools[m_currPrefabIndex++]->getGameObject();
	
	if (m_currPrefabIndex >= (int)m_prefabPools.size())
	{
		m_currPrefabIndex = 0;
	}

	if (go)
	{
		go->transform->setParent(gameObject()->transform);
		auto mover = go->getComponent<FloorObjectMover>();
		if (mover)
		{
			mover->init(m_floorManager, spawnX, 1 - m_normalizedSpawnY, 1 - m_normalizedDespawnY, 0, 1);
		}
		auto explosiveObject = go->getComponent<ExplosiveObject>();
		if (explosiveObject)
		{
			explosiveObject->init(m_explosionsPool, m_sfxExplosion);
		}
		go->setActive(true);
	}
}
