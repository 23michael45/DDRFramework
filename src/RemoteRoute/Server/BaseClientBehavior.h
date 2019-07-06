#ifndef BaseClientBehavior_h__
#define BaseClientBehavior_h__
#include "src/Logic/BaseBehavior.h"
#include "asio.hpp"
#include "src/Utility/Timer.hpp"
class BaseClientBehavior : public DDRFramework::BaseBehavior
{
public:
	BaseClientBehavior();
	~BaseClientBehavior();

	virtual void OnStart(std::shared_ptr<DDRFramework::TcpSocketContainer> spContainer) override;
	virtual void Update(std::shared_ptr<DDRFramework::TcpSocketContainer> spContainer) override;
	virtual void OnStop(std::shared_ptr<DDRFramework::TcpSocketContainer> spContainer) override;

	void ResetHeartBeat();
protected:
	void HeartBeatTimeout(std::shared_ptr<DDRFramework::TcpSocketContainer> spContainer);


	DDRFramework::Timer m_Timer;
	DDRFramework::timer_id m_HeartBeatTimerID;
};


#endif // BaseClientBehavior_h__