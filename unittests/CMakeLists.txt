cmake_minimum_required(VERSION 3.17)

add_test_snippet(shader_test bvm beam)
target_include_directories(shader_test PUBLIC ${PROJECT_SOURCE_DIR}/shaders)

add_shader(pipe_contract ${PROJECT_SOURCE_DIR}/shaders)
add_shader(pipe_app ${PROJECT_SOURCE_DIR}/shaders)
add_shader(token_contract ${PROJECT_SOURCE_DIR}/shaders)
add_shader(token_app ${PROJECT_SOURCE_DIR}/shaders)

add_custom_target(make_pipe_header
    ALL
    COMMAND generate-sid ${PROJECT_SOURCE_DIR}/shaders/pipe_contract.wasm > ${PROJECT_SOURCE_DIR}/shaders/pipe_contract_sid.i
    COMMENT "Generating SID ..."
    DEPENDS generate-sid pipe_contract_target
)

add_custom_target(make_token_header
    ALL
    COMMAND generate-sid ${PROJECT_SOURCE_DIR}/shaders/token_contract.wasm > ${PROJECT_SOURCE_DIR}/shaders/token_contract_sid.i
    COMMENT "Generating SID ..."
    DEPENDS generate-sid token_contract_target
)

add_dependencies(shader_test make_pipe_header make_token_header)
add_dependencies(pipe_app_target make_pipe_header)
add_dependencies(token_app_target make_token_header)