#pragma once

#include <cmath>
#include <map>
#include <thread>

#include <boost/asio/io_service.hpp>
#include <boost/optional/optional.hpp>

#include "metrics/detail/metric/ewma.hpp"
#include "metrics/detail/metric/meter.hpp"
#include "metrics/detail/metric/utility.hpp"

namespace metrics {
namespace detail {
namespace metric {

// template<typename... U>
// struct counters_t {
//     std::tuple<std::map<std::string, U>...> containers;
//
//     template<typename T>
//     std::map<std::string, T>&
//     get() {
//         return std::get<T>(containers);
//     }
// };

/// \internal
class processor_t {
    boost::asio::io_service loop;
    boost::optional<boost::asio::io_service::work> work;

    std::thread thread;

    /// Data.
    struct {
        struct {
            std::map<std::string, std::function<std::uint64_t()>> u64;
        } gauges;

        struct {
            std::map<std::string, std::int64_t>  i64;
            std::map<std::string, std::uint64_t> u64;
        } counters;

        std::map<std::string, meter_t> meters;
    } data;

public:
    processor_t()
        : work(boost::optional<boost::asio::io_service::work>(boost::asio::io_service::work(loop))),
          thread([&] { loop.run(); }) {}

    ~processor_t() {
        work.reset();
        thread.join();
    }

    template<class F>
    void
    post(F fn) {
        loop.post(std::move(fn));
    }

    /// \warning must be called from event loop's thread.
    template<typename T>
    const std::map<std::string, std::function<T()>>&
    gauges() const;

    /// \overload
    /// \warning must be called from event loop's thread.
    template<typename T>
    std::map<std::string, std::function<T()>>&
    gauges() {
        typedef std::map<
            std::string,
            std::function<T()>
        >& result_type;

        return const_cast<result_type>(static_cast<const processor_t&>(*this).gauges<T>());
    }

    /// \warning must be called from event loop's thread.
    template<typename T>
    const std::map<std::string, T>&
    counters() const;

    /// \overload
    /// \warning must be called from event loop's thread.
    template<typename T>
    std::map<std::string, T>&
    counters() {
        typedef std::map<
            std::string,
            T
        >& result_type;

        return const_cast<result_type>(static_cast<const processor_t&>(*this).counters<T>());
    }

    /// \warning must be called from event loop's thread.
    template<typename T>
    boost::optional<std::function<T()>>
    gauge(const std::string& name) const {
        const auto& gauges = this->gauges<T>();
        const auto it = gauges.find(name);

        if (it != gauges.end()) {
            return boost::make_optional(it->second);
        } else {
            return boost::none;
        }
    }

    /// \warning must be called from event loop's thread.
    template<typename T>
    T&
    counter(const std::string& name) {
        return counters<T>()[name];
    }

    /// \warning must be called from event loop's thread.
    meter_t&
    meter(const std::string& name) {
        return data.meters[name];
    }
};

template<>
inline
const std::map<std::string, std::function<std::uint64_t()>>&
processor_t::gauges<std::uint64_t>() const {
    return data.gauges.u64;
}

template<>
inline
const std::map<std::string, std::int64_t>&
processor_t::counters<std::int64_t>() const {
    return data.counters.i64;
}

template<>
inline
const std::map<std::string, std::uint64_t>&
processor_t::counters<std::uint64_t>() const {
    return data.counters.u64;
}

}  // namespace metric
}  // namespace detail
}  // namespace metrics