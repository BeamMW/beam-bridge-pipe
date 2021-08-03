#include "Shaders/common.h"
#include "pipe_contract.h"
#include "mirrortoken_contract.h"
#include "Shaders/Eth.h"

BEAM_EXPORT void Ctor(const MirrorToken::Create& r)
{
    MirrorToken::Params params;
    _POD_(params.m_BridgeID) = r.m_BridgeID;
    _POD_(params.m_Remote).SetZero();

    Env::Halt_if(!Env::RefAdd(r.m_BridgeID));

    params.m_Aid = Env::AssetCreate(&r + 1, r.m_MetadataSize);
    Env::Halt_if(!params.m_Aid);

    Env::SaveVar_T(MirrorToken::kParamsKey, params);
}

BEAM_EXPORT void Dtor(void*)
{
    MirrorToken::Params params;
    Env::LoadVar_T(MirrorToken::kParamsKey, params);

    Env::Halt_if(!Env::RefRelease(params.m_BridgeID));
    Env::Halt_if(!Env::AssetDestroy(params.m_Aid));
    Env::DelVar_T(MirrorToken::kParamsKey);
}

BEAM_EXPORT void Method_2(const MirrorToken::SetRemote& r)
{
    MirrorToken::Params params;
    Env::LoadVar_T(MirrorToken::kParamsKey, params);

    // TODO: uncomment after testing
    // Env::Halt_if(!_POD_(params.m_Remote).IsZero() || _POD_(r.m_Remote).IsZero());
    _POD_(params.m_Remote) = r.m_Remote;

    Env::SaveVar_T(MirrorToken::kParamsKey, params);
}

BEAM_EXPORT void Method_3(const MirrorToken::Send& r)
{
    MirrorToken::Params params;
    Env::LoadVar_T(MirrorToken::kParamsKey, params);

    Env::Halt_if(_POD_(params.m_Remote).IsZero());

#pragma pack (push, 1)
    struct Arg :public Bridge::PushLocal
    {
        MirrorToken::OutMessage m_Msg;
    };
#pragma pack (pop)

    Arg arg;
    _POD_(arg.m_ContractReceiver) = params.m_Remote;
    arg.m_MsgSize = sizeof(arg.m_Msg);
    _POD_(arg.m_Msg) = r;

    Env::CallFar_T(params.m_BridgeID, arg);

    Env::FundsLock(params.m_Aid, r.m_Amount);
    Env::AssetEmit(params.m_Aid, r.m_Amount, 0);
}

BEAM_EXPORT void Method_4(const MirrorToken::Receive& r)
{
    MirrorToken::Params params;
    Env::LoadVar_T(MirrorToken::kParamsKey, params);

    Env::Halt_if(_POD_(params.m_Remote).IsZero());

#pragma pack (push, 1)
    struct Arg : public Bridge::ReadRemote
    {
        MirrorToken::InMessage m_Msg;
    };
#pragma pack (pop)

    Arg arg;
    arg.m_MsgId = r.m_MsgId;
    arg.m_MsgSize = sizeof(arg.m_Msg);
    Env::CallFar_T(params.m_BridgeID, arg);

    Env::Halt_if(
        (_POD_(arg.m_ContractSender) != params.m_Remote) ||
        (sizeof(arg.m_Msg) != arg.m_MsgSize)
    );

    arg.m_Msg.m_Amount = Utils::FromBE(arg.m_Msg.m_Amount);

    Env::AssetEmit(params.m_Aid, arg.m_Msg.m_Amount, 1);

    Env::FundsUnlock(params.m_Aid, arg.m_Msg.m_Amount);
    Env::AddSig(arg.m_Msg.m_User);
}

BEAM_EXPORT void Method_5(const MirrorToken::Mint& mint)
{
    MirrorToken::Params params;
    Env::LoadVar_T(MirrorToken::kParamsKey, params);

    Env::AssetEmit(params.m_Aid, mint.m_Amount, 1);
    Env::FundsUnlock(params.m_Aid, mint.m_Amount);
}

