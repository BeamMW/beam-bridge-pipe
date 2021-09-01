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
    void OnError(const char* sz)
    {
        Env::DocAddText("error", sz);
    }
} // namespace

namespace manager
{
    void Create()
    {
        //Env::GenerateKernel(nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 0, "create Pipe contract", 0);
    }

    void View()
    {
        EnumAndDumpContracts(Pipe::s_SID);
    }    
} // namespace manager

BEAM_EXPORT void Method_0()
{
    // scheme
    Env::DocGroup root("");
    {
        Env::DocGroup grMethod("create");
    }
    {
        Env::DocGroup grMethod("view");
    }
}

BEAM_EXPORT void Method_1()
{
    Env::DocGroup root("");

    char szAction[0x12];

    if (!Env::DocGetText("action", szAction, sizeof(szAction)))
        return OnError("Action not specified");

    if (!Env::Strcmp(szAction, "create"))
    {
        manager::Create();
    }
    else if (!Env::Strcmp(szAction, "view"))
    {
        manager::View();
    }        
    else
    {
        return OnError("invalid Action.");
    }
}