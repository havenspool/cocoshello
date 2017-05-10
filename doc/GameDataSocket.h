#pragma once
#include "cocos2d.h"
#include "pthread.h"

USING_NS_CC;
using namespace std;

class  GameDataSocket : private cocos2d::Application
{
public:
	GameDataSocket();
	virtual int connectThreadStart();
	virtual void connectSocket(void* args);
	virtual int connect(const char* ip, unsigned int port);
	virtual void initReceiveThread(CCObject* obj);
	virtual void listenSocketData(void* obj);
};
