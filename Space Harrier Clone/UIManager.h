#ifndef H_UI_MANAGER
#define H_UI_MANAGER

#include "Engine/Behaviour.h"
#include "MessengerEventListener.h"
#include "Engine/Reference.h"
#include "Engine/SFX.h"
class GameObject;
class Sprite;
class TextRenderer;
class Ranking;


class UIManager :
	public Behaviour,
	public MessengerEventListener
{
public:
	virtual void onDestroy() override;

	void init(const Reference<Ranking>& ranking);
	virtual void awake() override;
	virtual void start() override;
	virtual void update() override;

	// Inherited via MessengerEventListener
	virtual void eventsCallback(MessengerEventType eventType) override;

	int getTopScore() const;
	void setTopScore(int topScore);
	int getStageNumber() const;
	void setStageNumber(int stageNumber);

private:
	void updateTopScoreText();
	void updateScoreText();
	void updateLivesText();
	void updateStageText();

	void addPoints(int pointsToAdd);
	void showPostReviveMessage();
	void startInsertCoinsPrompt();
	void finishInsertCoinsPrompt();
	void handleGameWon();
	void handleGameOver();

	Reference<Ranking> m_ranking;

	// Containing GO
	Reference<GameObject> m_uiGO;
	bool m_isActive = true;

	// Constant UI
	Reference<Sprite> m_topLabel;
	Reference<Sprite> m_scoreLabel;
	Reference<TextRenderer> m_stageLabel;
	Reference<TextRenderer> m_postReviveLabel;
	Reference<TextRenderer> m_stageNumberLabel;
	Reference<TextRenderer> m_stageNameLabel;
	Reference<TextRenderer> m_insertCoinsLabel;
	Reference<TextRenderer> m_postCoinsStartLabel;
	Reference<TextRenderer> m_gameWonLabel;

	// Changing UI
	Reference<TextRenderer> m_topScoreText;
	Reference<TextRenderer> m_scoreText;
	Reference<TextRenderer> m_livesText;
	Reference<TextRenderer> m_stageText;
	Reference<TextRenderer> m_insertCoinsCountdownText;

	// Points management
	int m_pointsPerStep;
	int m_pointsStepsPerSecond;
	bool m_shouldAddPoints;
	float m_pointsElapsedTime;
	float m_pointsTimeLimit;

	// Temporary message timers and info
	int m_totalElapsedTime;

	int m_hideStageInfoEndTime;
	int m_hidePostReviveLabelEndTime;
	int m_nextCountDownDropTime;
	int m_currentCountDownNumber;
	bool m_showingInsertCointsPrompt;

	// Game Win timer
	int m_gameWonLabelDuration;
	int m_gameWonLabelEndTime;

	SFX m_sfxCoin;

	int m_currentTopScore;
	int m_currentScore;
	int m_currentStage;
};


#endif // !H_UI_MANAGER
