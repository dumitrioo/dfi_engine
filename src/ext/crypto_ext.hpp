#pragma once

#include "../inc/commondefs.hpp"
#include "../inc/crypto/sha256.hpp"
#include "../inc/crypto/sha512.hpp"

#include "../dfi_value.hpp"
#include "../dfi_util.hpp"
#include "../dfi_interfaces.hpp"

namespace dfi {

    class crypto_ext: public extension_interface {
    public:
        crypto_ext() = default;
        ~crypto_ext() {
            unregister_runtime();
        }
        crypto_ext(crypto_ext const &) = delete;
        crypto_ext &operator=(crypto_ext const &) = delete;
        crypto_ext(crypto_ext &&) = delete;
        crypto_ext &operator=(crypto_ext &&) = delete;

        void register_runtime(runtime_interface *rt) override {
            std::unique_lock l{rt_mtp_};
            if(rt_ != nullptr) {
                return;
            }
            rt_ = rt;
            if(rt_ == nullptr) {
                return;
            }
            rt->add_function("sha256", DFIFUN(args) {
                if(args.size() == 1) {
                    std::string s{args[0].cast_to_string()};
                    std::array<std::uint8_t, dfi::crypt::sha256::DIGEST_SIZE> h{dfi::crypt::sha256sum(s.data(), s.size())};
                    return dfi::valbox{std::string{h.begin(), h.end()}};
                }
                return dfi::valbox{std::string{}};
            });
            rt->add_function("sha512", DFIFUN(args) {
                if(args.size() == 1) {
                    std::string s{args[0].cast_to_string()};
                    std::array<std::uint8_t, dfi::crypt::sha512::digest_size()> h{dfi::crypt::sha512sum(s.data(), s.size())};
                    return dfi::valbox{std::string{h.begin(), h.end()}};
                }
                return dfi::valbox{std::string{}};
            });
        }

        void unregister_runtime() override {
            std::unique_lock l{rt_mtp_};
            if(rt_ == nullptr) {
                return;
            }
            rt_->remove_function("sha256");
            rt_->remove_function("sha512");
            rt_ = nullptr;
        }

    private:
        shared_mutex rt_mtp_{};
        runtime_interface *rt_{nullptr};
    };

}
