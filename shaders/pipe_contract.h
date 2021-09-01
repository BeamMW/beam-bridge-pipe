#pragma once
#include "Shaders/Eth.h"

namespace Pipe
{
#pragma pack (push, 1) // the following structures will be stored in the node in binary form
    struct Create
    {
        static const uint32_t s_iMethod = 0;
    };

    struct SetRemote
    {
        static const uint32_t s_iMethod = 2;
    };

    struct Send
    {
        static const uint32_t s_iMethod = 3;
    };

    struct Receive
    {
        static const uint32_t s_iMethod = 4;
    };

    struct PushRemote
    {
        static const uint32_t s_iMethod = 5;
    };

    struct PayFee
    {
        static const uint32_t s_iMethod = 6;
    };

    struct StartDispute
    {
        static const uint32_t s_iMethod = 7;
    };

    struct NextHeader
    {
        static const uint32_t s_iMethod = 8;
    };

    struct Finish
    {
        static const uint32_t s_iMethod = 9;
    };
#pragma pack (pop)
}