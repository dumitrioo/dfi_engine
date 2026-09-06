#pragma once

#include "commondefs.hpp"

namespace dfi {

    template<typename T>
    class sequence_generator {
    public:
        sequence_generator(T starting = 0):
            stored_{starting}
        {
        }

        T next() {
            return stored_++;
        }

        T curr() const {
            return stored_;
        }

        operator T() {
            return next();
        }

        T operator()() {
            return next();
        }

        void reset(T val = 0) {
            stored_ = val;
        }

    private:
        T stored_{0};
    };


    template<typename T>
    class alignas(64) atomic_sequence_generator {
    public:
        atomic_sequence_generator(T starting = 0) {
            stored_.store(starting, std::memory_order_relaxed);
        }

        T next() {
            return stored_.fetch_add(static_cast<T>(1), std::memory_order_relaxed);
        }

        T curr() const {
            return stored_.load(std::memory_order_relaxed);
        }

        operator T() { return next(); }

        T operator()() { return next(); }

        void reset(T val = 0) {
            stored_.store(val, std::memory_order_relaxed);
        }

    private:
        std::atomic<T> stored_{0};
    };

}
