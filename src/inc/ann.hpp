#pragma once

#include "commondefs.hpp"
#include "math/math_util.hpp"
#include "json.hpp"

namespace dfi {

    template<typename T>
    class neuron {
    public:
        neuron() = default;
        neuron(const neuron &) = delete;
        neuron &operator=(const neuron &) = delete;
        neuron(neuron &&) = default;
        neuron &operator=(neuron &&) = default;
        ~neuron() = default;

        T value() const {
            return v_;
        }

        operator T() const {
            return value();
        }

        void set_value(T v) {
            v_ = v;
        }

        void set_value_and_normalize(T v) {
            if(!afn_) {
                return;
            }
            v_ = afn_(v);
        }

        T bias_weight() const {
            return wb_;
        }

        void set_bias_weight(T v) {
            wb_ = v;
        }

        void connect(std::shared_ptr<neuron> const &n, T w) {
            in_.emplace_back(n, w);
        }

        std::shared_ptr<neuron> input(std::size_t no) {
            if(no >= in_.size()) {
                return {};
            }
            return in_[no].lock();
        }

        T weight(std::size_t no) const {
            if(no >= in_.size()) {
                return {};
            }
            return in_.at(no).second;
        }

        void set_weight(std::size_t no, T w) {
            if(no >= in_.size()) {
                throw std::out_of_range{"index out of range"};
            }
            in_[no].second = w;
        }

        std::size_t num_of_inputs() const {
            return in_.size();
        }

        void fortrain(T target_val, T actual_val, T lrate, std::function<T()> const &recalculator) {
            if(!afn_) {
                return;
            }
            T err{target_val - actual_val};
            T delta{math::sign(err) * pow(abs(err), 0.3 /*0.6*/ /*1.5*/) * lrate};
            std::size_t insz{in_.size()};
            for(std::size_t i = 0; i < insz; ++i) {
                std::pair<std::weak_ptr<neuron>, T> &nw{in_[i]};
                std::shared_ptr<neuron> n_ptr{nw.first.lock()};
                if(n_ptr) {
                    T prevw{nw.second};
                    nw.second += delta;
                    T new_actual_val{recalculator()};
                    T new_err{target_val - new_actual_val};
                    if(abs(err) < abs(new_err)) {
                        nw.second = prevw - delta;
                        T new_actual_val2{recalculator()};
                        T new_err2{target_val - new_actual_val2};
                        if(abs(err) < abs(new_err2)) {
                            nw.second = prevw;
                            recalculator();
                        }
                    }
                }
            }
            T prevwb{wb_};
            wb_ += delta;
            T new_actual_val{recalculator()};
            T new_err{target_val - new_actual_val};
            if(abs(err) < abs(new_err)) {
                wb_ = prevwb - delta;
                T new_actual_val2{recalculator()};
                T new_err2{target_val - new_actual_val2};
                if(abs(err) < abs(new_err2)) {
                    wb_ = prevwb;
                    recalculator();
                }
            }
        }

        void recalc() {
            if(!afn_) {
                return;
            }
            T v{0};
            auto insz{in_.size()};
            for(std::size_t i{0}; i < insz; ++i) {
                std::pair<std::weak_ptr<neuron>, T> &nw{in_[i]};
                std::shared_ptr<neuron> n_ptr{nw.first.lock()};
                if(n_ptr) {
                    T in_val{n_ptr->value()};
                    T const &w{nw.second};
                    v += w * in_val;
                }
            }
            v += wb_;
            v_ = afn_(v);
        }

        void set_act_fun(std::function<T(T)> const &f) {
            afn_ = f;
        }

        void set_act_fun(std::function<T(T)> &&f) {
            afn_ = std::move(f);
        }

    private:
        std::function<T(T)> afn_{math::sigmoid<T>/*[](T x) { return math::sigmoid<T>(x); }*/};
        std::vector<std::pair<std::weak_ptr<neuron>, T>> in_{};
        T wb_{0};
        T v_{0};
    };


    template<typename T>
    class layer {
    public:
        layer() = default;
        layer(const layer &) = delete;
        layer &operator=(const layer &) = delete;
        layer(layer &&) = delete;
        layer &operator=(layer &&) = delete;
        ~layer() = default;

        std::shared_ptr<neuron<T>> at(std::int64_t x, std::int64_t y = 0, std::int64_t z = 0) {
            std::shared_ptr<neuron<T>> &res{l_[z][y][x]};
            if(!res) {
                res = std::make_shared<neuron<T>>();
                res->set_act_fun(afn_);
                l_[z][y][x] = res;
            }
            return *res;
        }

        std::shared_ptr<neuron<T>> at(std::int64_t x, std::int64_t y = 0, std::int64_t z = 0) const {
            if(
                l_.find(z) != l_.end() &&
                l_.at(z).find(y) != l_.at(z).end() &&
                l_.at(z).at(y).find(x) != l_.at(z).at(y).end()
            ) {
                return *l_.at(z).at(y).at(x);
            }
            throw std::runtime_error{"item not found by index"};
        }

        void for_each(std::function<void(std::shared_ptr<neuron<T>> const &, std::int64_t, std::int64_t, std::int64_t)> const &f) const {
            for(auto &&z: l_) {
                std::int64_t iz{z.first};
                for(auto &&y: z.second) {
                    std::int64_t iy{y.first};
                    for(auto &&x: y.second) {
                        std::int64_t ix{x.first};
                        f(x.second, ix, iy, iz);
                    }
                }
            }
        }

        void for_each(std::function<void(std::shared_ptr<neuron<T>> &, std::int64_t, std::int64_t, std::int64_t)> const &f) {
            for(auto &&z: l_) {
                std::int64_t iz{z.first};
                for(auto &&y: z.second) {
                    std::int64_t iy{y.first};
                    for(auto &&x: y.second) {
                        std::int64_t ix{x.first};
                        f(x.second, ix, iy, iz);
                    }
                }
            }
        }

        void recalc() {
            for_each([](std::shared_ptr<neuron<T>> &n, std::int64_t /*x*/, std::int64_t /*y*/, std::int64_t /*z*/) {
                n->recalc();
            });
        }

        void connect_full(std::shared_ptr<layer> const &other, T w) {
            for_each([&](std::shared_ptr<neuron<T>> &n, std::int64_t /*x*/, std::int64_t /*y*/, std::int64_t /*z*/) {
                other->for_each([&](std::shared_ptr<neuron<T>> &n_oth, std::int64_t /*x_oth*/, std::int64_t /*y_oth*/, std::int64_t /*z_oth*/) {
                    n->connect(n_oth, w);
                });
                n->set_bias_weight(w);
            });
        }

        void connect_full_rand(std::shared_ptr<layer> const &other, T rmin = 0, T rmax = 1) {
            std::random_device rd{};
            std::mt19937 re{rd()};
            std::uniform_real_distribution<T> urd{std::min(rmin, rmax), std::max(rmin, rmax)};
            for_each([&](std::shared_ptr<neuron<T>> &n, std::int64_t /*x*/, std::int64_t /*y*/, std::int64_t /*z*/) {
                other->for_each([&](std::shared_ptr<neuron<T>> &n_oth, std::int64_t /*x_oth*/, std::int64_t /*y_oth*/, std::int64_t /*z_oth*/) {
                    n->connect(n_oth, urd(re));
                });
                n->set_bias_weight(urd(re));
            });
        }

        void reset_all_rand(T rmin = 0, T rmax = 1) {
            std::random_device rd{};
            std::mt19937 re{rd()};
            std::uniform_real_distribution<T> urd{std::min(rmin, rmax), std::max(rmin, rmax)};
            for_each([&](std::shared_ptr<neuron<T>> &n, std::int64_t /*x*/, std::int64_t /*y*/, std::int64_t /*z*/) {
                for(int indx{}; indx < n->num_of_inputs(); ++indx) {
                    n->set_weight(indx, urd(re));
                }
                n->set_bias_weight(urd(re));
            });
        }

        void set_act_fun(std::function<T(T)> const &f) {
            afn_ = f;
            for_each([&](std::shared_ptr<neuron<T>> &n, std::int64_t /*x*/, std::int64_t /*y*/, std::int64_t /*z*/) {
                n->set_act_fun(afn_);
            });
        }

        void set_act_fun(std::function<T(T)> &&f) {
            afn_ = std::move(f);
            for_each([&](std::shared_ptr<neuron<T>> &n, std::int64_t /*x*/, std::int64_t /*y*/, std::int64_t /*z*/) {
                n->set_act_fun(std::move(afn_));
            });
        }

        void clear() {
            l_.clear();
        }

        void fortrain(T target_val, T actual_val, T lrate, std::function<T()> const &recalculator) {
            for_each([target_val, actual_val, lrate, recalculator](std::shared_ptr<neuron<T>> &n, std::int64_t, std::int64_t, std::int64_t) {
                n->fortrain(target_val, actual_val, lrate, recalculator);
            });
        }

    private:
        std::function<T(T)> afn_{math::sigmoid<T>};
        std::map<std::int64_t, std::map<std::int64_t, std::map<std::int64_t, std::shared_ptr<neuron<T>>>>> l_{};
    };


    template<typename T>
    class multilayer {
    public:
        multilayer() = default;
        multilayer(const multilayer &) = delete;
        multilayer &operator=(const multilayer &) = delete;
        multilayer(multilayer &&) = delete;
        multilayer &operator=(multilayer &&) = delete;
        ~multilayer() = default;

        std::shared_ptr<layer<T>> at(std::size_t n) {
            if(n >= layers_.size()) {
                layers_.resize(n + 1);
            }
            if(!layers_[n]) {
                layers_[n] = std::make_shared<layer<T>>();
            }
            return layers_[n];
        }

        std::shared_ptr<layer<T>> const &at(std::size_t n) const {
            if(n >= layers_.size()) {
                throw std::runtime_error{"index out of range"};
            }
            if(!layers_[n]) {
                throw std::runtime_error{"layer not found by index"};
            }
            return layers_[n];
        }

        // first layer (index 0) can be pure data layer or it can be connected to some other layer
        // may be even from other multilayer or free layer...
        void recalc( std::shared_ptr<layer<T>> const &tgt_out) {
            if(layers_.empty()) {
                return;
            }
            for(int64_t i{0}; i < layers_.size(); ++i) {
                if(layers_[i]) {
                    layers_[i]->recalc();
                }
            }
        }

        std::shared_ptr<layer<T>> add_layer() {
            std::shared_ptr<layer<T>> res{at(layers_.size())};
            return res;
        }

        void reset_all_weights_rand(T rmin = 0, T rmax = 1) {
            for(int64_t i{0}; i < layers_.size(); ++i) {
                if(layers_[i]) {
                    layers_[i]->reset_all_rand(rmin, rmax);
                }
            }
        }

        void reset_weights_rand(std::int64_t indx, T rmin = 0, T rmax = 1) {
            at(indx)->reset_all_rand(rmin, rmax);
        }

        void reset_weights(T val) {
            for_each([&](neuron<T> *n, std::int64_t /*x*/, std::int64_t /*y*/, std::int64_t /*z*/) {
                for(size_t indx{}; indx < n->num_of_inputs(); ++indx) {
                    n->set_weight(indx, val);
                }
                n->set_bias_weight(val);
            });
        }

        void set_act_fun(std::function<T(T)> const &f) {
            afn_ = f;
            for_each([&](neuron<T> *n, std::int64_t /*x*/, std::int64_t /*y*/, std::int64_t /*z*/) {
                n->set_act_fun(afn_);
            });
        }

        void set_act_fun(std::function<T(T)> &&f) {
            afn_ = std::move(f);
            for_each([&](neuron<T> *n, std::int64_t /*x*/, std::int64_t /*y*/, std::int64_t /*z*/) {
                n->set_act_fun(std::move(afn_));
            });
        }

        void clear() {
            layers_.clear();
        }

        void fortrain(T target_val, T actual_val, T lrate, std::function<T()> const &recalculator) {
            if(lrate <= 0) {
                throw std::runtime_error{"invalid learning rate"};
            }
            for_each([target_val, actual_val, lrate, recalculator](neuron<T> *n, std::int64_t, std::int64_t, std::int64_t) {
                n->fortrain(target_val, actual_val, lrate, recalculator);
            });
        }

        json serialize() const {
            json res{};
            res.become_array();
            for_each([&](neuron<T> const *n, std::int64_t x, std::int64_t y, std::int64_t z) {
                json j{n->as_json()};
                j["x"] = x;
                j["y"] = y;
                j["z"] = z;
                res.push_back(j);
            });
            return res;
        }

    private:
        bool recalc_first_layer_{false};
        std::function<T(T)> afn_{math::sigmoid<T>};
        std::vector<std::shared_ptr<layer<T>>> layers_{};
    };

}
