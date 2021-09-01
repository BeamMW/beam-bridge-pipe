#pragma once
#include "Shaders/Eth.h"

namespace Pipe
{
#pragma pack (push, 1) // the following structures will be stored in the node in binary form
    using RemoteID = Eth::Address;

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

    struct Send
    {
        static const uint32_t s_iMethod = 3;

        Eth::Address m_User;
        Amount m_Amount;
    };

    struct Receive
    {
        static const uint32_t s_iMethod = 4;

        uint32_t m_MsgId;
    };

    struct PushRemote
    {
        static const uint32_t s_iMethod = 5;

        uint32_t m_MsgId;
        uint32_t m_MsgSize;
        // followed by message variable data
    };

    struct PayFee
    {
        static const uint32_t s_iMethod = 6;

        uint32_t m_MsgId;
        Amount m_Amount;
    };

    struct StartDispute
    {
        static const uint32_t s_iMethod = 7;

        uint32_t m_MsgId;
        //RemoteMsgHdr m_MsgHdr;
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
        static const uint32_t s_iMethod = 8;

        uint32_t m_MsgId;
        Eth::Header m_Header;
        uint32_t m_DatasetCount;
        uint32_t m_ProofSize;
    };

    struct Finish
    {
        static const uint32_t s_iMethod = 9;
    };

    struct Params
    {
        RemoteID m_Remote;
        AssetID m_Aid;
    };
#pragma pack (pop)
}