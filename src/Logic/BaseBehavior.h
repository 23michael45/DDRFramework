#ifndef BaseBehavior_h__
#define BaseBehavior_h__


namespace DDRFramework
{
	class TcpSocketContainer;
	enum ENUM_BEHAVIOR_TYPE {
		EBT_UNKNOWN,
		EBT_ROBOT,
		EBT_MONITOR,
		EBT_CLIENT,
		EBT_LSM,
		EBT_SUBCH
	};

	class BaseBehavior
	{
	public:


		virtual void OnStart(DDRFramework::TcpSocketContainer& container)
		{
		};
		virtual void Update(DDRFramework::TcpSocketContainer& container)
		{
		};
		virtual void OnStop(DDRFramework::TcpSocketContainer& container)
		{
		};

		virtual void CopyDataFrom(const BaseBehavior& fromBehavior)
		{
			m_eType = fromBehavior.m_eType;
			m_bValid = fromBehavior.m_bValid;
			m_uID = fromBehavior.m_uID;

		};

	protected:
		ENUM_BEHAVIOR_TYPE m_eType;
		bool m_bValid;
		int m_uID;
	};
}

#endif // BaseBehavior_h__
