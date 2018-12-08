#ifndef MessageSerializer_h__
#define MessageSerializer_h__
#include <memory>
#include "../../src/FSM/FSM.h"
#include "asio.hpp"
#include "../../proto/BaseCmd.pb.h"
#include "BaseMessageDispatcher.h"

#define HeadSignal "pbh\0"
#define TEMP_BUFFER_SIZE 4096
namespace DDRFramework
{

	class MessageSerializer : public std::enable_shared_from_this<MessageSerializer>
	{
	public:
		MessageSerializer();
		~MessageSerializer();
		virtual void Update();

		void Init();
		void Deinit();
		bool Pack(google::protobuf::Message& msg);
		void Receive(asio::streambuf& buf);

		std::mutex& GetRecLock()
		{
			return mMutexRec;
		}
		std::mutex& GetSendLock()
		{
			return mMutexSend;
		}
		asio::streambuf& GetRecBuf()
		{
			return mDataStreamReceive;
		}
		asio::streambuf& GetSendBuf()
		{
			return mDataStreamSend;
		}

		void BindDispatcher(std::shared_ptr<BaseMessageDispatcher> sp)
		{
			m_spDispatcher = sp;

		}
		void Dispatch(std::shared_ptr<DDRCommProto::CommonHeader> spHeader, std::shared_ptr<google::protobuf::Message> spMsg);


		void BindSendFunction(std::function<void(asio::streambuf&)> f)
		{
			m_fSendFunc = f;
		}

	protected:
	private:
		std::shared_ptr<StateMachine<MessageSerializer>> m_spStateMachine;

		asio::streambuf mDataStreamReceive;
		asio::streambuf mDataStreamSend;

		std::mutex mMutexRec;
		std::mutex mMutexSend;

		std::shared_ptr<BaseMessageDispatcher> m_spDispatcher;

		std::function<void(asio::streambuf&)> m_fSendFunc;

		char mTempRecvBuffer[TEMP_BUFFER_SIZE];
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

}


#endif // MessageSerializer_h__
