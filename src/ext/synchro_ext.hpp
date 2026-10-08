#pragma once

#include "../inc/commondefs.hpp"
#include "../inc/sequence_generator.hpp"
#include "../inc/str_util.hpp"
#include "../inc/file_util.hpp"
#include "../inc/mt_synchro.hpp"

#include "../dfi_value.hpp"
#include "../dfi_util.hpp"
#include "../dfi_interfaces.hpp"

namespace dfi {

    class syncro_ext: public extension_interface {
    public:
        syncro_ext() = default;
        ~syncro_ext() {
            unregister_runtime();
        }
        syncro_ext(syncro_ext const &) = delete;
        syncro_ext &operator=(syncro_ext const &) = delete;
        syncro_ext(syncro_ext &&) = delete;
        syncro_ext &operator=(syncro_ext &&) = delete;

        void register_runtime(runtime_interface *rt) override {
            std::unique_lock l{rt_mtp_};
            if(rt_ != nullptr) {
                return;
            }
            rt_ = rt;
            if(rt_ == nullptr) {
                return;
            }

            rt->add_function("mutex", DFIFUN() {
                return dfi::valbox{std::make_shared<dfi::mt::atomic_spin_mutex>(), "mutex"};
            });
            rt->add_method("mutex", "lock", DFIFUN(args) {
                DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                DFITHIS(args, std::shared_ptr<dfi::mt::atomic_spin_mutex>)->lock();
                return true;
            });
            rt->add_method("mutex", "unlock", DFIFUN(args) {
                DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                DFITHIS(args, std::shared_ptr<dfi::mt::atomic_spin_mutex>)->unlock();
                return true;
            });
            rt->add_method("mutex", "try_lock", DFIFUN(args) {
                DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return DFITHIS(args, std::shared_ptr<dfi::mt::atomic_spin_mutex>)->try_lock();
            });


            rt->add_function("shared_mutex", DFIFUN() {
                return dfi::valbox{std::make_shared<dfi::mt::atomic_rw_spin_mutex>(), "shared_mutex"};
            });
            rt->add_method("shared_mutex", "lock", DFIFUN(args) {
                DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                DFITHIS(args, std::shared_ptr<dfi::mt::atomic_rw_spin_mutex>)->lock();
                return true;
            });
            rt->add_method("shared_mutex", "unlock", DFIFUN(args) {
                DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                DFITHIS(args, std::shared_ptr<dfi::mt::atomic_rw_spin_mutex>)->unlock();
                return true;
            });
            rt->add_method("shared_mutex", "try_lock", DFIFUN(args) {
                DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return DFITHIS(args, std::shared_ptr<dfi::mt::atomic_rw_spin_mutex>)->try_lock();
            });
            rt->add_method("shared_mutex", "lock_shared", DFIFUN(args) {
                DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                DFITHIS(args, std::shared_ptr<dfi::mt::atomic_rw_spin_mutex>)->lock_shared();
                return true;
            });
            rt->add_method("shared_mutex", "unlock_shared", DFIFUN(args) {
                DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                DFITHIS(args, std::shared_ptr<dfi::mt::atomic_rw_spin_mutex>)->unlock_shared();
                return true;
            });
            rt->add_method("shared_mutex", "try_lock_shared", DFIFUN(args) {
                DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return DFITHIS(args, std::shared_ptr<dfi::mt::atomic_rw_spin_mutex>)->try_lock_shared();
            });
        }

        void unregister_runtime() override {
            std::unique_lock l{rt_mtp_};
            if(rt_ == nullptr) {
                return;
            }
            rt_->remove_function("shared_mutex");
            rt_->remove_method("shared_mutex", "lock_shared");
            rt_->remove_method("shared_mutex", "unlock_shared");
            rt_->remove_method("shared_mutex", "try_lock_shared");
            rt_->remove_method("shared_mutex", "lock");
            rt_->remove_method("shared_mutex", "unlock");
            rt_->remove_method("shared_mutex", "try_lock");

            rt_->remove_function("mutex");
            rt_->remove_method("mutex", "lock");
            rt_->remove_method("mutex", "unlock");
            rt_->remove_method("mutex", "try_lock");
            rt_ = nullptr;
        }

    private:
        shared_mutex rt_mtp_{};
        runtime_interface *rt_{nullptr};
    };

}
