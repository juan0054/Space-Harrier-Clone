#include "Player.h"

#include <assert.h>
#include "Engine/API.h"
#include "Engine/GameObject.h"
#include "Engine/Transform.h"
#include "Engine/Sprite.h"
#include "Engine/SpriteSheet.h"
#include "Engine/Collider.h"
#include "gameData.h"
#include "utils.h"
#include "FloorManager.h"
#include "FloorObjectMover.h"
#include "ObjectEffectType.h"
#include "AnimationSection.h"
#include "GameObjectPool.h"
#include "PlayerShot.h"
#include "Messenger.h"
#include "MessengerEventType.h"


void Player::onDestroy()
{
	Messenger::removeListener(this, MessengerEventType::PLAYER_REVIVED);
	Messenger::removeListener(this, MessengerEventType::GAME_WON);

	Audio::unloadSFX(m_sfxTrip);
	Audio::unloadSFX(m_sfxDie);
	Audio::unloadSFX(m_sfxPostDie);
	Audio::unloadSFX(m_sfxShot);

	delete m_dieAnimation;
	m_dieAnimation = nullptr;
	delete m_shotsPool;
	m_shotsPool = nullptr;
}


void Player::init(const Reference<GameObject>& characterGo, const Reference<GameObject>& shadowGo)
{
	m_shadowGo = shadowGo;
	m_characterGo = characterGo;
	m_spriteSheet = characterGo->getComponent<SpriteSheet>();
	assert(m_characterGo && m_shadowGo && m_spriteSheet);

	m_state = PlayerState::MOVE;

	m_motionSpeed = 1.0f / 1300; // One normalized screen dimension (could be height or width) divided by the time taken to go over it
	// x-coordinates-related fields
	m_minX = -0.42f;
	m_midX = 0;
	m_maxX = 0.42f;
	m_xTarget = m_midX;

	// y-coordinates-related fields
	m_minY = 0;
	m_midY = 0.35f;
	m_maxY = 0.71f;
	m_yTarget = m_midY;
}

void Player::awake()
{
	assert(floorManager);

	Messenger::addListener(this, MessengerEventType::PLAYER_REVIVED);
	Messenger::addListener(this, MessengerEventType::GAME_WON);

	m_sfxTrip = Audio::loadSFX(ASSET_SFX_PLAYER_TRIP);
	m_sfxDie = Audio::loadSFX(ASSET_SFX_PLAYER_DIE);
	m_sfxPostDie = Audio::loadSFX(ASSET_SFX_PLAYER_REVIVE);
	m_sfxShot = Audio::loadSFX(ASSET_SFX_PLAYER_SHOT);

	// Die Animation
	m_dieAnimation = new TimedAnimation(getDieAnimationInfo(), m_spriteSheet);

	// Shooting
	m_shotsPool = new GameObjectPool(Prefabs::getPrefab(PLAYER_SHOT_PREFAB), 6);
}


void Player::start()
{

	gameObject()->transform->setLocalPosition(Vector2(0, 0));
	m_currentNormalizedPosition.x = m_midX;
	m_currentNormalizedPosition.y = m_minY;
	m_shadowGo->transform->setLocalPosition(Vector2(m_midX * SCREEN_WIDTH, m_minY * SCREEN_HEIGHT));
	m_characterGo->transform->setLocalPosition(Vector2(m_midX * SCREEN_WIDTH, m_minY * SCREEN_HEIGHT));
	m_spriteSheet->setAnimationSpeed(16);
	m_spriteSheet->playAnimation("run");
	m_currentAnimation = "run";
}


void Player::update()
{
	switch (m_state)
	{
	case PlayerState::MOVE:
		float normalizedRequestedX;
		float normalizedRequestedY;
		handleInput(normalizedRequestedX, normalizedRequestedY);
		move(normalizedRequestedX, normalizedRequestedY);
		moveAnimationUpdate();
		if (m_gameWon)
		{
			floorManager->stopHorizontal = true;
			m_state = PlayerState::GAME_WON;
		}
		break;
	case PlayerState::SHORT_TRIP:
		tripUpdate();
		break;
	case PlayerState::LONG_TRIP:
		tripUpdate();
		break;
	case PlayerState::DIE:
		dieUpdate();
		break;
	case PlayerState::POST_DIE:
		postDieUpdate();
		break;
	case PlayerState::GAME_WON:
		
		move(m_midX, m_minY);
		moveAnimationUpdate();
		break;
		break;
	default:
		break;
	}
}


void Player::onTriggerEnter(Reference<Collider>& other)
{
	if (m_gameWon)
	{
		return;
	}
	ObjectEffectType oet = ObjectEffectType::UNDEFINED;
	// Player can only collider with Obstacles, Enemies and Enemy Shots
	// If the collision occured against an Obstacle, the ObjectEffectType must be read from the obstacle
	Reference<FloorObjectMover> fom = other->gameObject()->getComponentInParent<FloorObjectMover>();
	if (fom)
	{
		oet = fom->getType();
	}
	// In any other case (Enemy or EnemyShot), the Effect is Death
	else
	{
		oet = ObjectEffectType::DIE;
	}

	handleStateChangingCollision(oet);
}


void Player::eventsCallback(MessengerEventType eventType)
{
	if (eventType == MessengerEventType::PLAYER_REVIVED)
	{
		revive();
	}
	else if (eventType == MessengerEventType::GAME_WON)
	{
		m_gameWon = true;
	}
}


bool Player::isAnimatingDeath() const
{
	return m_isAnimatingDeath;
}


Reference<Transform> Player::getCharacterTransform() const
{
	return m_characterGo->transform;
}


void Player::handleInput(float & normalizedRequestedX, float & normalizedRequestedY) const
{
	// Motion
	normalizedRequestedX = m_midX;
	if (Input::getKey(SDL_SCANCODE_LEFT))
	{
		normalizedRequestedX = m_minX;
	}
	if (Input::getKey(SDL_SCANCODE_RIGHT))
	{
		normalizedRequestedX = m_maxX;
	}

	normalizedRequestedY = m_midY;
	if (Input::getKey(SDL_SCANCODE_DOWN))
	{
		normalizedRequestedY = m_minY;
	}
	if (Input::getKey(SDL_SCANCODE_UP))
	{
		normalizedRequestedY = m_maxY;
	}

	// Shooting
	if (Input::getKeyDown(SDL_SCANCODE_LCTRL))
	{
		shoot();
	}
}


void Player::move(float normalizedRequestedX, float normalizedRequestedY, bool horizontalOnly, bool verticalOnly)
{
	if (!verticalOnly)
	{
		// Get motion direction
		int xDirection = normalizedRequestedX - m_currentNormalizedPosition.x > 0 ? 1 : (normalizedRequestedX - m_currentNormalizedPosition.x < 0 ? -1 : 0);
		// Modify currentPosition (clamp so we don't overshoot the requested position)
		m_currentNormalizedPosition.x = clamp(m_currentNormalizedPosition.x + xDirection * m_motionSpeed * Time::deltaTime(), m_currentNormalizedPosition.x, normalizedRequestedX);
		float xPos = m_currentNormalizedPosition.x * SCREEN_WIDTH;

		// Apply the modified position to the corresponding transform (x moves this GO while y moves the characterGp)
		Vector2 goPos = gameObject()->transform->getLocalPosition();
		goPos.x = xPos;
		gameObject()->transform->setLocalPosition(goPos);
	}

	if (!horizontalOnly)
	{
		// Get motion direction
		int yDirection = normalizedRequestedY - m_currentNormalizedPosition.y > 0 ? 1 : (normalizedRequestedY - m_currentNormalizedPosition.y < 0 ? -1 : 0);
		// Modify currentPosition (clamp so we don't overshoot the requested position)
		m_currentNormalizedPosition.y = clamp(m_currentNormalizedPosition.y + yDirection * m_motionSpeed * Time::deltaTime(), m_currentNormalizedPosition.y, normalizedRequestedY);
		float yPos = m_currentNormalizedPosition.y * SCREEN_HEIGHT;

		// Apply the modified position to the corresponding transform (x moves this GO while y moves the characterGp)
		Vector2 charPos = m_characterGo->transform->getLocalPosition();
		charPos.y = yPos;
		m_characterGo->transform->setLocalPosition(charPos);
	}
}


void Player::moveAnimationUpdate()
{
	std::string requestedAnimation = "";
	// Decide between run of a version of fly
	if (m_currentNormalizedPosition.y == m_minY)
	{
		requestedAnimation = "run";
	}
	else
	{
		// Decide between flying animations
		float normalizedXPos = m_currentNormalizedPosition.x;
		if (normalizedXPos < m_minX * 0.9f)
		{
			requestedAnimation = "flyLeft";
		}
		else if (normalizedXPos > m_maxX * 0.9f)
		{
			requestedAnimation = "flyRight";
		}
		else if (normalizedXPos < (m_midX + m_minX) / 2.0f)
		{
			requestedAnimation = "flyCenterLeft";
		}
		else if (normalizedXPos > (m_maxX + m_midX) / 2.0f)
		{
			requestedAnimation = "flyCenterRight";
		}
		else
		{
			requestedAnimation = "flyCenter";
		}
	}

	// Play new animation if required
	if (requestedAnimation != m_currentAnimation)
	{
		m_currentAnimation = requestedAnimation;
		m_spriteSheet->playAnimation(m_currentAnimation, 16.0f);
	}
}


void Player::tripUpdate()
{
	floorManager->stopHorizontal = true;
	
	// Start animations
	if (m_state == PlayerState::SHORT_TRIP && m_currentAnimation != "shortTrip" || m_state == PlayerState::LONG_TRIP && m_currentAnimation != "longTrip")
	{
		if (m_state == PlayerState::SHORT_TRIP && m_currentAnimation != "shortTrip")
		{
			m_currentAnimation = "shortTrip";
		}
		else
		{
			m_currentAnimation = "longTrip";
		}
		m_spriteSheet->playAnimation(m_currentAnimation, 8.0f, false);
		Vector2 charPos = m_characterGo->transform->getLocalPosition();
		charPos.y = m_minY;
		m_characterGo->transform->setLocalPosition(charPos);
	}
		
	if (m_spriteSheet->isFinished())
	{
		if (m_gameWon)
		{
			floorManager->stopHorizontal = true;
			m_state = PlayerState::GAME_WON;
		}
		else
		{
			floorManager->stopHorizontal = false;
			m_state = PlayerState::MOVE;
		}
	}
}


void Player::dieUpdate()
{
	m_isAnimatingDeath = true;
	if (m_dieAnimation->isFinished())
	{
		m_state = PlayerState::POST_DIE;
		return;
	}
	Vector2 currentCharPos = m_characterGo->transform->getLocalPosition();
	m_dieAnimation->update(currentCharPos.y);
	m_characterGo->transform->setLocalPosition(currentCharPos);
}


void Player::postDieUpdate()
{
	if (m_postDieElapsedTime == INT_MIN)
	{
		Audio::playSFX(m_sfxPostDie);
		m_postDieElapsedTime = -(int)Time::deltaTime();
		m_currentAnimation = "flyCenter";
		m_spriteSheet->playAnimation(m_currentAnimation, 16.0f);
		
		if (!m_gameWon)
		{
			--player_lives;
		}
		if (player_lives > 0)
		{
			Messenger::broadcastEvent(MessengerEventType::PLAYER_LOSE_LIFE);
		}
		else
		{
			Messenger::broadcastEvent(MessengerEventType::PLAYER_DEAD);
			m_spriteSheet->pauseAnimation();
			m_isDead = true;
		}
	}

	// Verify if player still has lives. if not, hold until revived (the UI will issue the order throught the Messenger service)
	if (m_isDead)
	{
		return;
	}
	m_spriteSheet->resumeAnimation();
	
	// Limited motion
	float normalizedRequestedX;
	float normalizedRequestedY;
	handleInput(normalizedRequestedX, normalizedRequestedY);

	m_postDieElapsedTime += Time::deltaTime();
	float pastCyclesTime = 0;

	// Blinking and limited motion (horizontal only) starts
	for (int i = 0; i < POST_DIE_BLINK_FLOOR_CYCLE_COUNT; ++i)
	{
		if (m_postDieElapsedTime - pastCyclesTime < POST_DIE_BLINK_CYCLE_DURATION)
		{
			if (m_postDieElapsedTime - pastCyclesTime < POST_DIE_BLINK_CYCLE_DURATION / 2)
			{
				m_spriteSheet->gameObject()->transform->setLocalScale(Vector2(0, 0));
			}
			else
			{
				m_spriteSheet->gameObject()->transform->setLocalScale(Vector2(1, 1));
			}
			move(normalizedRequestedX, normalizedRequestedY, true);
			return;
		}
		pastCyclesTime += POST_DIE_BLINK_CYCLE_DURATION;
	}

	// Blinking and free motion starts (free motion)
	if (floorManager->freezeAtBottom)
	{
		floorManager->freezeAtBottom = false;
		Messenger::broadcastEvent(MessengerEventType::FLOOR_MOTION_RESUMED);
	}

	move(normalizedRequestedX, normalizedRequestedY);
	moveAnimationUpdate();
	for (int i = 0; i < POST_DIE_BLINK_FREE_CYCLE_COUNT; ++i)
	{
		// Off-part of the blink
		if (m_postDieElapsedTime - pastCyclesTime < POST_DIE_BLINK_CYCLE_DURATION / 2)
		{
			m_spriteSheet->gameObject()->transform->setLocalScale(Vector2(0, 0));
			return;
		}
		// On-part of the blink
		else if (m_postDieElapsedTime - pastCyclesTime < POST_DIE_BLINK_CYCLE_DURATION)
		{
			m_spriteSheet->gameObject()->transform->setLocalScale(Vector2(1, 1));
			return;
		}
		pastCyclesTime += POST_DIE_BLINK_CYCLE_DURATION;
	}

	// postDie is finished
	m_spriteSheet->gameObject()->transform->setLocalScale(Vector2(1, 1));
	m_postDieElapsedTime = INT_MIN;
	if (m_gameWon)
	{
		floorManager->stopHorizontal = true;
		m_state = PlayerState::GAME_WON;
	}
	else
	{
		m_state = PlayerState::MOVE;
	}
	m_isAnimatingDeath = false;
}


void Player::revive()
{
	assert(m_isDead);
	m_isDead = false;
}


void Player::handleStateChangingCollision(ObjectEffectType oet)
{
	if (m_state != PlayerState::DIE && m_state != PlayerState::POST_DIE) {
		switch (oet)
		{
		case ObjectEffectType::SHORT_TRIP:
			if (m_state != PlayerState::LONG_TRIP)
			{
				m_state = PlayerState::SHORT_TRIP;
				Audio::playSFX(m_sfxTrip);
			}
			break;
		case ObjectEffectType::LONG_TRIP:
			if (m_state != PlayerState::SHORT_TRIP)
			{
				m_state = PlayerState::LONG_TRIP;
				Audio::playSFX(m_sfxTrip);
			}
			break;
		case ObjectEffectType::DIE:
			m_currentNormalizedPosition.y = m_minY;
			m_spriteSheet->selectAnimation("die", 0);
			m_dieAnimation->start(m_characterGo->transform->getLocalPosition().y);

			floorManager->stopHorizontal = false;
			if (!floorManager->freezeAtBottom)
			{
				floorManager->freezeAtBottom = true;
				Messenger::broadcastEvent(MessengerEventType::FLOOR_MOTION_STOPPED);
			}

			m_state = PlayerState::DIE;
			Audio::playSFX(m_sfxDie);

			break;
		case ObjectEffectType::UNDEFINED:
		default:
			assert(false);
			break;
		}
	}
}


void Player::shoot() const
{
	if (m_shotsPool)
	{
		Vector2 currPos = m_characterGo->transform->getWorldPosition();
		// The shot is lifted 10 px from the feet of the character
		currPos.y += 10;
		Reference<GameObject> shotGO = m_shotsPool->getGameObject();
		shotGO->setActive(true);
		Reference<PlayerShot> shot = shotGO->getComponent<PlayerShot>();
		assert(shot);
		shot->init(floorManager, currPos);
		Audio::playSFX(m_sfxShot);
	}
}
