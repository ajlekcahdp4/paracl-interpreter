add_test(NAME test.paracl.external
         COMMAND ${BASH_PROGRAM} ${SCRIPTS_DIR}/test_compare.sh
                 "$<TARGET_FILE:pclc>" ${CMAKE_CURRENT_SOURCE_DIR}/external)

add_test(NAME test.paracl.basic
         COMMAND ${BASH_PROGRAM} ${SCRIPTS_DIR}/test_compare.sh
                 "$<TARGET_FILE:pclc>" ${CMAKE_CURRENT_SOURCE_DIR}/basic)

add_test(NAME test.paracl.blocks
         COMMAND ${BASH_PROGRAM} ${SCRIPTS_DIR}/test_compare.sh
                 "$<TARGET_FILE:pclc>" ${CMAKE_CURRENT_SOURCE_DIR}/blocks)

add_test(NAME test.paracl.functions COMMAND ${BASH_PROGRAM}
${SCRIPTS_DIR}/test_compare.sh "$<TARGET_FILE:pclc>"
${CMAKE_CURRENT_SOURCE_DIR}/functions)

add_test(NAME test.paracl.fail
         COMMAND ${BASH_PROGRAM} ${SCRIPTS_DIR}/test_fail.sh
                 "$<TARGET_FILE:pclc>" ${CMAKE_CURRENT_SOURCE_DIR}/errors)
