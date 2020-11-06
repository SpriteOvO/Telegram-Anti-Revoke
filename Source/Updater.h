#pragma once

class Updater
{
public:
	static Updater& GetInstance();

	BOOLEAN CheckUpdate();

private:
	BOOLEAN GetDataByBridge(string &ReturnedResponse);
	BOOLEAN GetDataDirectly(string &ReturnedResponse);

};
