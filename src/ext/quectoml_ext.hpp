#pragma once

#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
#include <sys/times.h>
#endif

#include "../inc/commondefs.hpp"
#include "../inc/timespec_wrapper.hpp"
#include "../inc/containers/circular_buffer.hpp"
#include "../inc/mt_synchro.hpp"
#include "../inc/math/math_util.hpp"
#include "../inc/ann.hpp"

#include "../dfi_value.hpp"
#include "../dfi_util.hpp"
#include "../dfi_interfaces.hpp"

namespace dfi {

    class quectoml_ext: public extension_interface {
    public:
        quectoml_ext() = default;
        ~quectoml_ext() {
            unregister_runtime();
        }
        quectoml_ext(quectoml_ext const &) = delete;
        quectoml_ext &operator=(quectoml_ext const &) = delete;
        quectoml_ext(quectoml_ext &&) = delete;
        quectoml_ext &operator=(quectoml_ext &&) = delete;

        void register_runtime(runtime_interface *rt) override {
            std::unique_lock l{rt_mtp_};
            if(rt_ != nullptr) {
                return;
            }
            rt_ = rt;
            if(rt_ == nullptr) {
                return;
            }
            rt->add_function("nn_layer", DFIFUN() {
                return dfi::valbox{std::make_shared<layer<float>>(), "nn_layer"};
            });
            rt->add_method("nn_layer", "at", DFIFUN(args) {
                DFI_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 4)
                std::int64_t x{0}; if(args.size() > 0) { x = DFINUMARG(args, 0, std::int64_t); }
                std::int64_t y{0}; if(args.size() > 1) { y = DFINUMARG(args, 1, std::int64_t); }
                std::int64_t z{0}; if(args.size() > 2) { z = DFINUMARG(args, 2, std::int64_t); }
                return dfi::valbox{DFITHIS(args, std::shared_ptr<layer<float>>)->at(x, y ,z), "nn_neuron"};
            });
            rt->add_method("nn_layer", "recalc", DFIFUN(args) {
                DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                DFITHIS(args, std::shared_ptr<layer<float>>)->recalc();
                return true;
            });
            rt->add_method("nn_layer", "connect_full", DFIFUN(args) {
                DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                DFITHIS(args, std::shared_ptr<layer<float>>)->recalc();
                return true;
            });

            rt->add_method("nn_neuron", "value", DFIFUN(args) {
                DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return DFITHIS(args, std::shared_ptr<neuron<float>>)->value();
            });
            rt->add_method("nn_neuron", "set_value", DFIFUN(args) {
                DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                DFITHIS(args, std::shared_ptr<neuron<float>>)->set_value(args[1].cast_to_float());
                return true;
            });
            rt->add_method("nn_neuron", "set_value_and_normalize", DFIFUN(args) {
                DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                DFITHIS(args, std::shared_ptr<neuron<float>>)->set_value_and_normalize(args[1].cast_to_float());
                return true;
            });
            rt->add_method("nn_neuron", "bias", DFIFUN(args) {
                DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return DFITHIS(args, std::shared_ptr<neuron<float>>)->bias_weight();
            });
            rt->add_method("nn_neuron", "set_bias", DFIFUN(args) {
                DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                DFITHIS(args, std::shared_ptr<neuron<float>>)->set_bias_weight(args[1].cast_to_float());
                return true;
            });
            rt->add_method("nn_neuron", "connect", DFIFUN(args) {
                DFI_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 2, 3)
                float w{}; if(args.size() > 2) { w = args[2].cast_to_float(); }
                DFITHIS(args, std::shared_ptr<neuron<float>>)->connect(
                    DFICLASSARG(args, 1, std::shared_ptr<neuron<float>>), w
                );
                return true;
            });
            rt->add_method("nn_neuron", "inputs", DFIFUN(args) {
                DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return DFITHIS(args, std::shared_ptr<neuron<float>>)->num_of_inputs();
            });
            rt->add_method("nn_neuron", "input", DFIFUN(args) {
                DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                return dfi::valbox{
                    DFITHIS(args, std::shared_ptr<neuron<float>>)->input(args[1].cast_to_size_t()),
                    "nn_neuron"
                };
            });
            rt->add_method("nn_neuron", "weight", DFIFUN(args) {
                DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                return DFITHIS(args, std::shared_ptr<neuron<float>>)->weight(args[1].cast_to_size_t());
            });
            rt->add_method("nn_neuron", "set_weight", DFIFUN(args) {
                DFI_CHCK_FUN_PARMS_NUM_EQ(args, 3)
                DFITHIS(args, std::shared_ptr<neuron<float>>)->set_weight(
                    args[1].cast_to_size_t(),
                    args[2].cast_to_float()
                );
                return true;
            });
            rt->add_method("nn_neuron", "fortrain", DFIFUN(args) {
                DFI_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 2, 3)
                float w{}; if(args.size() > 2) { w = args[2].cast_to_float(); }
                DFITHIS(args, std::shared_ptr<neuron<float>>)->fortrain(
                    DFICLASSARG(args, 1, std::shared_ptr<neuron<float>>), w
                    );
                return true;
            });
        }
        void unregister_runtime() override {
            std::unique_lock l{rt_mtp_};
            if(rt_ == nullptr) {
                return;
            }
            rt_->remove_function("nn_layer");
            rt_->remove_method("nn_layer", "at");
            rt_->remove_method("nn_layer", "recalc");
            rt_->remove_method("nn_layer", "connect_full");
            rt_->remove_function("nn_neuron");
            rt_->remove_method("nn_neuron", "self_cpu_consumption");
            rt_ = nullptr;
        }

    private:
        shared_mutex rt_mtp_{};
        runtime_interface *rt_{nullptr};
    };

}
