#include "MessageSerializer.h"
#include "../Utility/DDRMacro.h"

#include "../Utility/Logger.h"
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "../../proto/BaseCmd.pb.h"
#include "../Utility/CommonFunc.h"

#define PROTOBUF_ENCRYPT_LEN 5

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
		m_TotalPackLen = 0;
	}

	MessageSerializer::~MessageSerializer()
	{
		DebugLog("\nSerializer Destroy");
		Deinit();
	}


	

	void MessageSerializer::Init()
	{

		m_spStateMachine = std::make_shared<StateMachine<MessageSerializer>>();
		ADD_STATE(m_spStateMachine, ParsePBHState)
		ADD_STATE(m_spStateMachine, ParseLengthState)
		ADD_STATE(m_spStateMachine, ParseHeadState)
		ADD_STATE(m_spStateMachine, ParseBodyState)
		ADD_STATE(m_spStateMachine, WaitNextBuffState)


		m_spStateMachine->enterState<ParsePBHState>();

	}

	void MessageSerializer::Deinit()
	{

		while (!mDataStreamSendQueue.empty()) mDataStreamSendQueue.pop();
		if (m_spDispatcher)
		{

			m_spDispatcher.reset();
		}
		if (m_spStateMachine)
		{

			m_spStateMachine.reset();
		}
	}
	void MessageSerializer::Update()
	{
		std::lock_guard<std::mutex> lock(mMutexUpdate);

		if (m_spStateMachine && this)
		{
			std::lock_guard<std::mutex> lock(mMutexRec);

			auto pCurState = m_spStateMachine->getState();
			auto spWaitState = m_spStateMachine->findStateByName(typeid(WaitNextBuffState).name());
			do
			{
				if (m_spStateMachine)
				{
					m_spStateMachine->updateWithDeltaTime();

					pCurState = m_spStateMachine->getState();//must put after updateWithDeltaTime,cause state may change in that function
				}

			} while (pCurState != spWaitState.get());

		}

	}

	std::shared_ptr<asio::streambuf> MessageSerializer::SerializeMsg(std::shared_ptr<google::protobuf::Message> spmsg)
	{
		return SerializeMsg(nullptr,spmsg);
	}
	std::shared_ptr<asio::streambuf> MessageSerializer::SerializeMsg(std::shared_ptr<DDRCommProto::CommonHeader> spheader, std::shared_ptr<google::protobuf::Message> spmsg)
	{
		auto spbuf = std::make_shared<asio::streambuf>();

		//DebugLog("\nStart Pack");
		string stype = spmsg->GetTypeName();

		int bodylen = spmsg->ByteSize();


		CommonHeader commonHeader;
		if (spheader)
		{
			commonHeader.CopyFrom(*(spheader.get()));
		}

		commonHeader.set_bodytype(stype);


		int headlen = commonHeader.ByteSize();

		int totallen = 8 + headlen + bodylen;//+10 means Encrypt head and body 

		std::ostream oshold(spbuf.get());
		google::protobuf::io::OstreamOutputStream oos(&oshold);
		google::protobuf::io::CodedOutputStream cos(&oos);


		cos.WriteRaw(HeadSignal, sizeof(int));

#ifdef PROTOBUF_ENCRYPT

		cos.WriteLittleEndian32(totallen + 2 * PROTOBUF_ENCRYPT_LEN);
		cos.WriteLittleEndian32(headlen + PROTOBUF_ENCRYPT_LEN);

		asio::streambuf temp_headbuf;
		std::ostream temp_heados(&temp_headbuf);
		commonHeader.SerializeToOstream(&temp_heados);
		temp_heados.flush();
		
		asio::streambuf temp_headbufencrypt;
		temp_headbufencrypt.prepare(temp_headbuf.size() + PROTOBUF_ENCRYPT_LEN);

		auto temp_headrawbuf = asio::buffer(temp_headbufencrypt.prepare(temp_headbuf.size() + PROTOBUF_ENCRYPT_LEN));
	
		DDRFramework::Txt_Encrypt(temp_headbuf.data().data(), temp_headbuf.size(), temp_headrawbuf.data(), temp_headbuf.size() + PROTOBUF_ENCRYPT_LEN);

		cos.WriteRaw(temp_headrawbuf.data(), temp_headrawbuf.size());



		asio::streambuf temp_bodybuf;
		std::ostream temp_bodyos(&temp_bodybuf);
		spmsg->SerializeToOstream(&temp_bodyos);
		temp_bodyos.flush();




		asio::streambuf temp_bodybufencrypt;
		temp_bodybufencrypt.prepare(temp_bodybuf.size() + PROTOBUF_ENCRYPT_LEN);
		auto temp_bodyrawbuf = asio::buffer(temp_headbufencrypt.prepare(temp_bodybuf.size() + PROTOBUF_ENCRYPT_LEN));


		DDRFramework::Txt_Encrypt(temp_bodybuf.data().data(), temp_bodybuf.size(), temp_bodyrawbuf.data(), temp_bodybuf.size() + PROTOBUF_ENCRYPT_LEN);

		cos.WriteRaw(temp_bodyrawbuf.data(), temp_bodyrawbuf.size());

#else
		cos.WriteLittleEndian32(totallen);
		cos.WriteLittleEndian32(headlen);

		commonHeader.SerializeToCodedStream(&cos);
		spmsg->SerializeToCodedStream(&cos);
#endif
		return spbuf;

	}

	bool MessageSerializer::Pack(std::shared_ptr<DDRCommProto::CommonHeader> spheader, std::shared_ptr<google::protobuf::Message> spmsg)
	{

		std::lock_guard<std::mutex> lock(mMutexSend);

		auto spbuf = SerializeMsg(spheader,spmsg);
		mDataStreamSendQueue.push(spbuf);


		m_TotalPackLen += spbuf->size();
		DebugLog("\ntotal Pack Len:%i   Queue Len: %i ", m_TotalPackLen, mDataStreamSendQueue.size())
		//DebugLog("\nEnd Pack");
		return true;
	}
	bool MessageSerializer::IgnoreBody(std::shared_ptr<CommonHeader> spHeader)
	{
		if (m_spHeadRuleRouter)
		{
			return m_spHeadRuleRouter->IgnoreBody(m_spBaseSocketContainer, spHeader);
		}
		return false;
	}

	void MessageSerializer::Dispatch(std::shared_ptr<CommonHeader> spHeader, std::shared_ptr<google::protobuf::Message> spMsg)
	{
		if (m_spDispatcher)
		{
			m_spDispatcher->Dispatch(m_spBaseSocketContainer,spHeader,spMsg);
		}
	}

	void ParsePBHState::updateWithDeltaTime(float delta)
	{
		//DebugLog("\nParsePBHState");

		asio::streambuf& buf = m_spParentObject.lock()->GetRecBuf();

		char readhead[4] = { 0 };
		if (buf.size() < sizeof(int))
		{

			std::shared_ptr<WaitNextBuffState> sp = m_spParentStateMachine.lock()->findState< WaitNextBuffState>();
			sp->SetPreStateAndNextLen(typeid(ParsePBHState).name(), sizeof(int));
			m_spParentStateMachine.lock()->enterState<WaitNextBuffState>();
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
			std::shared_ptr<WaitNextBuffState> sp = m_spParentStateMachine.lock()->findState< WaitNextBuffState>();
			sp->SetPreStateAndNextLen(typeid(ParseLengthState).name(), sizeof(int)*2);
			m_spParentStateMachine.lock()->enterState<WaitNextBuffState>();
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

		asio::streambuf& buf = m_spParentObject.lock()->GetRecBuf();

		if (buf.size() < m_HeadLen)
		{
			std::shared_ptr<WaitNextBuffState> sp = m_spParentStateMachine.lock()->findState< WaitNextBuffState>();
			sp->SetPreStateAndNextLen(typeid(ParseHeadState).name(), m_HeadLen);
			m_spParentStateMachine.lock()->enterState<WaitNextBuffState>();
			return;
		}


		try
		{
			std::shared_ptr<DDRCommProto::CommonHeader> spCommonHeader = std::make_shared<DDRCommProto::CommonHeader>();

#ifdef PROTOBUF_ENCRYPT

			asio::streambuf temp_headbufdecrypt;
			temp_headbufdecrypt.prepare(m_HeadLen - PROTOBUF_ENCRYPT_LEN);


			auto temp_headrawbuf = asio::buffer(temp_headbufdecrypt.prepare(m_HeadLen - PROTOBUF_ENCRYPT_LEN));

			
			DDRFramework::Txt_Decrypt(buf.data().data(), m_HeadLen, temp_headrawbuf.data(), m_HeadLen - PROTOBUF_ENCRYPT_LEN);



			spCommonHeader->ParseFromArray(temp_headrawbuf.data(), m_HeadLen - PROTOBUF_ENCRYPT_LEN);

#else

			spCommonHeader->ParseFromArray(buf.data().data(), m_HeadLen);
#endif
			buf.consume(m_HeadLen);


			int bodyLen = m_TotalLen - m_HeadLen - sizeof(int) * 2;




			//do header logic
			//if message body ignore , 

			bool bIgnoreBody = false;
			if (m_spParentObject.lock())
			{
				bIgnoreBody = m_spParentObject.lock()->IgnoreBody(spCommonHeader);
			}


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
		catch (std::exception& e)
		{
			DebugLog("%s---------------------------------------------------------------------------------------Head Deserialize Error", e.what());
			m_spParentStateMachine.lock()->enterState<ParsePBHState>();

		}
		catch (google::protobuf::FatalException& e)
		{
			DebugLog("\nParseHead error %s" , e.message().c_str());
			m_spParentStateMachine.lock()->enterState<ParsePBHState>();
			
		}




	}
	void ParseBodyState::updateWithDeltaTime(float delta)
	{

		//DebugLog("\nParseBodyState");

		asio::streambuf& buf = m_spParentObject.lock()->GetRecBuf();

		if (buf.size() < m_BodyLen)
		{
			std::shared_ptr<WaitNextBuffState> sp = m_spParentStateMachine.lock()->findState< WaitNextBuffState>();
			sp->SetPreStateAndNextLen(typeid(ParseBodyState).name(), m_BodyLen);
			m_spParentStateMachine.lock()->enterState<WaitNextBuffState>();
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

#ifdef PROTOBUF_ENCRYPT

				asio::streambuf temp_bodybufdecrypt;
				temp_bodybufdecrypt.prepare(m_BodyLen - PROTOBUF_ENCRYPT_LEN);


				auto temp_bodyrawbuf = asio::buffer(temp_bodybufdecrypt.prepare(m_BodyLen - PROTOBUF_ENCRYPT_LEN));


				DDRFramework::Txt_Decrypt(buf.data().data(), m_BodyLen, temp_bodyrawbuf.data(), m_BodyLen - PROTOBUF_ENCRYPT_LEN);

				spmsg->ParseFromArray(temp_bodyrawbuf.data(), m_BodyLen - PROTOBUF_ENCRYPT_LEN);

#else

				spmsg->ParseFromArray(buf.data().data(), m_BodyLen);
#endif
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
		catch (std::exception& e)
		{
			DebugLog("%s---------------------------------------------------------------------------------------Body Dispatch Error", e.what());
			m_spParentStateMachine.lock()->enterState<ParsePBHState>();

		}
		catch (google::protobuf::FatalException& e)
		{
			DebugLog("\nParseHead error %s", e.message().c_str());
			m_spParentStateMachine.lock()->enterState<ParsePBHState>();

		}


	}


	void WaitNextBuffState::updateWithDeltaTime(float delta)
	{

		//DebugLog("\nParseBodyState");

		asio::streambuf& buf = m_spParentObject.lock()->GetRecBuf();

		if (buf.size() < m_NextLen)
		{
			return;
		}
		else
		{
			m_spParentStateMachine.lock()->enterState(m_PreStateName);

		}


	}
}