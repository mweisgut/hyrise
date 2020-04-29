#include "micro_benchmark_basic_fixture.hpp"
#include <array>

using namespace opossum;

namespace {

struct JoinBenchmarkConfiguration {
	std::string a{};
	std::string b{};
};

const auto configurations = std::vector<JoinBenchmarkConfiguration> {
	JoinBenchmarkConfiguration{"Hash", "Index"},
	JoinBenchmarkConfiguration{"SortMerge", "NestedLoop"}
};

static void all_configurations_indices(benchmark::internal::Benchmark* benchmark) {
  for (size_t i = 0; i < configurations.size(); ++i) {
      benchmark->Args({static_cast<int64_t>(i)});
  }
}

// the string (name of the used function is passed later)
static void join_benchmark(benchmark::State& state, std::string) {
  // cout for testing purposes
  const auto configuration = configurations[state.range(0)];
  std::cout << configuration.a << ", " << configuration.b << "\n";
  for (auto _ : state) {
     // do whatever with i and j and func_name
  }
}

BENCHMARK_CAPTURE(join_benchmark, JoinIndexBenchmark, "")
  -> Apply(all_configurations_indices);

}  // namespace opossum