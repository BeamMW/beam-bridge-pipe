#pragma once
#include "Shaders/Eth.h"

namespace Token
{
#pragma pack (push, 1) // the following structures will be stored in the node in binary form
    static const uint8_t PARAMS_KEY = 0;

    struct Create
    {
        static const uint32_t s_iMethod = 0;

        uint32_t m_MetadataSize;
    };

    struct ChangeOwner
    {
        static const uint32_t s_iMethod = 2;

        PubKey m_NewOwner;
    };

    struct ChangeManager
    {
        static const uint32_t s_iMethod = 3;

        ContractID m_NewContractId;
    };

    struct Mint
    {
        static const uint32_t s_iMethod = 4;

        Amount m_Amount;
    };

    struct Burn
    {
        static const uint32_t s_iMethod = 5;

        Amount m_Amount;
    };

    struct Init
    {
        static const uint32_t s_iMethod = 6;

        PubKey m_Owner;
    };

    struct Params
    {
        PubKey m_Owner;
        ContractID m_ContractId;
        AssetID m_Aid;
        bool m_IsInit;
    };
#pragma pack (pop)
} // namespace Token