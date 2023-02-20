To launch bridges, we need the following things on the Beam side:
New token(bUSDT as example) - wrapped ERC20 token or ETH. Use token_app.wasm to create a new token.
Bridge contact - contract that manages the minting(transactions to Beam) and burning(transactions from Beam) of tokens

Deployment process using Beam dappnet as an example:

1. deploy new Token
	beam-wallet-dappnet.exe -n 127.0.0.1:10004   --wallet_path="d:/beam_wallets/dappnet/Admin/7.2.13309.5570/wallet.db" shader --shader_app_file="d:/bridge/beam-bridge-pipe/shaders/token_app.wasm" --shader_args="action=create,metadata=STD:SCH_VER=1;N=bETH3;SN=bETH3;UN=bETH3;NTHUN=TGROTH" --shader_contract_file="d:/bridge/beam-bridge-pipe/shaders/token_contract.wasm"

2. get token cid:
	beam-wallet-dappnet.exe -n 127.0.0.1:10004   shader --wallet_path="d:/beam_wallets/dappnet/Admin/7.2.13309.5570/wallet.db" --shader_app_file="d:/bridge/beam-bridge-pipe/shaders/token_app.wasm" --shader_args="action=view"
	
-> tokenCID=d68cc6de5ac395ac3c5651375fce5df9201c2c4c84c211e2873c44bf428d3bdb

3. init owner publicKey for the token contract 
	beam-wallet-dappnet.exe -n 127.0.0.1:10004   shader --wallet_path="d:/beam_wallets/dappnet/Admin/7.2.13309.5570/wallet.db" --shader_app_file="d:/bridge/beam-bridge-pipe/shaders/token_app.wasm" --shader_args="action=init,cid=d68cc6de5ac395ac3c5651375fce5df9201c2c4c84c211e2873c44bf428d3bdb"

4. get asset id for the token contract
	beam-wallet-dappnet.exe -n 127.0.0.1:10004   shader --wallet_path="d:/beam_wallets/dappnet/Admin/7.2.13309.5570/wallet.db" --shader_app_file="d:/bridge/beam-bridge-pipe/shaders/token_app.wasm" --shader_args="action=get_aid,cid=d68cc6de5ac395ac3c5651375fce5df9201c2c4c84c211e2873c44bf428d3bdb"

5. deploy pipe shader
	beam-wallet-dappnet.exe -n 127.0.0.1:10004   shader --wallet_path="d:/beam_wallets/dappnet/Admin/7.2.13309.5570/wallet.db" --shader_app_file="d:/bridge/beam-bridge-pipe/shaders/pipe_app.wasm" --shader_args="action=create,tokenCID=d68cc6de5ac395ac3c5651375fce5df9201c2c4c84c211e2873c44bf428d3bdb,tokenAID=116" --shader_contract_file="d:/bridge/beam-bridge-pipe/shaders/pipe_contract.wasm"

6. get CID of the pipe
	beam-wallet-dappnet.exe -n 127.0.0.1:10004   shader --wallet_path="d:/beam_wallets/dappnet/Admin/7.2.13309.5570/wallet.db" --shader_app_file="d:/bridge/beam-bridge-pipe/shaders/pipe_app.wasm" --shader_args="action=view"

-> pipeCid=2ce5d66babf25f1a1908df54b8d7ca6a14f7f9432e78ef94ac97293170924dec

7. setup pipe as manager of the Token
	beam-wallet-dappnet.exe -n 127.0.0.1:10004   shader --wallet_path="d:/beam_wallets/dappnet/Admin/7.2.13309.5570/wallet.db" --shader_app_file="d:/bridge/beam-bridge-pipe/shaders/token_app.wasm" --shader_args="action=change_manager,cid=d68cc6de5ac395ac3c5651375fce5df9201c2c4c84c211e2873c44bf428d3bdb,manager=2ce5d66babf25f1a1908df54b8d7ca6a14f7f9432e78ef94ac97293170924dec"

8. generate relayer's publicKey
	beam-wallet-dappnet.exe -n 127.0.0.1:10004   shader --wallet_path="d:/beam_wallets/dappnet/relayer_bETH3/7.2.13309.5570/wallet.db" --shader_app_file="d:/bridge/beam-bridge-pipe/shaders/pipe_app.wasm" --shader_args="action=get_pk,cid=2ce5d66babf25f1a1908df54b8d7ca6a14f7f9432e78ef94ac97293170924dec"
	
-> relayerPubkey=3e8ec1895fa1155a574963f4892ad98eaecc35b1b52a36f4251bb67a3f7583d401

9. setup new relayer for the Pipe
	beam-wallet-dappnet.exe -n 127.0.0.1:10004   shader --wallet_path="d:/beam_wallets/dappnet/Admin/7.2.13309.5570/wallet.db" --shader_app_file="d:/bridge/beam-bridge-pipe/shaders/pipe_app.wasm" --shader_args="action=set_relayer,cid=2ce5d66babf25f1a1908df54b8d7ca6a14f7f9432e78ef94ac97293170924dec,relayer=3e8ec1895fa1155a574963f4892ad98eaecc35b1b52a36f4251bb67a3f7583d401"
