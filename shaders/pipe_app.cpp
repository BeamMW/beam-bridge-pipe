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
        Env::DocGroup gr("roles");
        {
            Env::DocGroup grRole("manager");
            {
                Env::DocGroup grMethod("create");
            }
            {
                Env::DocGroup grMethod("view");
            }
            {
                Env::DocGroup grMethod("pushRemote");
                Env::DocAddText("cid", "ContractID");
                Env::DocAddText("msgId", "uint32");
            }
            {
                Env::DocGroup grMethod("getLocalMsgCount");
                Env::DocAddText("cid", "ContractID");
            }
            {
                Env::DocGroup grMethod("getLocalMsg");
                Env::DocAddText("cid", "ContractID");
                Env::DocAddText("msgId", "uint32");
            }
            {
                Env::DocGroup grMethod("getLocalMsgProof");
                Env::DocAddText("cid", "ContractID");
                Env::DocAddText("msgId", "uint32");
            }
            {
                Env::DocGroup grMethod("getRemoteMsg");
                Env::DocAddText("cid", "ContractID");
                Env::DocAddText("msgId", "uint32");
            }
        }
    }
}

BEAM_EXPORT void Method_1()
{
    Env::DocGroup root("");

    char szRole[0x10], szAction[0x12];

    if (!Env::DocGetText("role", szRole, sizeof(szRole)))
        return OnError("Role not specified");

    if (!Env::DocGetText("action", szAction, sizeof(szAction)))
        return OnError("Action not specified");

    if (!Env::Strcmp(szRole, "manager"))
    {
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
}