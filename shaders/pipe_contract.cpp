#include "Shaders/common.h"
#include "Shaders/Math.h"
#include "pipe_contract.h"
#include "Shaders/Ethash.h"
#include "Shaders/Eth.h"
#include "token_contract.h"

// Method_0 - constructor, called once when the contract is deployed
BEAM_EXPORT void Ctor(const Pipe::Create& args)
{
    Pipe::Params params;
    
    params.m_Aid = args.m_Aid;
    _POD_(params.m_TokenID) = args.m_TokenID;

    Env::SaveVar_T(Pipe::PARAMS_KEY, params);
}

// Method_1 - destructor, called once when the contract is destroyed
BEAM_EXPORT void Dtor(void*)
{
}

BEAM_EXPORT void Method_2(const Pipe::SetRelayer& args)
{
    // TODO roman.strilets set only one time
    Pipe::Params params;
    Env::LoadVar_T(Pipe::PARAMS_KEY, params);

    _POD_(params.m_Relayer) = args.m_Relayer;

    Env::SaveVar_T(Pipe::PARAMS_KEY, params);
}

BEAM_EXPORT void Method_3(const Pipe::SendFunds& args)
{
    Pipe::Params params;
    Env::LoadVar_T(Pipe::PARAMS_KEY, params);

    uint32_t localMsgCounter = 0;
    Env::LoadVar_T(Pipe::LOCAL_MSG_COUNTER_KEY, localMsgCounter);

    Pipe::LocalMsgHdr::Key msgKey;
    msgKey.m_MsgId_BE = Utils::FromBE(++localMsgCounter);

    Pipe::LocalMsgHdr msg;
    msg.m_Amount = args.m_Amount;
    msg.m_RelayerFee = args.m_RelayerFee;

    Env::SaveVar_T(msgKey, msg);

    Env::SaveVar_T(Pipe::LOCAL_MSG_COUNTER_KEY, localMsgCounter);

    Token::Burn burn;

    burn.m_Amount = msg.m_Amount + msg.m_RelayerFee;

    Env::CallFar_T(params.m_TokenID, burn);
}

BEAM_EXPORT void Method_4(const Pipe::ReceiveFunds& args)
{
    bool received;
    Env::Halt_if(!Env::LoadVar_T(args.m_MsgId, received));
    Env::Halt_if(received);

    Pipe::RemoteMsgHdr::Key keyMsg;
    keyMsg.m_MsgId_BE = Utils::FromBE(args.m_MsgId);

    Pipe::RemoteMsgHdr msg;
    Env::Halt_if(!Env::LoadVar_T(keyMsg, msg));

    Pipe::Params params;
    Env::LoadVar_T(Pipe::PARAMS_KEY, params);

    // mint asset
    Env::AddSig(msg.m_UserPK);

    Token::Mint mint;

    mint.m_Amount = msg.m_Amount;

    Env::CallFar_T(params.m_TokenID, mint);

    Env::SaveVar_T(args.m_MsgId, true);
}

BEAM_EXPORT void Method_5(const Pipe::PushRemote& args)
{
    bool received;
    // push only onte time
    Env::Halt_if(Env::LoadVar_T(args.m_MsgId, received));

    Pipe::Params params;
    Env::LoadVar_T(Pipe::PARAMS_KEY, params);

    Pipe::RemoteMsgHdr::Key keyMsg;
    keyMsg.m_MsgId_BE = Utils::FromBE(args.m_MsgId);

    Pipe::RemoteMsgHdr msg;
    _POD_(msg) = args.m_RemoteMsg;

    Env::SaveVar_T(keyMsg, msg);

    Env::SaveVar_T(args.m_MsgId, false);

    // mint asset
    Env::AddSig(params.m_Relayer);

    Token::Mint mint;

    mint.m_Amount = msg.m_RelayerFee;

    Env::CallFar_T(params.m_TokenID, mint);
}
