#pragma once

namespace MirrorToken
{
#pragma pack (push, 1)

    static const uint8_t kParamsKey = 0;

    using RemoteID = Eth::Address;

    struct Create
    {
        static const uint32_t s_iMethod = 0;

        ContractID m_BridgeID;
        uint32_t m_MetadataSize;
        // followed by metadata
    };

    struct SetRemote {
        static const uint32_t s_iMethod = 2;
        RemoteID m_Remote;
    };

    struct OutMessage {
        Eth::Address m_User;
        Amount m_Amount;
    };

    struct Send :public OutMessage {
        static const uint32_t s_iMethod = 3;
    };

    struct InMessage {
        PubKey m_User;
        Amount m_Amount;
    };

    struct Receive
    {
        static const uint32_t s_iMethod = 4;

        uint32_t m_MsgId;
    };

    struct Mint
    {
        static const uint32_t s_iMethod = 5;

        Amount m_Amount;
    };

    struct Params
    {
        ContractID m_BridgeID;
        RemoteID m_Remote;
        AssetID m_Aid;
    };

#pragma pack (pop)

}
