#include "metrics/metric/registry.hpp"

#include "metrics/metric/counter.hpp"
#include "metrics/metric/gauge.hpp"

#include "metrics/detail/metric/processor.hpp"

namespace metrics {
namespace metric {

template<class M>
struct metric_traits;

template<typename T>
struct metric_traits<gauge<T>> {
    static void
    apply(processor_t& processor, std::string name, std::function<T()> metric) {
        processor.post([&, name, metric] {
            processor.gauges<T>().insert({std::move(name), std::move(metric)});
        });
    }
};

registry_t::registry_t():
    processor(new processor_t)
{}

registry_t::~registry_t() {}

template<typename M>
void
registry_t::listen(std::string name, std::function<typename M::value_type()> metric) {
    metric_traits<M>::apply(*processor, std::move(name), std::move(metric));
}

template<typename T>
gauge<T>
registry_t::gauge(const std::string& name) const {
    return metric::gauge<T>(name, *processor);
}

template<typename T>
counter<T>
registry_t::counter(const std::string& name) const {
    return metrics::metric::counter<T>(name, *processor);
}

meter_t
registry_t::meter(const std::string& name) const {
    return meter_t(name, *processor);
}

/// Instantiations.
template
void
registry_t::listen<gauge<std::uint64_t>>(std::string name, std::function<std::uint64_t()> metric);

template
gauge<std::uint64_t>
registry_t::gauge<std::uint64_t>(const std::string&) const;

template
counter<std::int64_t>
registry_t::counter<std::int64_t>(const std::string&) const;

template
counter<std::uint64_t>
registry_t::counter<std::uint64_t>(const std::string&) const;

}  // namespace metric
}  // namespace metrics