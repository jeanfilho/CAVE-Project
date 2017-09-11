#pragma once

class TimeManager
{
private:

	int _previousTime;
	float _deltaTime;
	
public:
	TimeManager(){}
	~TimeManager(){}

	void update()
	{
		int currentTime;
		currentTime = glutGet(GLUT_ELAPSED_TIME);
		_deltaTime = (currentTime - _previousTime) / 1000.0;
		_previousTime = currentTime;
	}

	float deltaTime()
	{
		return _deltaTime;
	}
};

