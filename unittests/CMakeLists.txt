cmake_minimum_required(VERSION 3.17)

add_test_snippet(shader_test bvm beam)
target_include_directories(shader_test PUBLIC ${PROJECT_SOURCE_DIR}/shaders)

add_shader(pipe_contract ${PROJECT_SOURCE_DIR}/shaders)
add_shader(pipe_app ${PROJECT_SOURCE_DIR}/shaders)
add_shader(mirrortoken_contract ${PROJECT_SOURCE_DIR}/shaders)
add_shader(mirrortoken_app ${PROJECT_SOURCE_DIR}/shaders)

add_custom_target(make_pipe_header
    ALL
    COMMAND generate-sid ${PROJECT_SOURCE_DIR}/shaders/pipe_contract.wasm > ${PROJECT_SOURCE_DIR}/shaders/pipe_contract_sid.i
    COMMENT "Generating SID ..."
    DEPENDS generate-sid pipe_contract_target
)

add_custom_target(make_mirrortoken_header
    ALL
    COMMAND generate-sid ${PROJECT_SOURCE_DIR}/shaders/mirrortoken_contract.wasm > ${PROJECT_SOURCE_DIR}/shaders/mirrortoken_contract_sid.i
    COMMENT "Generating SID ..."
    DEPENDS generate-sid mirrortoken_contract_target
)

add_dependencies(shader_test make_pipe_header make_mirrortoken_header)
add_dependencies(pipe_app_target make_pipe_header)
add_dependencies(mirrortoken_app_target make_mirrortoken_header)