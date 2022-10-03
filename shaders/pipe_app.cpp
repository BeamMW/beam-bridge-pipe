#include "Shaders/common.h"
#include "Shaders/app_common_impl.h"
#include "Shaders/Ethash.h"
#include "pipe_contract.h"

namespace Pipe
{
#include "pipe_contract_sid.i"
}

namespace
{
    const char* TOKEN_CID = "tokenCID";
    const char* TOKEN_AID = "tokenAID";
    const char* CONTRACT_ID = "cid";
    const char* AMOUNT = "amount";
    const char* RELAYER_FEE = "relayerFee";
    const char* RECEIVER = "receiver";
    const char* MSG_ID = "msgId";
    const char* START_FROM = "startFrom";
    const char* RELAYER = "relayer";

    namespace Actions
    {
        const char* CREATE = "create";
        const char* VIEW = "view";
        const char* SET_RELAYER = "set_relayer";
        const char* GET_PK = "get_pk";
        const char* SEND = "send";
        const char* RECEIVE = "receive";
        const char* PUSH_REMOTE = "push_remote";
        const char* VIEW_INCOMING = "view_incoming";
        const char* LOCAL_MSG_COUNT = "local_msg_count";
        const char* LOCAL_MSG = "local_msg";
        const char* REMOTE_MSG = "remote_msg";
    } // namespace Actions

    void OnError(const char* sz)
    {
        Env::DocAddText("error", sz);
    }

    struct ParamsPlus : public Pipe::Params
    {
        bool get(const ContractID& cid)
        {
            Env::Key_T<uint8_t> gk;
            _POD_(gk.m_Prefix.m_Cid) = cid;
            gk.m_KeyInContract = Pipe::PARAMS_KEY;

            if (Env::VarReader::Read_T(gk, *this))
                return true;

            OnError("no params");
            return false;
        }
    };

    struct IncomingWalker
    {
        const ContractID& m_Cid;
        IncomingWalker(const ContractID& cid) :m_Cid(cid) {}

        PubKey m_Relayer;
        Env::VarReaderEx<true> m_Reader;
        Env::Key_T<Pipe::RemoteMsgHdr::Key> m_Key;
        Pipe::RemoteMsgHdr m_Msg;


        bool Restart(uint64_t iStartFrom)
        {
            ParamsPlus params;
            if (!params.get(m_Cid))
                return false;

            m_Relayer = params.m_Relayer;

            Env::Key_T<Pipe::RemoteMsgHdr::Key> k1;
            k1.m_Prefix.m_Cid = m_Cid;
            k1.m_KeyInContract.m_MsgId_BE = Utils::FromBE(iStartFrom);

            auto k2 = k1;
            k2.m_KeyInContract.m_MsgId_BE = -1;

            m_Reader.Enum_T(k1, k2);
            return true;
        }

        bool MoveNext(const PubKey* pPk)
        {
            while (true)
            {
                if (!m_Reader.MoveNext_T(m_Key, m_Msg))
                    return false;

                if (pPk && (_POD_(*pPk) != m_Msg.m_UserPK))
                    continue;

                return true;
            }
        }
    };

    void ViewIncoming(const ContractID& cid, const PubKey* pPk, uint64_t iStartFrom)
    {
        Env::DocArray gr("incoming");

        IncomingWalker wlk(cid);
        if (!wlk.Restart(iStartFrom))
            return;

        while (wlk.MoveNext(pPk))
        {
            Env::DocGroup gr("");
            Env::DocAddNum("MsgId", Utils::FromBE(wlk.m_Key.m_KeyInContract.m_MsgId_BE));
            Env::DocAddNum("amount", wlk.m_Msg.m_Amount);

            if (!pPk)
                Env::DocAddBlob_T("User", wlk.m_Msg.m_UserPK);
        }
    }
} // namespace

namespace manager
{
    void Create()
    {
        Pipe::Create args;
        Env::DocGet(TOKEN_CID, args.m_TokenCID);
        Env::DocGetNum32(TOKEN_AID, &args.m_AssetID);

        Env::GenerateKernel(nullptr, args.s_iMethod, &args, sizeof(args), nullptr, 0, nullptr, 0, "create Pipe contract", 0);
    }

    void View()
    {
        EnumAndDumpContracts(Pipe::s_SID);
    }

    void SetRelayer()
    {
        ContractID cid;
        Env::DocGet(CONTRACT_ID, cid);

        Pipe::SetRelayer args;
        Env::DocGet(RELAYER, args.m_Relayer);

        Env::GenerateKernel(&cid, args.s_iMethod, &args, sizeof(args), nullptr, 0, nullptr, 0, "Set relayer public key", 0);
    }

    void GetPk()
    {
        ContractID cid;
        Env::DocGet(CONTRACT_ID, cid);

        PubKey pk;
        Env::DerivePk(pk, &cid, sizeof(cid));
        Env::DocAddBlob_T("pk", pk);
    }

    void SendFunds()
    {
        ContractID cid;
        Env::DocGet(CONTRACT_ID, cid);

        ParamsPlus params;
        if (!params.get(cid))
            return;
        
        Pipe::SendFunds args;
        Env::DocGetNum64(AMOUNT, &args.m_Amount);
        Env::DocGetNum64(RELAYER_FEE, &args.m_RelayerFee);
        Env::DocGetBlobEx(RECEIVER, &args.m_Receiver, sizeof(args.m_Receiver));

        FundsChange fc;
        fc.m_Aid = params.m_AssetID;
        fc.m_Amount = args.m_Amount + args.m_RelayerFee;
        fc.m_Consume = 1;

        Env::GenerateKernel(&cid, args.s_iMethod, &args, sizeof(args), &fc, 1, nullptr, 0, "Send funds", 0);
    }

    void ReceiveFunds()
    {
        ContractID cid;
        Env::DocGet(CONTRACT_ID, cid);

        ParamsPlus params;
        if (!params.get(cid))
            return;

        Pipe::ReceiveFunds args;
        Env::DocGetNum64(MSG_ID, &args.m_MsgId);

        Env::Key_T<Pipe::RemoteMsgHdr::Key> msgKey;
        Pipe::RemoteMsgHdr msg;

        msgKey.m_Prefix.m_Cid = cid;
        msgKey.m_KeyInContract.m_MsgId_BE = Utils::FromBE(args.m_MsgId);
        
        if (!Env::VarReader::Read_T(msgKey, msg))
        {
            OnError("msg with current id is absent");
            return;
        }

        // check msgId. maybe it is processed
        Env::Key_T<uint64_t> receivedKey;
        receivedKey.m_Prefix.m_Cid = cid;
        receivedKey.m_KeyInContract = args.m_MsgId;

        bool received = false;
        if (!Env::VarReader::Read_T(receivedKey, received))
        {
            OnError("msg with current id is absent");
            return;
        }

        if (received)
        {
            OnError("msg is processed");
            return;
        }

        FundsChange fc;
        fc.m_Aid = params.m_AssetID;
        fc.m_Amount = msg.m_Amount;
        fc.m_Consume = 0;

        SigRequest sig;
        sig.m_pID = &cid;
        sig.m_nID = sizeof(cid);

        Env::GenerateKernel(&cid, args.s_iMethod, &args, sizeof(args), &fc, 1, &sig, 1, "Receive funds", 1200000);
    }

    void PushRemote()
    {
        ContractID cid;
        Env::DocGet(CONTRACT_ID, cid);

        Pipe::PushRemote args;
        Env::DocGetNum64(MSG_ID, &args.m_MsgId);
        Env::DocGetNum64(AMOUNT, &args.m_RemoteMsg.m_Amount);
        Env::DocGetNum64(RELAYER_FEE, &args.m_RemoteMsg.m_RelayerFee);
        Env::DocGet(RECEIVER, args.m_RemoteMsg.m_UserPK);

        // check msgId. maybe it is processed
        Env::Key_T<uint64_t> receivedKey;
        receivedKey.m_Prefix.m_Cid = cid;
        receivedKey.m_KeyInContract = args.m_MsgId;

        bool received;
        if (Env::VarReader::Read_T(receivedKey, received))
        {
            OnError("msg is exist");
            return;
        }

        ParamsPlus params;
        if (!params.get(cid))
            return;

        FundsChange fc;
        fc.m_Aid = params.m_AssetID;
        fc.m_Amount = args.m_RemoteMsg.m_RelayerFee;
        fc.m_Consume = 0;

        SigRequest sig;
        sig.m_pID = &cid;
        sig.m_nID = sizeof(cid);

        Env::GenerateKernel(&cid, args.s_iMethod, &args, sizeof(args), &fc, 1, &sig, 1, "Push remote message", 0);
    }

    void ViewIncomingMsg()
    {
        ContractID cid;
        uint64_t startFrom = 0;
        Env::DocGet(CONTRACT_ID, cid);
        Env::DocGet(START_FROM, startFrom);

        PubKey pk;
        Env::DerivePk(pk, &cid, sizeof(cid));

        ViewIncoming(cid, &pk, startFrom);
    }

    void GetLocalMsgCount()
    {
        ContractID cid;
        Env::DocGet(CONTRACT_ID, cid);

        Env::Key_T<uint8_t> key;
        key.m_KeyInContract = Pipe::LOCAL_MSG_COUNTER_KEY;
        key.m_Prefix.m_Cid = cid;

        uint64_t localMsgCounter = 0;
        Env::VarReader::Read_T(key, localMsgCounter);

        Env::DocAddNum64("count", localMsgCounter);
    }

    void GetLocalMsg()
    {
        ContractID cid;
        uint64_t msgId;
        Env::DocGet(CONTRACT_ID, cid);
        Env::DocGetNum64(MSG_ID, &msgId);

        Env::Key_T<Pipe::LocalMsgHdr::Key> msgKey;
        msgKey.m_Prefix.m_Cid = cid;
        msgKey.m_KeyInContract.m_MsgId_BE = Utils::FromBE(msgId);

        Env::VarReader reader(msgKey, msgKey);

        uint32_t keySize = sizeof(msgKey);
        Pipe::LocalMsgHdr msg;
        uint32_t size = sizeof(msg);
        if (!reader.MoveNext(nullptr, keySize, &msg, size, 0))
        {
            OnError("msg with current id is absent");
            return;
        }

        Env::DocAddNum(AMOUNT, msg.m_Amount);
        Env::DocAddNum(RELAYER_FEE, msg.m_RelayerFee);
        Env::DocAddBlob_T(RECEIVER, msg.m_Receiver);
        Env::DocAddNum("height", msg.m_Height);
    }

    void GetRemoteMsg()
    {
        ContractID cid;
        uint64_t msgId;
        Env::DocGet(CONTRACT_ID, cid);
        Env::DocGetNum64(MSG_ID, &msgId);

        Env::Key_T<Pipe::RemoteMsgHdr::Key> msgKey;
        msgKey.m_Prefix.m_Cid = cid;
        msgKey.m_KeyInContract.m_MsgId_BE = Utils::FromBE(msgId);

        Env::VarReader reader(msgKey, msgKey);

        uint32_t keySize = sizeof(msgKey);
        Pipe::RemoteMsgHdr msg;
        uint32_t size = sizeof(msg);
        if (!reader.MoveNext(nullptr, keySize, &msg, size, 0))
        {
            OnError("msg with current id is absent");
            return;
        }

        Env::DocAddNum(AMOUNT, msg.m_Amount);
        Env::DocAddNum(RELAYER_FEE, msg.m_RelayerFee);
        Env::DocAddBlob_T(RECEIVER, msg.m_UserPK);
    }
} // namespace manager

BEAM_EXPORT void Method_0()
{
    // scheme
    Env::DocGroup root("");
    {
        Env::DocGroup grMethod(Actions::CREATE);
        Env::DocAddText(TOKEN_CID, "ContractID");
        Env::DocAddText(TOKEN_AID, "AssedID");
    }
    {
        Env::DocGroup grMethod(Actions::VIEW);
    }
    {
        Env::DocGroup grMethod(Actions::SET_RELAYER);
        Env::DocAddText(CONTRACT_ID, "ContractID");
        Env::DocAddText(RELAYER, "PubKey");
    }
    {
        Env::DocGroup grMethod(Actions::GET_PK);
        Env::DocAddText(CONTRACT_ID, "ContractID");
    }
    {
        Env::DocGroup grMethod(Actions::SEND);
        Env::DocAddText(CONTRACT_ID, "ContractID");
        Env::DocAddText(AMOUNT, "uint64");
        Env::DocAddText(RELAYER_FEE, "uint64");
        Env::DocAddText(RECEIVER, "Address");
    }
    {
        Env::DocGroup grMethod(Actions::RECEIVE);
        Env::DocAddText(CONTRACT_ID, "ContractID");
        Env::DocAddText(MSG_ID, "uint64");
    }
    {
        Env::DocGroup grMethod(Actions::PUSH_REMOTE);
        Env::DocAddText(CONTRACT_ID, "ContractID");
        Env::DocAddText(MSG_ID, "uint64");
        Env::DocAddText(AMOUNT, "uint64");
        Env::DocAddText(RELAYER_FEE, "uint64");
        Env::DocAddText(RECEIVER, "PubKey");
    }
    // local
    {
        Env::DocGroup grMethod(Actions::VIEW_INCOMING);
        Env::DocAddText(CONTRACT_ID, "ContractID");
    }
    {
        Env::DocGroup grMethod(Actions::LOCAL_MSG_COUNT);
        Env::DocAddText(CONTRACT_ID, "ContractID");
    }
    {
        Env::DocGroup grMethod(Actions::LOCAL_MSG);
        Env::DocAddText(CONTRACT_ID, "ContractID");
        Env::DocAddText(MSG_ID, "uint64");
    }
    {
        Env::DocGroup grMethod(Actions::REMOTE_MSG);
        Env::DocAddText(CONTRACT_ID, "ContractID");
        Env::DocAddText(MSG_ID, "uint64");
    }
}

BEAM_EXPORT void Method_1()
{
    Env::DocGroup root("");

    char szAction[20];

    if (!Env::DocGetText("action", szAction, sizeof(szAction)))
    {
        OnError("Action should be specified");
        return;
    }

    if (!Env::Strcmp(szAction, Actions::CREATE))
    {
        manager::Create();
    }
    else if (!Env::Strcmp(szAction, Actions::VIEW))
    {
        manager::View();
    }
    else if (!Env::Strcmp(szAction, Actions::SET_RELAYER))
    {
        manager::SetRelayer();
    }
    else if (!Env::Strcmp(szAction, Actions::GET_PK))
    {
        manager::GetPk();
    }
    else if (!Env::Strcmp(szAction, Actions::SEND))
    {
        manager::SendFunds();
    }
    else if (!Env::Strcmp(szAction, Actions::RECEIVE))
    {
        manager::ReceiveFunds();
    }
    else if (!Env::Strcmp(szAction, Actions::PUSH_REMOTE))
    {
        manager::PushRemote();
    }
    else if (!Env::Strcmp(szAction, Actions::VIEW_INCOMING))
    {
        manager::ViewIncomingMsg();
    }
    else if (!Env::Strcmp(szAction, Actions::LOCAL_MSG_COUNT))
    {
        manager::GetLocalMsgCount();
    }
    else if (!Env::Strcmp(szAction, Actions::LOCAL_MSG))
    {
        manager::GetLocalMsg();
    }
    else if (!Env::Strcmp(szAction, Actions::REMOTE_MSG))
    {
        manager::GetRemoteMsg();
    }
    else
    {
        OnError("invalid Action.");
    }
}