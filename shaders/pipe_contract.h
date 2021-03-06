#pragma once
#include "Shaders/Eth.h"

namespace Pipe
{
    static const uint8_t PARAMS_KEY = 0;
    static const uint8_t LOCAL_MSG_COUNTER_KEY = 5;

    using RemoteID = Eth::Address;

    struct KeyType
    {
        static const uint8_t LocalMsg = 2;
        static const uint8_t RemoteMsg = 3;
    };

#pragma pack (push, 1) // the following structures will be stored in the node in binary form
    struct MsgKeyBase
    {
        uint8_t m_Type;
        // big-endian, for simpler enumeration by app shader
        uint64_t m_MsgId_BE;
    };

    struct RemoteMsgHdr
    {
        struct Key : public MsgKeyBase
        {
            Key() { m_Type = KeyType::RemoteMsg; }
        };

        PubKey m_UserPK;
        Amount m_Amount;
        Amount m_RelayerFee;
    };

    struct LocalMsgHdr
    {
        struct Key : public MsgKeyBase
        {
            Key() { m_Type = KeyType::LocalMsg; }
        };

        RemoteID m_Receiver;
        Amount m_Amount;
        Amount m_RelayerFee;
        Height m_Height;
    };

    struct Create
    {
        static const uint32_t s_iMethod = 0;

        ContractID m_TokenCID;
        AssetID m_AssetID;
    };

    struct SetRelayer
    {
        static const uint32_t s_iMethod = 2;

        PubKey m_Relayer;
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

        uint64_t m_MsgId;
    };

    struct PushRemote
    {
        static const uint32_t s_iMethod = 5;

        uint64_t m_MsgId;
        RemoteMsgHdr m_RemoteMsg;
    };

    struct Params
    {
        PubKey m_Relayer;
        ContractID m_TokenCID;
        AssetID m_AssetID;
    };
#pragma pack (pop)
}