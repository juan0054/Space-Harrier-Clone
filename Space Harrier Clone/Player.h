#ifndef H_PLAYER
#define H_PLAYER

#include "Engine/Behaviour.h"
#include "Engine/Reference.h"
#include "Engine/Vector2.h"
#include "Engine/SFX.h"
#include "Engine/Music.h"
#include "TimedAnimation.h"
class GameObject;
class SpriteSheet;
class Collider;


class Player :
	public Behaviour
{
public:
	~Player();

	void init(const Reference<GameObject>& characterGo, const Reference<GameObject> shadowGo);
	virtual void start() override;
	virtual void update() override;
	virtual void onTriggerEnter(Reference<Collider>& other) override;

private:
	enum class PlayerState
	{
		STOP,
		MOVE,
		SHORT_TRIP,
		LONG_TRIP,
		DIE,
		POST_DIE
	};

	// MOVE related functions
	void handleInput(float& normalizedRequestedX, float& normalizedRequestedY) const;
	void move(float normalizedRequestedX, float normalizedRequestedY);
	void moveAnimationUpdate();

	// TRIP related functions
	void tripUpdate();

	// DIE related functions
	void dieUpdate();
	void postDieUpdate();

	Reference<GameObject> m_shadowGo;
	Reference<GameObject> m_characterGo;
	Reference<SpriteSheet> m_spriteSheet;
	TimedAnimation* m_dieAnimation = nullptr;

	std::string m_currentAnimation = "";
	float m_motionSpeed = 0;
	Vector2 m_currentNormalizedPosition;
	
	PlayerState m_state;

	// x-coordinates-related fields
	float m_minX = 0;
	float m_midX = 0;
	float m_maxX = 0;
	float m_xTarget = 0;

	// y-coordinates-related fields
	float m_minY = 0;
	float m_midY = 0;
	float m_maxY = 0;
	float m_yTarget = 0;

	SFX m_sfxOuch;
	Music m_m1;
	Music m_m2;
};


#endif // !H_PLAYER
