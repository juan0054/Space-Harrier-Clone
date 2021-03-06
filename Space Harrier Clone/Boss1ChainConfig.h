#ifndef H_BOSS_1_CHAIN_CONFIG
#define H_BOSS_1_CHAIN_CONFIG

#include <vector>


struct Boss1ChainConfig
{
	int chainLength;
	int chainDelay;
	int deathDelay;

	// Position motion
	float minXAxisDistance;
	float maxXAxisDistance;
	float maxXDifferencePerSecond;
	float minYPos;
	float maxYPos;

	// Depth motion
	float minNormalizedProgress;
	float maxNormalizedProgress;
	int depthCycleTimeMS;

	// Shooting
	int shotsSpreadDistance;
	std::vector<float> normalizedCycleShootingTimes;
};


#endif // !H_BOSS_1_CHAIN_CONFIG
