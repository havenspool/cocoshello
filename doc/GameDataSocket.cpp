#include "GameDataSocket.h"
#include "scripting/lua-bindings/manual/CCLuaEngine.h"
#include "cocos2d.h"
#include "scripting/lua-bindings/manual/lua_module_register.h"
#include "pthread.h"

USING_NS_CC;
using namespace std;

GameDataSocket::GameDataSocket()
{
}

int GameDataSocket::connectThreadStart() {
	//    connect(GAMESERVER, CCString::create(GAMESERVER_PORT)->intValue());  

	int errCode = 0;
	do {
		pthread_attr_t tAttr;
		errCode = pthread_attr_init(&tAttr);

		CC_BREAK_IF(errCode != 0);

		errCode = pthread_attr_setdetachstate(&tAttr, PTHREAD_CREATE_DETACHED);

		if (errCode != 0) {
			pthread_attr_destroy(&tAttr);
			break;
		}

		errCode = pthread_create(&m_gameThread, &tAttr, connectSocket, this);

	} while (0);
	return errCode;
}

void GameDataSocket::connectSocket(void* args)
{
	connect("192.168.1.2", 3343);
}

int GameDataSocket::connect(const char* ip, unsigned int port)
{
	CCLOG("Client begin connect IP: %s:%d ", ip, port);
	struct sockaddr_in sa;
	struct hostent* hp;

	hp = gethostbyname(ip);
	if (!hp) {
		return -1;
	}
	memset(&sa, 0, sizeof(sa));
	memcpy((char*)&sa.sin_addr, hp->h_addr, hp->h_length);
	sa.sin_family = hp->h_addrtype;
	sa.sin_port = htons(port);

	m_socketHandle = socket(sa.sin_family, SOCK_STREAM, 0);

	if (m_socketHandle < 0) {
		printf("failed to create socket\n");
		return -1;
	}
	if (::connect(m_socketHandle, (sockaddr*)&sa, sizeof(sa)) < 0) {
		printf("failed to connect socket\n");
		::close(m_socketHandle);
		return -1;
	}

	CCLOG("Client connect OK ！ IP: %s:%d ", ip, port);

	MTNotificationQueue::sharedNotificationQueue()->postNotification("connectok", NULL);
	return 0;
}

void GameDataSocket::initReceiveThread(CCObject* obj)
{
	int errCode = 0;
	pthread_attr_t tAttr;
	errCode = pthread_attr_init(&tAttr);

	errCode = pthread_attr_setdetachstate(&tAttr, PTHREAD_CREATE_DETACHED);

	if (errCode != 0) {
		pthread_attr_destroy(&tAttr);
	}
	else {
		errCode = pthread_create(&m_gameThread, &tAttr, listenSocketData, this);
	}
	if (errCode == 0) {
		CCLOG("Receive Thread OK!!!");
	}
	else {
		CCLOG("Receive Thread Error!!!!");
	}

	MTNotificationQueue::sharedNotificationQueue()->postNotification("jointable", NULL);
}

void* GameDataSocket::listenSocketData(void* obj)
{
	byte buffer[5];
	string contents;
	int ret = 0;
	// 先接受4字节，获取服务返回长度  
	bool rs = true;
	while (rs)
	{
		contents = "";
		ret = recv(m_socketHandle, buffer, 4, 0);
		// 服务器关闭  
		if (ret == 0)
		{
			//            CCLog("Error: server close");  
			rs = false;
		}
		if (ret == 4)
		{
			buffer[4] = '\0';
			int packetlen = Utils::bytes2int(buffer);
			CCLOG("packetlen %d", packetlen);
			char buf[packetlen];
			int rets = 0;
			while ((ret = recv(m_socketHandle, buf, packetlen - rets, 0))>0)
			{
				contents.append(buf, ret);
				packetlen -= ret;
				if (packetlen <= 0)
					break;
			}
			CCLog("recv content:%s\n", contents.c_str());
			CCString* str = CCString::create(Utils::getUnPackMsg(contents));
			MTNotificationQueue::sharedNotificationQueue()->postNotification("receivedata", str);
		}
		else {
			CCLog("Error: recv data Error %d", ret);
		}
	}
	return NULL;
}