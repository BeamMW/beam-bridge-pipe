cmake_minimum_required(VERSION 3.17)

include(GenerateSID)

# Pipe shader
add_executable(pipe_contract pipe_contract.cpp)
copy_shader(pipe_contract)
generate_sid_header(pipe_contract)

add_executable(pipe_app pipe_app.cpp)

add_dependencies(pipe_app pipe_contract_header)
copy_shader(pipe_app)
