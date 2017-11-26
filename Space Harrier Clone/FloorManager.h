#ifndef H_FLOOR_MANAGER
#define H_FLOOR_MANAGER

#include "Engine/Behaviour.h"
#include <vector>
#include <string>
#include "Engine/Reference.h"
#include "Engine/Sprite.h"
#include "FloorWarpController.h"
#include "DarkLinesController.h"

class FloorManager :
	public Behaviour
{
public:
	virtual void start() override;
	virtual void update() override;
	
	std::string texturePath;

private:
	void updateCurrentFloorHeight(float normalizedSpeed);

	FloorWarpController floorWarpController;
	DarkLinesController darkLinesController;

	float m_currentPixelHeight = 0;
	float m_pixelScaleSpeed = 0;
	int m_maxHeight = 0;
	int m_midHeight = 0;
	int m_minHeight = 0;

	// Interpolation related
	float m_horInterpolationTime = 0;
	float m_horInterpolationStartTime = 0;
	float m_horSpeedStartingValue = 0;
	float m_horSpeedCurrentValue = 0;
	float m_horSpeedTargetValue = 0;

	int m_targetHeight = 0;
	float m_vertInterpolationTime = 0;
	float m_vertInterpolationStartTime = 0;
	float m_vertSpeedStartingValue = 0;
	float m_vertSpeedCurrentValue = 0;
	float m_vertSpeedTargetValue = 0;
};


#endif // !H_FLOOR_MANAGER
