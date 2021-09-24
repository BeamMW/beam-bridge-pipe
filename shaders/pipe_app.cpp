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
    const char* METADATA = "metadata";
    const char* CONTRACT_ID = "cid";
    const char* ADDRESS_REMOTE = "addressRemote";
    const char* AMOUNT = "amount";
    const char* RECEIVER = "receiver";
    const char* MSG_ID = "msgId";
    const char* HEIGHT = "height";
    const char* TIMESTAMP = "timestamp";
    const char* REMOTE_MSG = "remoteMsg"; // ????
    const char* CONTRACT_RECEIVER = "contractReceiver";
    const char* CONTRACT_SENDER = "contractSender";
    const char* START_FROM = "startFrom";
    const char* USER_PK = "userPK";

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

        Pipe::RemoteID m_Remote;
        Env::VarReaderEx<true> m_Reader;
        Env::Key_T<Pipe::RemoteMsgHdr::Key> m_Key;
        Pipe::RemoteMsgHdr m_Msg;


        bool Restart(uint32_t iStartFrom)
        {
            ParamsPlus params;
            if (!params.get(m_Cid))
                return false;

            m_Remote = params.m_Remote;

            Env::Key_T<Pipe::RemoteMsgHdr::Key> k1;
            //k1.m_Prefix.m_Cid = params.m_PipeID;
            k1.m_Prefix.m_Cid = m_Cid; // TODO check this ????
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

                if ((_POD_(m_Msg.m_ContractSender) != m_Remote) || (_POD_(m_Msg.m_ContractReceiver) != m_Cid))
                    continue;

                if (pPk && (_POD_(*pPk) != m_Msg.m_UserPK))
                    continue;

                return true;
            }
        }
    };

    void ViewIncoming(const ContractID& cid, const PubKey* pPk, uint32_t iStartFrom)
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
        int32_t metaSize = Env::DocGetText(METADATA, nullptr, 0);
        if (metaSize < 2)
        {
            OnError("metadata should be non-empty");
            return;
        }

        auto* args = (Pipe::Create*)Env::StackAlloc(sizeof(Pipe::Create) + metaSize);

        Env::DocGetText(METADATA, (char*)(args + 1), metaSize);
        metaSize--; // ??????

        args->m_MetadataSize = metaSize;

        FundsChange fc;
        fc.m_Aid = 0; // asset id
        fc.m_Amount = 300000000000ULL; // amount of the input or output
        fc.m_Consume = 1; // contract consumes funds (i.e input, in this case)

        Env::GenerateKernel(nullptr, args->s_iMethod, args, sizeof(*args) + metaSize, &fc, 1, nullptr, 0, "create Pipe contract", 0);
    }

    void View()
    {
        EnumAndDumpContracts(Pipe::s_SID);
    }

    void SetRemote()
    {
        ContractID cid;
        Env::DocGet(CONTRACT_ID, cid);

        Pipe::RemoteID addressRemote;
        Env::DocGetBlobEx(ADDRESS_REMOTE, &addressRemote, sizeof(addressRemote));

        Pipe::SetRemote args;
        args.m_Remote = addressRemote;

        Env::GenerateKernel(&cid, args.s_iMethod, &args, sizeof(args), nullptr, 0, nullptr, 0, "Set remote ID counter-part", 0);
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
        Env::DocGetBlobEx(RECEIVER, &args.m_Receiver, sizeof(args.m_Receiver));

        FundsChange fc;
        fc.m_Aid = params.m_Aid;
        fc.m_Amount = args.m_Amount;
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
        Env::DocGetNum32(MSG_ID, &args.m_MsgId);

        FundsChange fc;
        fc.m_Aid = params.m_Aid;
        //fc.m_Amount = args.m_Amount;
        fc.m_Consume = 0;

        Env::GenerateKernel(&cid, args.s_iMethod, &args, sizeof(args), &fc, 1, nullptr, 0, "Receive funds", 0);
    }

    void PushRemote()
    {
        ContractID cid;
        Env::DocGet(CONTRACT_ID, cid);

        Pipe::PushRemote args;        
        Env::DocGetNum32(MSG_ID, &args.m_MsgId);
        Env::DocGetNum64(HEIGHT, &args.m_RemoteMsg.m_Height);
        Env::DocGetNum64(TIMESTAMP, &args.m_RemoteMsg.m_Timestamp);
        Env::DocGet(CONTRACT_RECEIVER, args.m_RemoteMsg.m_ContractReceiver);
        Env::DocGetBlobEx(CONTRACT_SENDER, &args.m_RemoteMsg.m_ContractSender, sizeof(args.m_RemoteMsg.m_ContractSender));
        Env::DocGetNum64(AMOUNT, &args.m_RemoteMsg.m_Amount);
        Env::DocGet(USER_PK, args.m_RemoteMsg.m_UserPK);

        FundsChange fc;
        fc.m_Aid = 0;
        fc.m_Amount = 1000000000000ULL; // lock 10 beam of relayer
        fc.m_Consume = 1;

        Env::GenerateKernel(&cid, args.s_iMethod, &args, sizeof(args), &fc, 1, nullptr, 0, "Push remote message", 0);
    }

    void PayFee()
    {
        ContractID cid;
        Env::DocGet(CONTRACT_ID, cid);

        Pipe::PayFee args;
        Env::DocGetNum64(AMOUNT, &args.m_Amount);
        Env::DocGetNum32(MSG_ID, &args.m_MsgId);
        Env::DocGetNum64(HEIGHT, &args.m_Height);
        Env::DocGetNum64(TIMESTAMP, &args.m_Timestamp);

        FundsChange fc;
        fc.m_Aid = 0;
        fc.m_Amount = args.m_Amount;
        fc.m_Consume = 1;

        Env::GenerateKernel(&cid, args.s_iMethod, &args, sizeof(args), &fc, 1, nullptr, 0, "Pay fee to relayer", 0);
    }

    void StartDispute()
    {
        ContractID cid;
        Env::DocGet(CONTRACT_ID, cid);

        Pipe::StartDispute args;

        Env::GenerateKernel(&cid, args.s_iMethod, &args, sizeof(args), nullptr, 0, nullptr, 0, "Start dispute", 0);
    }

    void ContinueDispute()
    {
        ContractID cid;
        Env::DocGet(CONTRACT_ID, cid);

        Pipe::ContinueDispute args;

        Env::GenerateKernel(&cid, args.s_iMethod, &args, sizeof(args), nullptr, 0, nullptr, 0, "Continue dispute", 0);
    }

    void FinalizeRemoteMsg()
    {
        ContractID cid;
        Env::DocGet(CONTRACT_ID, cid);

        Pipe::FinilizeRemoteMsg args;
        Env::DocGetNum32(MSG_ID, &args.m_MsgId);

        Env::GenerateKernel(&cid, args.s_iMethod, &args, sizeof(args), nullptr, 0, nullptr, 0, "Finalize remote message", 0);
    }

    void ViewIncomingMsg()
    {
        ContractID cid;
        uint32_t startFrom = 0;
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

        uint32_t localMsgCounter = 0;
        Env::VarReader::Read_T(key, localMsgCounter);

        Env::DocAddNum32("count", localMsgCounter);
    }

    void GetLocalMsg()
    {
        ContractID cid;
        uint32_t msgId;
        Env::DocGet(CONTRACT_ID, cid);
        Env::DocGetNum32(MSG_ID, &msgId);
    }

    void GetLocalMsgProof()
    {
        ContractID cid;
        uint32_t msgId;
        Env::DocGet(CONTRACT_ID, cid);
        Env::DocGetNum32(MSG_ID, &msgId);
    }
} // namespace manager

BEAM_EXPORT void Method_0()
{
    // scheme
    Env::DocGroup root("");
    {
        Env::DocGroup grMethod("create");
        Env::DocAddText("metadata", "string");
    }
    {
        Env::DocGroup grMethod("view");
    }
    {
        Env::DocGroup grMethod("set_remote");
        Env::DocAddText(CONTRACT_ID, "ContractID");
        Env::DocAddText("remote_addr", "Address");
    }
    {
        Env::DocGroup grMethod("send");
        Env::DocAddText(CONTRACT_ID, "ContractID");
        Env::DocAddText(AMOUNT, "uint64");
        Env::DocAddText(RECEIVER, "Address");
    }
    {
        Env::DocGroup grMethod("receive");
        Env::DocAddText(CONTRACT_ID, "ContractID");
        Env::DocAddText(MSG_ID, "uint32");
    }
    {
        Env::DocGroup grMethod("push_remote");
        Env::DocAddText(CONTRACT_ID, "ContractID");
        Env::DocAddText(MSG_ID, "uint32");
        Env::DocAddText(HEIGHT, "Height");
        Env::DocAddText(TIMESTAMP, "uint64");
        Env::DocAddText(CONTRACT_RECEIVER, "ContractID");
        Env::DocAddText(CONTRACT_SENDER, "Address");
        Env::DocAddText(AMOUNT, "uint64");
        Env::DocAddText(USER_PK, "PubKey");
    }
    {
        Env::DocGroup grMethod("pay_fee");
        Env::DocAddText(CONTRACT_ID, "ContractID");
        Env::DocAddText(MSG_ID, "uint32");
        Env::DocAddText(HEIGHT, "Height");
        Env::DocAddText(TIMESTAMP, "uint64");
        Env::DocAddText(AMOUNT, "uint64");
    }
    {
        Env::DocGroup grMethod("start_dispute");
        Env::DocAddText(CONTRACT_ID, "ContractID");
    }
    {
        Env::DocGroup grMethod("continue_dispute");
        Env::DocAddText(CONTRACT_ID, "ContractID");
    }
    {
        Env::DocGroup grMethod("finalize_remote_msg");
        Env::DocAddText(CONTRACT_ID, "ContractID");
        Env::DocAddText(MSG_ID, "uint32");
    }
    // local
    {
        Env::DocGroup grMethod("view_incoming");
        Env::DocAddText(CONTRACT_ID, "ContractID");
    }
    {
        Env::DocGroup grMethod("local_msg_count");
        Env::DocAddText(CONTRACT_ID, "ContractID");
    }
    {
        Env::DocGroup grMethod("local_msg");
        Env::DocAddText(CONTRACT_ID, "ContractID");
        Env::DocAddText(MSG_ID, "uint32");
    }

    {
        Env::DocGroup grMethod("local_msg_proof");
        Env::DocAddText(CONTRACT_ID, "ContractID");
        Env::DocAddText(MSG_ID, "uint32");
    }
}

BEAM_EXPORT void Method_1()
{
    Env::DocGroup root("");

    char szAction[20];

    if (!Env::DocGetText("action", szAction, sizeof(szAction)))
    {
        OnError("Action not specified");
        return;
    }

    if (!Env::Strcmp(szAction, "create"))
    {
        manager::Create();
    }
    else if (!Env::Strcmp(szAction, "view"))
    {
        manager::View();
    }
    else if (!Env::Strcmp(szAction, "set_remote"))
    {
        manager::SetRemote();
    }
    else if (!Env::Strcmp(szAction, "send"))
    {
        manager::SendFunds();
    }
    else if (!Env::Strcmp(szAction, "receive"))
    {
        manager::ReceiveFunds();
    }
    else if (!Env::Strcmp(szAction, "push_remote"))
    {
        manager::PushRemote();
    }
    else if (!Env::Strcmp(szAction, "pay_fee"))
    {
        manager::PayFee();
    }
    else if (!Env::Strcmp(szAction, "start_dispute"))
    {
        manager::StartDispute();
    }
    else if (!Env::Strcmp(szAction, "continue_dispute"))
    {
        manager::ContinueDispute();
    }
    else if (!Env::Strcmp(szAction, "finalize_remote_msg"))
    {
        manager::FinalizeRemoteMsg();
    }
    else if (!Env::Strcmp(szAction, "view_incoming"))
    {
        manager::ViewIncomingMsg();
    }
    else if (!Env::Strcmp(szAction, "local_msg_count"))
    {
        manager::GetLocalMsgCount();
    }
    else if (!Env::Strcmp(szAction, "local_msg"))
    {
        manager::GetLocalMsg();
    }
    else if (!Env::Strcmp(szAction, "local_msg_proof"))
    {
        manager::GetLocalMsgProof();
    }
    else
    {
        OnError("invalid Action.");
    }
}