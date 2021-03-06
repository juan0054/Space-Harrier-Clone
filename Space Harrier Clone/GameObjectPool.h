#ifndef H_GAME_OBJECT_POOL
#define H_GAME_OBJECT_POOL

#include <stack>
#include <list>
#include "Engine/Reference.h"
class Prefab;
class GameObject;


class GameObjectPool
{
public:
	GameObjectPool(const Reference<Prefab>& prefab, int initialPoolSize);
	~GameObjectPool();

	Reference<GameObject> getGameObject();
	bool returnGameObject(Reference<GameObject>& gameObjectToReturn);

private:
	void createGameObject();

	Reference<Prefab> m_prefab;
	std::list<Reference<GameObject>> m_gameObjects;
	std::stack<Reference<GameObject>*> m_availableGameObjects;
};


#endif // !H_GAME_OBJECT_POOL
