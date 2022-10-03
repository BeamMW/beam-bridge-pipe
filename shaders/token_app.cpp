#include "Shaders/common.h"
#include "Shaders/app_common_impl.h"
#include "token_contract.h"

namespace Token
{
#include "token_contract_sid.i"
}

namespace
{
    const char* METADATA = "metadata";
    const char* CONTRACT_ID = "cid";
    const char* AMOUNT = "amount";
    const char* OWNER = "owner";
    const char* MANAGER = "manager";
    const char* AID = "aid";

    namespace Actions
    {
        const char* CREATE = "create";
        const char* VIEW = "view";
        const char* INIT = "init";
        const char* GET_AID = "get_aid";
        const char* CHANGE_OWNER = "change_owner";
        const char* CHANGE_MANAGER = "change_manager";
    } // namespace Actions

    const Amount SHADER_PRICE = 1000000000ULL;

    void OnError(const char* sz)
    {
        Env::DocAddText("error", sz);
    }

    struct ParamsPlus : public Token::Params
    {
        bool get(const ContractID& cid)
        {
            Env::Key_T<uint8_t> gk;
            _POD_(gk.m_Prefix.m_Cid) = cid;
            gk.m_KeyInContract = Token::PARAMS_KEY;

            if (Env::VarReader::Read_T(gk, *this))
                return true;

            OnError("no params");
            return false;
        }
    };
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

        auto* args = (Token::Create*)Env::StackAlloc(sizeof(Token::Create) + metaSize);

        Env::DocGetText(METADATA, (char*)(args + 1), metaSize);
        metaSize--; // ignore last symbol '\0'

        args->m_MetadataSize = metaSize;

        FundsChange fc;
        fc.m_Aid = 0; // beam id
        fc.m_Amount = SHADER_PRICE; // amount of the input or output
        fc.m_Consume = 1; // contract consumes funds (i.e input, in this case)

        Env::GenerateKernel(nullptr, args->s_iMethod, args, sizeof(*args) + metaSize, &fc, 1, nullptr, 0, "create Token contract", 140000);
    }

    void View()
    {
        EnumAndDumpContracts(Token::s_SID);
    }

    void Init()
    {
        ContractID cid;
        Env::DocGet(CONTRACT_ID, cid);

        Token::Init args;

        Env::DerivePk(args.m_Owner, &cid, sizeof(cid));
        Env::GenerateKernel(&cid, args.s_iMethod, &args, sizeof(args), nullptr, 0, nullptr, 0, "Init token", 0);
    }

    void GetAid()
    {
        ContractID cid;
        Env::DocGet(CONTRACT_ID, cid);

        ParamsPlus params;
        if (!params.get(cid))
        {
            return;
        }

        Env::DocAddNum(AID, params.m_AssetID);
    }

    void ChangeOwner()
    {
        ContractID cid;
        Env::DocGet(CONTRACT_ID, cid);

        Token::ChangeOwner args;
        Env::DocGet(OWNER, args.m_NewOwner);
                
        SigRequest sig;
        sig.m_pID = &cid;
        sig.m_nID = sizeof(cid);

        Env::GenerateKernel(&cid, args.s_iMethod, &args, sizeof(args), nullptr, 0, &sig, 1, "Change owner", 0);
    }

    void ChangeManager()
    {
        ContractID cid;
        Env::DocGet(CONTRACT_ID, cid);

        Token::ChangeManager args;
        Env::DocGet(MANAGER, args.m_NewManager);

        SigRequest sig;
        sig.m_pID = &cid;
        sig.m_nID = sizeof(cid);

        Env::GenerateKernel(&cid, args.s_iMethod, &args, sizeof(args), nullptr, 0, &sig, 1, "Change contract id of manager", 0);
    }
} // namespace manager

BEAM_EXPORT void Method_0()
{
    // scheme
    Env::DocGroup root("");
    {
        Env::DocGroup grMethod(Actions::CREATE);
        Env::DocAddText(METADATA, "string");
    }
    {
        Env::DocGroup grMethod(Actions::VIEW);
    }
    {
        Env::DocGroup grMethod(Actions::INIT);
        Env::DocAddText(CONTRACT_ID, "ContractID");
    }
    {
        Env::DocGroup grMethod(Actions::GET_AID);
        Env::DocAddText(CONTRACT_ID, "ContractID");
    }
    {
        Env::DocGroup grMethod(Actions::CHANGE_OWNER);
        Env::DocAddText(CONTRACT_ID, "ContractID");
        Env::DocAddText(OWNER, "PubKey");
    }
    {
        Env::DocGroup grMethod(Actions::CHANGE_MANAGER);
        Env::DocAddText(CONTRACT_ID, "ContractID");
        Env::DocAddText(MANAGER, "ContractID");
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
    else if (!Env::Strcmp(szAction, Actions::INIT))
    {
        manager::Init();
    }
    else if (!Env::Strcmp(szAction, Actions::GET_AID))
    {
        manager::GetAid();
    }
    else if (!Env::Strcmp(szAction, Actions::CHANGE_OWNER))
    {
        manager::ChangeOwner();
    }
    else if (!Env::Strcmp(szAction, Actions::CHANGE_MANAGER))
    {
        manager::ChangeManager();
    }
    else
    {
        OnError("invalid Action.");
    }
}