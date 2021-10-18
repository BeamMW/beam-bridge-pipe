#include "token_contract.h"

// Method_0 - constructor, called once when the contract is deployed
BEAM_EXPORT void Ctor(const Token::Create& args)
{
    Token::Params params;

    params.m_Aid = Env::AssetCreate(&args + 1, args.m_MetadataSize);
    Env::Halt_if(!params.m_Aid);
    params.m_IsInit = false;

    Env::SaveVar_T(Token::PARAMS_KEY, params);
}

// Method_1 - destructor, called once when the contract is destroyed
BEAM_EXPORT void Dtor(void*)
{
}

BEAM_EXPORT void Method_2(const Token::ChangeOwner& args)
{
    Token::Params params;
    Env::LoadVar_T(Token::PARAMS_KEY, params);
    Env::AddSig(params.m_Owner);

    _POD_(params.m_Owner) = args.m_NewOwner;

    Env::SaveVar_T(Token::PARAMS_KEY, params);
}

BEAM_EXPORT void Method_3(const Token::ChangeManager& args)
{
    Token::Params params;
    Env::LoadVar_T(Token::PARAMS_KEY, params);
    Env::AddSig(params.m_Owner);

    _POD_(params.m_ContractId) = args.m_NewContractId;

    Env::SaveVar_T(Token::PARAMS_KEY, params);
}

BEAM_EXPORT void Method_4(const Token::Mint& args)
{
    Token::Params params;
    Env::LoadVar_T(Token::PARAMS_KEY, params);

    // check caller
    ContractID cid;
    Env::get_CallerCid(1, cid);
    Env::Halt_if(_POD_(cid) != params.m_ContractId);

    Env::AssetEmit(params.m_Aid, args.m_Amount, 1);
    Env::FundsUnlock(params.m_Aid, args.m_Amount);
}

BEAM_EXPORT void Method_5(const Token::Burn& args)
{
    Token::Params params;
    Env::LoadVar_T(Token::PARAMS_KEY, params);

    // check caller
    ContractID cid;
    Env::get_CallerCid(1, cid);
    Env::Halt_if(_POD_(cid) != params.m_ContractId);

    Env::FundsLock(params.m_Aid, args.m_Amount);
    Env::AssetEmit(params.m_Aid, args.m_Amount, 0);
}

BEAM_EXPORT void Method_6(const Token::Init& args)
{
    Token::Params params;
    Env::LoadVar_T(Token::PARAMS_KEY, params);

    Env::Halt_if(params.m_IsInit);

    _POD_(params.m_Owner) = args.m_Owner;
    params.m_IsInit = true;

    Env::SaveVar_T(Token::PARAMS_KEY, params);
}