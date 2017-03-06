#include <chrono>
#include <thread>
#include <random>

#include <cmath>

#include <gtest/gtest.h>

#include <metrics/accumulator/decaying/exponentially.hpp>

namespace metrics {
namespace accumulator {
namespace decaying {

constexpr auto RANDOM_SEED = 100500;
constexpr auto EPSILON = 150.0;

TEST(exponentially_t, Accumulate100OutOf1000Elements) {
    exponentially_t accumulator(100, 0.99, std::chrono::milliseconds(10), RANDOM_SEED);

    for (int i = 0; i < 1000; ++i) {
        accumulator.update(i);
    }

    EXPECT_EQ(100, accumulator.size());

    const auto snapshot = accumulator.snapshot();
    EXPECT_EQ(100, snapshot.size());

    for (auto value : snapshot.values()) {
        ASSERT_TRUE(value <= 1000);
    }

    EXPECT_NEAR(snapshot.mean(), 500, EPSILON);
    EXPECT_NEAR(snapshot.stddev(), 288, EPSILON);

    EXPECT_NEAR(snapshot.min(), 0, EPSILON);
    EXPECT_NEAR(snapshot.max(), 1000, EPSILON);
}

TEST(exponentially_t, rescale) {
    exponentially_t accumulator(100, 0.05, std::chrono::milliseconds(10), RANDOM_SEED);

    for (int i = 0; i < 1000; ++i) {
        if (i % 100 == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }

        accumulator.update(i);
    }

    const auto snapshot = accumulator.snapshot();

    EXPECT_NEAR(snapshot.mean(), 500, EPSILON);
    EXPECT_NEAR(snapshot.stddev(), 288, EPSILON);

    EXPECT_NEAR(snapshot.min(), 0, EPSILON);
    EXPECT_NEAR(snapshot.max(), 1000, EPSILON);
}

struct marginal_case_t {
    double size;
    double alpha;

    double mean;
    double stddev;

    double expect_mean;
    double expect_std;

    double expect_min;
    double expect_max;

    double epsilon1;
};

class margins_test : public ::testing::TestWithParam<marginal_case_t> {};

TEST_P(margins_test, marginal_cases_test) {

    const auto &test = GetParam();

    std::mt19937 gen; // defaults to seed = 5489u
    std::normal_distribution<> norm{test.mean, test.stddev};

    exponentially_t accumulator(test.size, test.alpha, std::chrono::milliseconds{5}, RANDOM_SEED);

    for(int i = 0; i < 1000; ++i) {
        accumulator(abs(norm(gen)) % 1000);
    }

    const auto snapshot = accumulator.snapshot();

    EXPECT_NEAR(snapshot.mean(), test.expect_mean, test.epsilon1);
    EXPECT_NEAR(snapshot.stddev(), test.expect_std, test.epsilon1);

    EXPECT_NEAR(snapshot.min(), test.expect_min, test.epsilon1);
    EXPECT_NEAR(snapshot.max(), test.expect_max, test.epsilon1);
}

INSTANTIATE_TEST_CASE_P(exponentially_t, margins_test,
    ::testing::Values(
        //              SIZE,    ALPHA, AVG, STD, EXP_AVG, EXP_STD, MIN, MAX, EPS1
        marginal_case_t{1000,    0.001, 500, 288, 464.308, 248.085,   1,  994, 1e-3},
        marginal_case_t{1000,    13.10, 500, 288, 464.308, 248.085,   1,  994, 1e-3},

        marginal_case_t{ 100,   0.0001, 500, 288,  440.94, 237.772,   8,  942, 1e-3},
        marginal_case_t{ 100,    0.001, 500, 288,  440.94, 237.772,   8,  942, 1e-3},
        marginal_case_t{ 100,    13.10, 500, 288,  440.94, 237.772,   8,  942, 1e-3},
        marginal_case_t{ 100, 100500.0, 500, 288,  440.94, 237.772,   8,  942, 1e-3},

        marginal_case_t{  10,    0.001, 500, 288,   611.7, 255.597,  53,  942, 1e-3},
        marginal_case_t{  10,    13.10, 500, 288,   611.7, 255.597,  53,  942, 1e-3}
    ));

}  // namespace decaying
}  // namespace accumulator
}  // namespace metrics
