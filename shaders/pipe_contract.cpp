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
    _POD_(msg.m_Receiver) = args.m_Receiver;
    _POD_(msg.m_ContractReceiver) = params.m_Remote;
    
    Env::get_CallerCid(0, msg.m_ContractSender);
    Env::SaveVar(&msgKey, sizeof(msgKey), &msg, sizeof(msg), KeyTag::Internal);

    Env::SaveVar_T(Pipe::LOCAL_MSG_COUNTER_KEY, localMsgCounter);

    Env::FundsLock(params.m_Aid, (msg.m_Amount + msg.m_RelayerFee));
}

BEAM_EXPORT void Method_4(const Pipe::ReceiveFunds& args)
{
    Pipe::RemoteMsgHdr::Key keyMsg;
    keyMsg.m_MsgId_BE = Utils::FromBE(args.m_MsgId);

    Pipe::RemoteMsgHdr msg;
    Env::LoadVar(&keyMsg, sizeof(keyMsg), &msg, sizeof(msg), KeyTag::Internal);

    Env::Halt_if(!msg.m_Finalized);

    Pipe::Params params;
    Env::LoadVar_T(Pipe::PARAMS_KEY, params);

    // mint asset
    Env::AssetEmit(params.m_Aid, msg.m_Amount, 1);

    Env::FundsUnlock(params.m_Aid, msg.m_Amount);
    Env::AddSig(msg.m_UserPK);
}

BEAM_EXPORT void Method_5(const Pipe::PushRemote& args)
{
    // TODO check lock funds
    Pipe::Params params;
    Env::LoadVar_T(Pipe::PARAMS_KEY, params);

    Env::Halt_if(_POD_(args.m_RemoteMsg.m_ContractSender) != params.m_Remote);
    // TODO maybe need check receiver contract address???

    Pipe::RemoteMsgHdr::Key keyMsg;
    keyMsg.m_MsgId_BE = Utils::FromBE(args.m_MsgId);

    Pipe::RemoteMsgHdr msg;
    _POD_(msg) = args.m_RemoteMsg;

    msg.m_Finalized = false;

    Env::SaveVar(&keyMsg, sizeof(keyMsg), &msg, sizeof(msg), KeyTag::Internal);
    Env::FundsLock(0, Pipe::RELAYER_DEPOSIT);
}

BEAM_EXPORT void Method_6(const Pipe::StartDispute&)
{

}

BEAM_EXPORT void Method_7(const Pipe::ContinueDispute&)
{

}

BEAM_EXPORT void Method_8(const Pipe::FinalizeDispute&)
{

}

BEAM_EXPORT void Method_9(const Pipe::FinilizeRemoteMsg& args)
{
    // TODO unlock locked funds in PushRemote
    Pipe::RemoteMsgHdr::Key keyMsg;
    keyMsg.m_MsgId_BE = Utils::FromBE(args.m_MsgId);

    Pipe::RemoteMsgHdr msg;
    Env::LoadVar(&keyMsg, sizeof(keyMsg), &msg, sizeof(msg), KeyTag::Internal);

    msg.m_Finalized = true;

    Env::SaveVar(&keyMsg, sizeof(keyMsg), &msg, sizeof(msg), KeyTag::Internal);
    Env::FundsUnlock(0, Pipe::RELAYER_DEPOSIT);

    Pipe::Params params;
    Env::LoadVar_T(Pipe::PARAMS_KEY, params);

    // mint asset
    Env::AssetEmit(params.m_Aid, msg.m_RelayerFee, 1);

    Env::FundsUnlock(params.m_Aid, msg.m_RelayerFee);
}
