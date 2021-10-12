#pragma once
#include "Shaders/Eth.h"

namespace Pipe
{
#pragma pack (push, 1) // the following structures will be stored in the node in binary form
    static const uint8_t PARAMS_KEY = 0;
    static const uint8_t LOCAL_MSG_COUNTER_KEY = 5;

    static const Amount RELAYER_DEPOSIT = 1000000000ULL; // 10 beam

    using RemoteID = Eth::Address;

    struct KeyType
    {
        static const uint8_t LocalMsg = 2;
        static const uint8_t RemoteMsg = 3;
    };

    struct MsgKeyBase
    {
        uint8_t m_Type;
        // big-endian, for simpler enumeration by app shader
        uint32_t m_MsgId_BE;
    };

    struct RemoteMsgHdr
    {
        struct Key : public MsgKeyBase
        {
            Key() { m_Type = KeyType::RemoteMsg; }
        };

        ContractID m_ContractReceiver;
        RemoteID m_ContractSender;
        PubKey m_UserPK;
        Amount m_Amount;
        Amount m_RelayerFee;
        uint64_t m_Height; // ???
        uint64_t m_Timestamp; // ???
        PubKey m_Relayer;
        // height of block in beam chain
        uint64_t m_Height2; // TODO roman.strilets need best name
        bool m_Finalized;
        bool m_DisputeInProgress; // TODO roman.strilets need best name
    };

    struct LocalMsgHdr
    {
        struct Key : public MsgKeyBase
        {
            Key() { m_Type = KeyType::LocalMsg; }
        };

        ContractID m_ContractSender;
        RemoteID m_ContractReceiver;
        RemoteID m_Receiver;
        Amount m_Amount;
        uint64_t m_Height;
        Amount m_RelayerFee;
    };

    struct Create
    {
        static const uint32_t s_iMethod = 0;

        uint32_t m_MetadataSize;
    };

    struct SetRemote
    {
        static const uint32_t s_iMethod = 2;

        RemoteID m_Remote;
    };

    struct SendFunds
    {
        static const uint32_t s_iMethod = 3;

        RemoteID m_Receiver;
        Amount m_Amount;
        Amount m_RelayerFee;
    };

    struct ReceiveFunds
    {
        static const uint32_t s_iMethod = 4;

        uint32_t m_MsgId;
    };

    struct PushRemote
    {
        static const uint32_t s_iMethod = 5;

        uint32_t m_MsgId;
        RemoteMsgHdr m_RemoteMsg;
    };

    struct StartDispute
    {
        static const uint32_t s_iMethod = 6;

        uint32_t m_MsgId;
        Eth::Header m_Header;
        uint32_t m_DatasetCount;
        uint32_t m_ProofSize;
        uint32_t m_ReceiptProofSize;
        uint32_t m_TrieKeySize;
        uint32_t m_MsgSize;
        // followed by message variable data
    };

    struct ContinueDispute
    {
        static const uint32_t s_iMethod = 7;

        uint32_t m_MsgId;
        Eth::Header m_Header;
        uint32_t m_DatasetCount;
        uint32_t m_ProofSize;
        // followed by message variable data
    };

    struct FinalizeDispute
    {
        static const uint32_t s_iMethod = 8;

        uint32_t m_MsgId;
    };

    struct FinilizeRemoteMsg
    {
        static const uint32_t s_iMethod = 9;

        uint32_t m_MsgId;
    };

    struct Params
    {
        RemoteID m_Remote;
        AssetID m_Aid;
    };
#pragma pack (pop)
}