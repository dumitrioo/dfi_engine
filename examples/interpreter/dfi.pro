QT       -= core
QT       -= gui
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

# DEFINES += DFI_SINGLE_THREADED
# DEFINES += DFI_DEBUGGING
# DEFINES += DFI_JSON_DEBUGGING

# DEFINES += STR_UTIL_ENABLE_CUSTOM_UNICODE_OPERATIONS
# DEFINES += USE_CUSTOM_MEMORY_ALLOCATION

DEFINES += DFI_USE_CUSTOM_ANY

# DEFINES += DFI_USE_CUSTOM_SHARED_MUTEX
# DEFINES += RW_MUTEX_PRIORITIES
# DEFINES += RW_MUTEX_UPGRADEABLE

# DEFINES += DFI_TCPSERVER_USE_SSL
DEFINES += DFI_USE_EIGEN

INCLUDEPATH += ../../src

SOURCES += main.cpp

HEADERS += \
    ../../src/ext/array_buffer_ext.hpp \
    ../../src/ext/containers_ext.hpp \
    ../../src/ext/eigen_ext.hpp \
    ../../src/ext/socket_ext.hpp \
    ../../src/ext/geo_ext.hpp \
    ../../src/inc/any.hpp \
    ../../src/inc/base16.hpp \
    ../../src/inc/base64.hpp \
    ../../src/inc/base85.hpp \
    ../../src/inc/binned_allocator.hpp \
    ../../src/inc/bit_util.hpp \
    ../../src/inc/command_queue.hpp \
    ../../src/inc/commondefs.hpp \
    ../../src/inc/containers/buffer_queue.hpp \
    ../../src/inc/containers/circular_buffer.hpp \
    ../../src/inc/containers/concurrentqueue.h \
    ../../src/inc/containers/static_buffer.hpp \
    ../../src/inc/crypto/gamma.hpp \
    ../../src/inc/crypto/keccak.hpp \
    ../../src/inc/crypto/sha256.hpp \
    ../../src/inc/crypto/sha3_512.hpp \
    ../../src/inc/crypto/sha512.hpp \
    ../../src/inc/crypto/threefish.hpp \
    ../../src/inc/emhash/hash_set2.hpp \
    ../../src/inc/emhash/hash_set3.hpp \
    ../../src/inc/emhash/hash_set4.hpp \
    ../../src/inc/emhash/hash_set8.hpp \
    ../../src/inc/emhash/hash_table5.hpp \
    ../../src/inc/emhash/hash_table6.hpp \
    ../../src/inc/emhash/hash_table7.hpp \
    ../../src/inc/emhash/hash_table8.hpp \
    ../../src/inc/emhash/lru_size.h \
    ../../src/inc/emhash/lru_time.h \
    ../../src/inc/file_util.hpp \
    ../../src/inc/fsm_tokenizer.hpp \
    ../../src/inc/geocoordinates.hpp \
    ../../src/inc/hash/adler.hpp \
    ../../src/inc/hash/crc.hpp \
    ../../src/inc/hash/hash.hpp \
    ../../src/inc/http/httplib.h \
    ../../src/inc/json.hpp \
    ../../src/inc/lzari.hpp \
    ../../src/inc/math/math_util.hpp \
    ../../src/inc/math/matrix4.hpp \
    ../../src/inc/math/vector4.hpp \
    ../../src/inc/mt_synchro.hpp \
    ../../src/inc/net/net_data_transfer.hpp \
    ../../src/inc/net/net_utils.hpp \
    ../../src/inc/net/socket_poller.hpp \
    ../../src/inc/net/socket_wrapper.hpp \
    ../../src/inc/net/tcp_client.hpp \
    ../../src/inc/net/tcp_server.hpp \
    ../../src/inc/net/udp_client_muxed.hpp \
    ../../src/inc/net/udp_server_muxed.hpp \
    ../../src/inc/net/url.hpp \
    ../../src/inc/sequence_generator.hpp \
    ../../src/inc/serialization.hpp \
    ../../src/inc/so.hpp \
    ../../src/inc/str_util.hpp \
    ../../src/inc/sys_util.hpp \
    ../../src/inc/terminable.hpp \
    ../../src/inc/threaded_queue_processing_base.hpp \
    ../../src/inc/timespec_wrapper.hpp \
    ../../src/inc/unicode_operations.hpp \
    ../../src/ext/cpu_ext.hpp \
    ../../src/ext/crypto_ext.hpp \
    ../../src/ext/file_ext.hpp \
    ../../src/ext/math_ext.hpp \
    ../../src/ext/persistence_ext.hpp \
    ../../src/ext/pid_ext.hpp \
    ../../src/ext/quectoml_ext.hpp \
    ../../src/ext/rand_ext.hpp \
    ../../src/ext/strings_ext.hpp \
    ../../src/ext/time_ext.hpp \
    ../../src/dfi_cells.hpp \
    ../../src/dfi_net.hpp \
    ../../src/dfi_codegen.hpp \
    ../../src/dfi_console.hpp \
    ../../src/dfi_exec_ctx.hpp \
    ../../src/dfi_expr.hpp \
    ../../src/dfi_expr_poison.hpp \
    ../../src/dfi_interfaces.hpp \
    ../../src/dfi_lexer.hpp \
    ../../src/dfi_parser.hpp \
    ../../src/dfi_runtime.hpp \
    ../../src/dfi_statement.hpp \
    ../../src/dfi_token.hpp \
    ../../src/dfi_util.hpp \
    ../../src/dfi_value.hpp

QMAKE_CXXFLAGS += -std=c++20 -march=native -Wno-unused-parameter -Wno-unused-function -Wl,-rpath,.
# QMAKE_CXXFLAGS += -ftree-vectorize -mavx2 -ftree-vectorizer-verbose=5
QMAKE_CXXFLAGS += -mavx2
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3
QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE += -O3

LIBS += -lpthread -ldl

DISTFILES += \
    ../../README.md \
    ../alu74181.dfi \
    ../alu74181_model.dfi \
    ../alu74181_view.dfi \
    ../array_buffer_test.dfi \
    ../draft.dfi \
    ../ex_cli.dfi \
    ../ex_srv.dfi \
    ../alu74181.png \
    ../example.dfi \
    ../extending_example.dfi \
    ../external_client.dfi \
    ../external_server.dfi \
    ../external_value.dfi \
    ../fake_vehicle/doors.dfi \
    ../fake_vehicle/sleep_mode.dfi \
    ../fake_vehicle/inputs.dfi \
    ../fake_vehicle/intermediate.dfi \
    ../fake_vehicle/utils.dfi \
    ../fake_vehicle/x_vehicle.dfi \
    ../kalman_1D_example.dfi \
    ../one_second_limit.dfi \
    ../perf_limit.dfi \
    ../pid_regulator.dfi \
    ../pwm.dfi \
    ../quad_eq.dfi \
    ../sockets_server.dfi \
    ../tap_lights.dfi \
    ../tbbt_2cola.dfi \
    ../tests.dfi \
    ../unix_socket.dfi
