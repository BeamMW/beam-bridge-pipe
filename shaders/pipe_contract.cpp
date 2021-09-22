#include "Shaders/common.h"
#include "Shaders/Math.h"
#include "pipe_contract.h"
#include "Shaders/Ethash.h"
#include "Shaders/Eth.h"

// Method_0 - constructor, called once when the contract is deployed
BEAM_EXPORT void Ctor(const Pipe::Create& args)
{
    Pipe::Params params;
    
    params.m_Aid = Env::AssetCreate(&args + 1, args.m_MetadataSize);
    Env::Halt_if(!params.m_Aid);

    Env::SaveVar_T(Pipe::PARAMS_KEY, params);
}

// Method_1 - destructor, called once when the contract is destroyed
BEAM_EXPORT void Dtor(void*)
{
}

BEAM_EXPORT void Method_2(const Pipe::SetRemote& args)
{
    Pipe::Params params;
    Env::LoadVar_T(Pipe::PARAMS_KEY, params);

    _POD_(params.m_Remote) = args.m_Remote;

    Env::SaveVar_T(Pipe::PARAMS_KEY, params);
}

BEAM_EXPORT void Method_3(const Pipe::SendFunds&)
{
    uint32_t localMsgCounter = 0;
    Env::LoadVar_T(Pipe::LOCAL_MSG_COUNTER_KEY, localMsgCounter);

    Env::SaveVar_T(Pipe::LOCAL_MSG_COUNTER_KEY, localMsgCounter);
}

BEAM_EXPORT void Method_4(const Pipe::ReceiveFunds&)
{

}

BEAM_EXPORT void Method_5(const Pipe::PushRemote&)
{

}

BEAM_EXPORT void Method_6(const Pipe::PayFee&)
{

}

BEAM_EXPORT void Method_7(const Pipe::StartDispute&)
{

}

BEAM_EXPORT void Method_8(const Pipe::ContinueDispute&)
{

}

BEAM_EXPORT void Method_9(const Pipe::FinilizeRemoteMsg&)
{

}
