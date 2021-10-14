cmake_minimum_required(VERSION 3.17)

include(GenerateSID)

# Token shader
add_executable(token_contract token_contract.cpp)
copy_shader(token_contract)
generate_sid_header(token_contract)

add_executable(token_app token_app.cpp)

add_dependencies(token_app token_contract_header)
copy_shader(token_app)

# Pipe shader
add_executable(pipe_contract pipe_contract.cpp)
copy_shader(pipe_contract)
generate_sid_header(pipe_contract)

add_executable(pipe_app pipe_app.cpp)

add_dependencies(pipe_app pipe_contract_header)
copy_shader(pipe_app)
