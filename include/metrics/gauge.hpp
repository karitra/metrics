#pragma once

#include "fwd.hpp"

namespace metrics {
inline namespace v2 {

/// A gauge is the simplest metric type. It just returns a value.
///
/// Useful if, for example, your application has a value which is maintained by a third-party
/// library, you can easily expose it by registering a gauge instance which returns that value.
template <typename T>
using gauge = std::function<T()>;

} // namespace v2
} // namespace metrics
