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

namespace Shaders {

#ifdef _MSC_VER
#	pragma warning (disable : 4200 4702) // unreachable code
#endif // _MSC_VER

#define BEAM_EXPORT

#include "Shaders/common.h"
#include "Shaders/BeamHeader.h"
#include "Shaders/Eth.h"

#include "../shaders/pipe_contract.h"
#include "../shaders/mirrortoken_contract.h"


	template <bool bToShader> void Convert(MirrorToken::Create& x) {
		ConvertOrd<bToShader>(x.m_MetadataSize);
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


	namespace Pipe {
#include "../shaders/pipe_contract_sid.i"
#include "../shaders/pipe_contract.cpp"
	}

	namespace MirrorToken {
#include "../shaders/mirrortoken_contract_sid.i"
#include "../shaders/mirrortoken_contract.cpp"
	}

#ifdef _MSC_VER
#	pragma warning (default : 4200 4702)
#endif // _MSC_VER
}

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

namespace beam {
	namespace bvm2 {

		struct MyProcessor
			:public ContractTestProcessor
		{

			struct Code
			{
				ByteBuffer m_Pipe;
				ByteBuffer m_Mirrortoken;

			} m_Code;

			ContractID m_cidPipe;
			ContractID m_cidMirrortoken;

			void CallFar(const ContractID& cid, uint32_t iMethod, Wasm::Word pArgs, uint8_t bInheritContext) override
			{

				if (cid == m_cidPipe)
				{
					/*TempFrame f(*this, cid);
					switch (iMethod)
					{
					case 0: Shaders::Pipe::Ctor(nullptr); return;
					case 2: Shaders::Pipe::Method_2(CastArg<Shaders::Pipe::PushLocal>(pArgs)); return;
					}*/
				}

				ProcessorContract::CallFar(cid, iMethod, pArgs, bInheritContext);
			}


			void TestPipe();
			void TestMirrortoken();

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
			AddCode(m_Code.m_Pipe, "pipe_contract.wasm");
			AddCode(m_Code.m_Mirrortoken, "mirrortoken_contract.wasm");

			TestPipe();
			TestMirrortoken();
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

		void MyProcessor::TestPipe()
		{
			Zero_ zero;
			verify_test(ContractCreate_T(m_cidPipe, m_Code.m_Pipe, zero));

			bvm2::ShaderID sid;
			bvm2::get_ShaderID(sid, m_Code.m_Pipe);
			VERIFY_ID(Shaders::Pipe::s_SID, sid);
		}

		void MyProcessor::TestMirrortoken()
		{
			const std::string metadata("testcoin");
			auto* args = (Shaders::MirrorToken::Create*)malloc(sizeof(Shaders::MirrorToken::Create) + metadata.size());
			args->m_PipeID = m_cidPipe;
			args->m_MetadataSize = metadata.size();
			memcpy((args + 1), metadata.c_str(), metadata.size());
			verify_test(ContractCreate_T(m_cidMirrortoken, m_Code.m_Mirrortoken, *args));

			free(args);

			bvm2::ShaderID sid;
			bvm2::get_ShaderID(sid, m_Code.m_Mirrortoken);
			VERIFY_ID(Shaders::MirrorToken::s_SID, sid);
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
