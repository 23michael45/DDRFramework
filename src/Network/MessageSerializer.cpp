#include "MessageSerializer.h"
#include "../Utility/DDRMacro.h"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "../../proto/BaseCmd.pb.h"


#define CPPFSM_TEST_ENABLE
using namespace google::protobuf;
using namespace DDRCommProto;
namespace DDRFramework
{
	std::shared_ptr<google::protobuf::Message> CreateMessage(const std::string &type_name)
	{
		auto pmsg = std::shared_ptr<google::protobuf::Message>(NULL);
		const Descriptor* descriptor =
			DescriptorPool::generated_pool()->FindMessageTypeByName(type_name);
		if (descriptor) {
			const google::protobuf::Message* prototype =
				MessageFactory::generated_factory()->GetPrototype(descriptor);
			if (prototype) {
				pmsg = std::shared_ptr<google::protobuf::Message>(prototype->New());
			}
		}
		return pmsg;
	}


	MessageSerializer::MessageSerializer()
	{
	}

	MessageSerializer::~MessageSerializer()
	{
		Deinit();
		DebugLog("\nSerializer Destroy");
	}


	

	void MessageSerializer::Init()
	{

		m_spStateMachine = std::make_shared<StateMachine<MessageSerializer>>();
		ADD_STATE(m_spStateMachine, ParsePBHState)
		ADD_STATE(m_spStateMachine, ParseLengthState)
		ADD_STATE(m_spStateMachine, ParseHeadState)
		ADD_STATE(m_spStateMachine, ParseBodyState)


		m_spStateMachine->enterState<ParsePBHState>();

	}

	void MessageSerializer::Deinit()
	{
		m_spDispatcher.reset();
		m_spStateMachine.reset();
	}
	void MessageSerializer::Update()
	{

		if (m_spStateMachine)
		{
			std::lock_guard<std::mutex> lock(mMutexRec);
			m_spStateMachine->updateWithDeltaTime();

		}

		if (mDataStreamSend.size() == 0)
		{
			return;
		}
		if (m_fSendFunc)
		{
			m_fSendFunc(mDataStreamSend);

		}
	}

	bool MessageSerializer::Pack(google::protobuf::Message& msg)
	{

		std::lock_guard<std::mutex> lock(mMutexSend);

		string stype = msg.GetTypeName();

		int bodylen = msg.ByteSize();


		CommonHeader commonHeader;
		commonHeader.set_bodytype(stype);


		int headlen = commonHeader.ByteSize();

		int totallen = 8 + headlen + bodylen;//+10 means Encrypt head and body 


		std::ostream oshold(&mDataStreamSend);
		google::protobuf::io::OstreamOutputStream oos(&oshold);
		google::protobuf::io::CodedOutputStream cos(&oos);


		cos.WriteRaw(HeadSignal, sizeof(int));
		cos.WriteLittleEndian32(totallen);
		cos.WriteLittleEndian32(headlen);

		commonHeader.SerializeToCodedStream(&cos);
		msg.SerializeToCodedStream(&cos);


		//DebugLog("\ntotal send:%i", totallen+4)


		return true;
	}

	void MessageSerializer::Receive(asio::streambuf& buf)
	{
		std::lock_guard<std::mutex> lock(mMutexRec);
		std::istream is(&buf);

		std::ostream oshold(&mDataStreamReceive);
		google::protobuf::io::OstreamOutputStream oos(&oshold);
		google::protobuf::io::CodedOutputStream cos(&oos);


		while (buf.size() > 0)
		{
			int readSize = std::min((int)(buf.size()), (int)TEMP_BUFFER_SIZE);
			is.read(mTempRecvBuffer, readSize);

			cos.WriteRaw(mTempRecvBuffer, readSize);
			oshold.flush();
		}


	}

	void MessageSerializer::Dispatch(std::shared_ptr<CommonHeader> spHeader, std::shared_ptr<google::protobuf::Message> spMsg)
	{
		if (m_spDispatcher)
		{
			m_spDispatcher->Dispatch(spHeader,spMsg);
		}
	}

	void ParsePBHState::updateWithDeltaTime(float delta)
	{
		//DebugLog("\nParsePBHState");

		char readhead[4] = { 0 };
		asio::streambuf& buf = m_spParentObject.lock()->GetRecBuf();
		
		if (buf.size() < sizeof(int))
		{
			return;
		}


		if (memcmp(HeadSignal, buf.data().data(), 4) != 0)//not head
		{
			buf.consume(1);
			return;
		}
		buf.consume(sizeof(int));


		m_spParentStateMachine.lock()->enterState<ParseLengthState>();
	}
	void ParseLengthState::updateWithDeltaTime(float delta)
	{
		//DebugLog("\nParseLengthState");


		asio::streambuf& buf = m_spParentObject.lock()->GetRecBuf();

		if (buf.size() < sizeof(int) * 2)
		{
			return;
		}


		google::protobuf::uint32 totalLen = 0;
		google::protobuf::uint32 headLen = 0;

		std::istream ishold(&buf);
		google::protobuf::io::IstreamInputStream iistotal(&ishold, sizeof(int) * 2);
		google::protobuf::io::CodedInputStream cistotal(&iistotal);

		cistotal.ReadLittleEndian32(&totalLen);
		cistotal.ReadLittleEndian32(&headLen);

		if (totalLen < 0 || headLen < 0 || totalLen < headLen)
		{
			m_spParentStateMachine.lock()->enterState<ParsePBHState>();
			return;
		}


		std::shared_ptr<ParseHeadState> sp = m_spParentStateMachine.lock()->findState< ParseHeadState>();
		sp->SetLen(totalLen,headLen);
		m_spParentStateMachine.lock()->enterState<ParseHeadState>();

	}
	void ParseHeadState::updateWithDeltaTime(float delta)
	{

		//DebugLog("\nParseHeadState");


		asio::streambuf& buf = m_spParentObject.lock()->GetRecBuf();

		if (buf.size() < m_HeadLen)
		{
			return;
		}

		//std::istream ishold(&buf);


		//google::protobuf::io::IstreamInputStream iis(&ishold, m_HeadLen);
		//google::protobuf::io::CodedInputStream cis(&iis);

		try
		{
			std::shared_ptr<DDRCommProto::CommonHeader> spCommonHeader = std::make_shared<DDRCommProto::CommonHeader>();

			spCommonHeader->ParseFromArray(buf.data().data(),m_HeadLen);
			buf.consume(m_HeadLen);


			int bodyLen = m_TotalLen - m_HeadLen - sizeof(int) * 2;




			//do header logic
			//if message body ignore , 
			bool bIgnoreBody = false;

			//do ignore logic

			if (bIgnoreBody)
			{

				buf.consume(bodyLen);

				m_spParentStateMachine.lock()->enterState<ParsePBHState>();
				return;
			}
			else
			{

				std::shared_ptr<ParseBodyState> sp = m_spParentStateMachine.lock()->findState<ParseBodyState>();
				sp->SetLen(spCommonHeader, bodyLen);

				m_spParentStateMachine.lock()->enterState<ParseBodyState>();

			}

		}
		catch (std::exception e)
		{
			DebugLog("%s---------------------------------------------------------------------------------------Head Deserialize Error", e.what());
			m_spParentStateMachine.lock()->enterState<ParsePBHState>();

		}
		catch (google::protobuf::FatalException* e)
		{
			DebugLog("\nParseHead error %s" , e->message().c_str());
			m_spParentStateMachine.lock()->enterState<ParsePBHState>();
			
		}




	}
	void ParseBodyState::updateWithDeltaTime(float delta)
	{

		//DebugLog("\nParseBodyState");

		asio::streambuf& buf = m_spParentObject.lock()->GetRecBuf();

		if (buf.size() < m_BodyLen)
		{
			return;
		}




		//std::istream ishold(&buf);
		//google::protobuf::io::IstreamInputStream iis(&ishold, m_BodyLen);
		//google::protobuf::io::CodedInputStream cis(&iis);

		try  
		{


			auto spmsg = CreateMessage(m_spCommonHeader->bodytype());

			if (spmsg)
			{
				spmsg->ParseFromArray(buf.data().data(),m_BodyLen);
				buf.consume(m_BodyLen);


				//do body logic

				if (m_spParentObject.lock())
				{
										
					m_spParentObject.lock()->Dispatch(m_spCommonHeader, spmsg);
				}


				m_spParentStateMachine.lock()->enterState<ParsePBHState>();
			}
			else
			{

				DebugLog("---------------------------------------------------------------------------------------Body Deserialize Error");
				m_spParentStateMachine.lock()->enterState<ParsePBHState>();
			}
		}
		catch (std::exception e)
		{
			DebugLog("%s---------------------------------------------------------------------------------------Body Deserialize Error", e.what());
			m_spParentStateMachine.lock()->enterState<ParsePBHState>();

		}
		catch (google::protobuf::FatalException* e)
		{
			DebugLog("\nParseHead error %s", e->message().c_str());
			m_spParentStateMachine.lock()->enterState<ParsePBHState>();

		}


	}


}