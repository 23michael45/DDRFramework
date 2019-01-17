#ifndef MessageSerializer_h__
#define MessageSerializer_h__
#include <memory>
#include "../../src/FSM/FSM.h"
#include "asio.hpp"
#include "../../proto/BaseCmd.pb.h"
#include "BaseMessageDispatcher.h"
#include "BaseSocketContainer.h"
#include <queue>

#define HeadSignal "pbh\0"
#define TEMP_BUFFER_SIZE 4096
#define  MAX_SEND_QUEUESIZE 256
namespace DDRFramework
{
	class BaseMessageDispatcher;
	class BaseHeadRuleRouter;


	class MessageSerializer : public std::enable_shared_from_this<MessageSerializer>
	{
	public:
		MessageSerializer();
		~MessageSerializer();
		virtual void Update();

		void Init();
		void Deinit();

		std::shared_ptr<asio::streambuf> SerializeMsg(std::shared_ptr<DDRCommProto::CommonHeader> spheader, std::shared_ptr<google::protobuf::Message> spmsg);
		std::shared_ptr<asio::streambuf> SerializeMsg(std::shared_ptr<google::protobuf::Message> spmsg);
		std::shared_ptr<asio::streambuf> SerializeMsg(std::shared_ptr<DDRCommProto::CommonHeader> spheader, asio::streambuf& buf,int bodylen,bool needEncryptBody = false);


		bool Pack(std::shared_ptr<DDRCommProto::CommonHeader> spheader, std::shared_ptr<google::protobuf::Message> spmsg);
		bool Pack(std::shared_ptr<DDRCommProto::CommonHeader> spheader, asio::streambuf& buf, int bodylen);

		std::mutex& GetRecLock()
		{
			return mMutexRec;
		}
		std::mutex& GetSendLock()
		{
			return mMutexSend;
		}	
		std::mutex& GetUpdateLock()
		{
			return mMutexUpdate;
		}
		asio::streambuf& GetRecBuf()
		{
			return mDataStreamReceive;
		}
		std::shared_ptr<asio::streambuf> GetSendBuf();
		void PushSendBuf(std::shared_ptr<asio::streambuf> spbuf);
		void PopSendBuf();

		void BindDispatcher(std::shared_ptr<BaseMessageDispatcher> spDispatcher, std::shared_ptr<BaseHeadRuleRouter> spHeadRuleRouter = nullptr)
		{
			m_spDispatcher = spDispatcher;
			m_spHeadRuleRouter = spHeadRuleRouter;

		}
		void Dispatch(std::shared_ptr<DDRCommProto::CommonHeader> spHeader, std::shared_ptr<google::protobuf::Message> spMsg);

		bool IgnoreBody(std::shared_ptr<DDRCommProto::CommonHeader> spHeader, asio::streambuf& buf, int bodylen);

		void BindBaseSocketContainer(std::shared_ptr<BaseSocketContainer> sp)
		{
			m_spBaseSocketContainer = sp;
		}
		std::shared_ptr<BaseSocketContainer> GetSocketContainer()
		{
			return m_spBaseSocketContainer;
		}
	protected:
	
		std::shared_ptr<StateMachine<MessageSerializer>> m_spStateMachine;

		asio::streambuf mDataStreamReceive;
		//std::queue<std::shared_ptr<asio::streambuf>> m_DataStreamSendQueue;

		std::shared_ptr<std::queue<std::shared_ptr<asio::streambuf>>> m_spDataStreamSendQueue;
		bool m_bReachMaxQueue;

		std::mutex mMutexRec;
		std::mutex mMutexSend;
		std::mutex mMutexUpdate;

		std::shared_ptr<BaseHeadRuleRouter> m_spHeadRuleRouter;
		std::shared_ptr<BaseMessageDispatcher> m_spDispatcher;
		std::shared_ptr<BaseSocketContainer> m_spBaseSocketContainer;


		char m_TempRecvBuffer[TEMP_BUFFER_SIZE];
		int m_TotalPackLen;

	};

	class MessageSerializerState : public State<MessageSerializer>
	{
	public:
		MessageSerializerState(std::shared_ptr<MessageSerializer> sp) : State<MessageSerializer>::State(sp)
		{

		};
		~MessageSerializerState()
		{

		}

		virtual bool isValidNextState(std::shared_ptr<State<MessageSerializer>> state) { return true; };

	private:

	};
	class ParsePBHState : public MessageSerializerState
	{
	public:
		ParsePBHState(std::shared_ptr<MessageSerializer> sp) : MessageSerializerState::MessageSerializerState(sp)
		{

		};
		virtual void updateWithDeltaTime(float delta);
	};
	class ParseLengthState : public MessageSerializerState
	{
	public:
		ParseLengthState(std::shared_ptr<MessageSerializer> sp) : ParseLengthState::MessageSerializerState(sp)
		{

		};
		virtual void updateWithDeltaTime(float delta);

	};
	class ParseHeadState : public MessageSerializerState
	{
	public:
		ParseHeadState(std::shared_ptr<MessageSerializer> sp) : MessageSerializerState::MessageSerializerState(sp)
		{

		};
		virtual void updateWithDeltaTime(float delta);

		void SetLen(size_t totalLen, size_t headLen)
		{
			m_TotalLen = totalLen;
			m_HeadLen = headLen;
		}
	private:
		size_t m_TotalLen;
		size_t m_HeadLen;

	};
	class ParseBodyState : public MessageSerializerState
	{
	public:
		ParseBodyState(std::shared_ptr<MessageSerializer> sp) : MessageSerializerState::MessageSerializerState(sp)
		{

		};
		virtual void updateWithDeltaTime(float delta);
		void SetLen(std::shared_ptr<DDRCommProto::CommonHeader> spheader, size_t bodyLen)
		{
			m_spCommonHeader = spheader;
			m_BodyLen = bodyLen;
		}
	private:

		size_t m_BodyLen;
		std::shared_ptr<DDRCommProto::CommonHeader> m_spCommonHeader;
	};


	class WaitNextBuffState : public MessageSerializerState
	{
	public:
		WaitNextBuffState(std::shared_ptr<MessageSerializer> sp) : MessageSerializerState::MessageSerializerState(sp)
		{

		};
		virtual void updateWithDeltaTime(float delta);
		void SetPreStateAndNextLen(std::string prestate,int len)
		{
			m_NextLen = len;
			m_PreStateName = prestate;
		}
	private:
		int m_NextLen;
		std::string m_PreStateName;
	};

}


#endif // MessageSerializer_h__
