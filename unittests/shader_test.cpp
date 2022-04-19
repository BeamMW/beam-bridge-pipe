// Copyright 2018-2021 The Beam Team
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#define HOST_BUILD

#include "../beam/core/block_rw.h"
#include "../beam/core/keccak.h"
#include "../beam/bvm/bvm2.h"
#include "../beam/bvm/bvm2_impl.h"
#include <cmath>

namespace Shaders
{
#ifdef _MSC_VER
#	pragma warning (disable : 4200 4702) // unreachable code
#endif // _MSC_VER

#define BEAM_EXPORT

#include "Shaders/common.h"
#include "Shaders/BeamHeader.h"
#include "Shaders/Eth.h"

#include "../shaders/token_contract.h"
#include "../shaders/pipe_contract.h"

	template <bool bToShader> void Convert(Token::Create& x)
	{
		ConvertOrd<bToShader>(x.m_MetadataSize);
	}

	template <bool bToShader> void Convert(Token::Init& x)
	{
	}

	template <bool bToShader> void Convert(Token::ChangeManager& x)
	{
	}

	template <bool bToShader> void Convert(Token::Mint& x)
	{
		ConvertOrd<bToShader>(x.m_Amount);
	}

	template <bool bToShader> void Convert(Token::Burn& x)
	{
		ConvertOrd<bToShader>(x.m_Amount);
	}

	template <bool bToShader> void Convert(Pipe::Create& x)
	{
		ConvertOrd<bToShader>(x.m_AssetID);
	}

	template <bool bToShader> void Convert(Pipe::SetRelayer& x)
	{
	}

	template <bool bToShader> void Convert(Pipe::PushRemote& x)
	{
		ConvertOrd<bToShader>(x.m_MsgId);
		ConvertOrd<bToShader>(x.m_RemoteMsg.m_Amount);
		ConvertOrd<bToShader>(x.m_RemoteMsg.m_RelayerFee);
	}

	template <bool bToShader> void Convert(Pipe::ReceiveFunds& x)
	{
		ConvertOrd<bToShader>(x.m_MsgId);
	}

	template <bool bToShader> void Convert(Pipe::SendFunds& x)
	{
		ConvertOrd<bToShader>(x.m_Amount);
		ConvertOrd<bToShader>(x.m_RelayerFee);
	}

	namespace Env
	{
		void CallFarN(const ContractID& cid, uint32_t iMethod, void* pArgs, uint32_t nArgs, uint8_t bInheritContext);

		template <typename T>
		void CallFar_T(const ContractID& cid, T& args, uint8_t bInheritContext = 0)
		{
			//Convert<true>(args);
			CallFarN(cid, args.s_iMethod, &args, sizeof(args), bInheritContext);
			//Convert<false>(args);
		}
	}

	namespace Token
	{
#include "../shaders/token_contract_sid.i"
#include "../shaders/token_contract.cpp"
	} // namespace Token

	namespace Pipe
	{
#include "../shaders/pipe_contract_sid.i"
#include "../shaders/pipe_contract.cpp"
	} // namespace Pipe

#ifdef _MSC_VER
#	pragma warning (default : 4200 4702)
#endif // _MSC_VER
} // namespace Shaders

int g_TestsFailed = 0;

void TestFailed(const char* szExpr, uint32_t nLine)
{
	printf("Test failed! Line=%u, Expression: %s\n", nLine, szExpr);
	g_TestsFailed++;
	fflush(stdout);
}

#define verify_test(x) \
	do { \
		if (!(x)) \
			TestFailed(#x, __LINE__); \
	} while (false)

#define fail_test(msg) TestFailed(msg, __LINE__)

using namespace beam;
using namespace beam::bvm2;

#include "unittest/contract_test_processor.h"

namespace beam
{
	namespace bvm2 
	{
		struct MyProcessor
			: public ContractTestProcessor
		{
			struct Code
			{
				ByteBuffer m_Token;
				ByteBuffer m_Pipe;
			} m_Code;

			ContractID m_cidToken;
			ContractID m_cidPipe;

			void CallFar(const ContractID& cid, uint32_t iMethod, Wasm::Word pArgs, uint8_t bInheritContext) override
			{

				//if (cid == m_cidPipe)
				//{
				//	TempFrame f(*this, cid);
				//	switch (iMethod)
				//	{
				//	case 0: Shaders::Pipe::Ctor(CastArg<Shaders::Pipe::Create>(pArgs)); return;
				//	//case 2: Shaders::Pipe::Method_2(CastArg<Shaders::Pipe::PushLocal>(pArgs)); return;
				//	}
				//}

				ProcessorContract::CallFar(cid, iMethod, pArgs, bInheritContext);
			}

			template<class Key, class Value>
			Value ReadValue(ContractID cid, const Key& key, uint8_t tag = VarKey::Tag::Internal)
			{
				VarKey varKey;
				varKey.Set(cid);
				varKey.Append(tag, Blob(&key, sizeof(Key)));

				Blob buffer;

				LoadVar(Blob(varKey.m_p, varKey.m_Size), buffer);

				auto result = static_cast<const Value*>(buffer.p);

				return *result;
			}

			void TestTokenCreation();
			void TestToken();
			void TestPipeCreation();
			void TestPushRemote();
			void TestReceiveFunds();
			void TestSendFunds();
			void TestChangeManager();

			void TestAll();
		};

		template <>
		struct MyProcessor::Converter<beam::Zero_>
			:public Blob
		{
			Converter(beam::Zero_&)
			{
				p = nullptr;
				n = 0;
			}
		};


		void MyProcessor::TestAll()
		{
			AddCode(m_Code.m_Token, "token_contract.wasm");
			AddCode(m_Code.m_Pipe, "pipe_contract.wasm");

			TestTokenCreation();
			TestToken();
			TestPipeCreation();
			TestPushRemote();
			TestReceiveFunds();
			TestSendFunds();
			TestChangeManager();
		}

		struct CidTxt
		{
			char m_szBuf[Shaders::ContractID::nBytes * 5];

			void Set(const Shaders::ContractID& x)
			{
				char* p = m_szBuf;
				for (uint32_t i = 0; i < x.nBytes; i++)
				{
					if (i)
						*p++ = ',';

					*p++ = '0';
					*p++ = 'x';

					uintBigImpl::_Print(x.m_pData + i, 1, p);
					p += 2;
				}

				assert(p - m_szBuf < (long int)_countof(m_szBuf));
				*p = 0;
			}
		};

		static void VerifyId(const ContractID& cidExp, const ContractID& cid, const char* szName)
		{
			if (cidExp != cid)
			{
				CidTxt ct;
				ct.Set(cid);

				printf("Incorrect %s. Actual value: %s\n", szName, ct.m_szBuf);
				g_TestsFailed++;
				fflush(stdout);
			}
		}

#define VERIFY_ID(exp, actual) VerifyId(exp, actual, #exp)		

		void MyProcessor::TestTokenCreation()
		{
			const char metadata[] = "STD:SCH_VER=1;N=DemoX Coin;SN=DemoX;UN=DEMOX;NTHUN=DGROTH";
			const uint32_t metadataSize = sizeof(metadata);

#pragma pack (push, 1)
			struct Arg : public Shaders::Token::Create
			{
				char metadata[metadataSize];
			};
			Arg args;
#pragma pack (pop)

			memcpy(args.metadata, metadata, metadataSize);
			args.m_MetadataSize = metadataSize;

			verify_test(ContractCreate_T(m_cidToken, m_Code.m_Token, args));

			bvm2::ShaderID sid;
			bvm2::get_ShaderID(sid, m_Code.m_Token);
			VERIFY_ID(Shaders::Token::s_SID, sid);

			Shaders::Token::Init initArgs;

			// it is not used in the test
			//Shaders::Env::DerivePk(initArgs.m_Owner, &m_cidToken, sizeof(m_cidToken));

			verify_test(RunGuarded_T(m_cidToken, initArgs.s_iMethod, initArgs));

			auto params = ReadValue<uint8_t, Shaders::Token::Params>(m_cidToken, Shaders::Token::PARAMS_KEY);

			verify_test(params.m_IsInit);
			verify_test(params.m_AssetID == 1);
			verify_test(params.m_Owner == initArgs.m_Owner);
		}

		void MyProcessor::TestToken()
		{
			Shaders::Token::Mint mintArgs;

			mintArgs.m_Amount = 100000000ULL;

			verify_test(!RunGuarded_T(m_cidToken, mintArgs.s_iMethod, mintArgs));

			Shaders::Token::Burn burnArgs;

			burnArgs.m_Amount = 100000000ULL;

			verify_test(!RunGuarded_T(m_cidToken, burnArgs.s_iMethod, burnArgs));
		}

		void MyProcessor::TestPipeCreation()
		{
			Shaders::Pipe::Create createArgs;

			createArgs.m_TokenCID = m_cidToken;
			createArgs.m_AssetID = 1;

			verify_test(ContractCreate_T(m_cidPipe, m_Code.m_Pipe, createArgs));

			bvm2::ShaderID sid;
			bvm2::get_ShaderID(sid, m_Code.m_Pipe);
			VERIFY_ID(Shaders::Pipe::s_SID, sid);

			Shaders::Token::ChangeManager managerArgs;

			managerArgs.m_NewManager = m_cidPipe;
			verify_test(RunGuarded_T(m_cidToken, managerArgs.s_iMethod, managerArgs));

			Shaders::Pipe::SetRelayer relayerArgs;
			// it is not used in the test
			//Shaders::Env::DerivePk(relayerArgs.m_Relayer, &m_cidPipe, sizeof(m_cidPipe));
			verify_test(RunGuarded_T(m_cidPipe, relayerArgs.s_iMethod, relayerArgs));

			{
				auto params = ReadValue<uint8_t, Shaders::Token::Params>(m_cidToken, Shaders::Token::PARAMS_KEY);

				verify_test(params.m_Manager == m_cidPipe);
			}

			{
				auto params = ReadValue<uint8_t, Shaders::Pipe::Params>(m_cidPipe, Shaders::Pipe::PARAMS_KEY);

				verify_test(params.m_Relayer == relayerArgs.m_Relayer);
				verify_test(params.m_TokenCID == m_cidToken);
				verify_test(params.m_AssetID == 1);
			}
		}

		void MyProcessor::TestPushRemote()
		{
			Shaders::Pipe::PushRemote pushRemoteArgs;

			pushRemoteArgs.m_MsgId = 1;
			pushRemoteArgs.m_RemoteMsg.m_Amount = 1000000000ULL;
			pushRemoteArgs.m_RemoteMsg.m_RelayerFee = 100000000ULL;
			// it is not used in the test
			//Shaders::Env::DerivePk(pushRemoteArgs.m_RemoteMsg.m_UserPK, &m_cidPipe, sizeof(m_cidPipe));

			verify_test(RunGuarded_T(m_cidPipe, pushRemoteArgs.s_iMethod, pushRemoteArgs));

			Shaders::Pipe::RemoteMsgHdr::Key key;

			key.m_MsgId_BE = Shaders::Utils::FromBE(1ULL);

			auto remoteMsg = ReadValue<Shaders::Pipe::RemoteMsgHdr::Key, Shaders::Pipe::RemoteMsgHdr>(m_cidPipe, key);

			verify_test(remoteMsg.m_Amount == pushRemoteArgs.m_RemoteMsg.m_Amount);
			verify_test(remoteMsg.m_RelayerFee == pushRemoteArgs.m_RemoteMsg.m_RelayerFee);
			verify_test(remoteMsg.m_UserPK == pushRemoteArgs.m_RemoteMsg.m_UserPK);

			// try to push message once again
			verify_test(!RunGuarded_T(m_cidPipe, pushRemoteArgs.s_iMethod, pushRemoteArgs));
		}

		void MyProcessor::TestReceiveFunds()
		{
			Shaders::Pipe::ReceiveFunds receiveArgs;

			receiveArgs.m_MsgId = 1;

			verify_test(RunGuarded_T(m_cidPipe, receiveArgs.s_iMethod, receiveArgs));
			// try double spending
			verify_test(!RunGuarded_T(m_cidPipe, receiveArgs.s_iMethod, receiveArgs));

			// try to process unknown message
			receiveArgs.m_MsgId = 3;
			verify_test(!RunGuarded_T(m_cidPipe, receiveArgs.s_iMethod, receiveArgs));
		}

		void MyProcessor::TestSendFunds()
		{
			Shaders::Pipe::SendFunds sendArgs;

			sendArgs.m_Amount = 1000000000ULL;
			sendArgs.m_RelayerFee = 100000000ULL;
			sendArgs.m_Receiver.Scan("ea674fdde714fd979de3edf0f56aa9716b898ec8");

			verify_test(RunGuarded_T(m_cidPipe, sendArgs.s_iMethod, sendArgs));

			Shaders::Pipe::LocalMsgHdr::Key key;

			key.m_MsgId_BE = Shaders::Utils::FromBE(1ULL);

			auto localMsg = ReadValue<Shaders::Pipe::LocalMsgHdr::Key, Shaders::Pipe::LocalMsgHdr>(m_cidPipe, key);

			verify_test(localMsg.m_Amount == sendArgs.m_Amount);
			verify_test(localMsg.m_RelayerFee == sendArgs.m_RelayerFee);
			verify_test(localMsg.m_Receiver == sendArgs.m_Receiver);
		}

		void MyProcessor::TestChangeManager()
		{
			Shaders::Token::ChangeManager managerArgs;

			managerArgs.m_NewManager = m_cidToken;
			verify_test(RunGuarded_T(m_cidToken, managerArgs.s_iMethod, managerArgs));

			Shaders::Pipe::SendFunds sendArgs;

			sendArgs.m_Amount = 1000000000ULL;
			sendArgs.m_RelayerFee = 100000000ULL;
			sendArgs.m_Receiver.Scan("ea674fdde714fd979de3edf0f56aa9716b898ec8");

			verify_test(!RunGuarded_T(m_cidPipe, sendArgs.s_iMethod, sendArgs));

			Shaders::Pipe::PushRemote pushRemoteArgs;

			pushRemoteArgs.m_MsgId = 2;
			pushRemoteArgs.m_RemoteMsg.m_Amount = 1000000000ULL;
			pushRemoteArgs.m_RemoteMsg.m_RelayerFee = 100000000ULL;
			// it is not used in the test
			//Shaders::Env::DerivePk(pushRemoteArgs.m_RemoteMsg.m_UserPK, &m_cidPipe, sizeof(m_cidPipe));

			verify_test(!RunGuarded_T(m_cidPipe, pushRemoteArgs.s_iMethod, pushRemoteArgs));

			Shaders::Pipe::ReceiveFunds receiveArgs;

			receiveArgs.m_MsgId = pushRemoteArgs.m_MsgId;

			verify_test(!RunGuarded_T(m_cidPipe, receiveArgs.s_iMethod, receiveArgs));

			// restore manager to pipe shader
			managerArgs.m_NewManager = m_cidPipe;
			verify_test(RunGuarded_T(m_cidToken, managerArgs.s_iMethod, managerArgs));

			verify_test(!RunGuarded_T(m_cidPipe, receiveArgs.s_iMethod, receiveArgs));
		}
	} // namespace bvm2
} // namespace beam

void Shaders::Env::CallFarN(const ContractID& cid, uint32_t iMethod, void* pArgs, uint32_t nArgs, uint8_t bInheritContext)
{
	Cast::Up<beam::bvm2::MyProcessor>(g_pEnv)->CallFarN(cid, iMethod, pArgs, nArgs, bInheritContext);
}

int main()
{
	try
	{
		ECC::PseudoRandomGenerator prg;
		ECC::PseudoRandomGenerator::Scope scope(&prg);

		MyProcessor proc;

		proc.TestAll();
	}
	catch (const std::exception& ex)
	{
		printf("Expression: %s\n", ex.what());
		g_TestsFailed++;
	}

	return g_TestsFailed ? -1 : 0;
}
