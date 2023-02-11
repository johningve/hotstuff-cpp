add_library(hotstuff STATIC
	blockchain.cpp
	consensus.cpp
	crypto.cpp
	peers.cpp
	network.cpp
)

target_include_directories(hotstuff PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(hotstuff PRIVATE ${BOTAN_INCLUDE_DIR})
target_link_libraries(hotstuff PRIVATE ${BOTAN_LIBRARY} cereal::cereal fmt::fmt spdlog::spdlog)

add_executable(tests
	blockchain_test.cpp
	crypto_test.cpp
	network_test.cpp
	tests/util.cpp
)

include(Catch)

catch_discover_tests(tests)

target_include_directories(tests PRIVATE ${BOTAN_INCLUDE_DIR})
target_link_libraries(tests PRIVATE hotstuff Catch2::Catch2WithMain ${BOTAN_LIBRARY} cereal::cereal)
