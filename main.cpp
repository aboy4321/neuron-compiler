#include "neuron_compiler.h"

void dfs(ThresholdTest& test, Counter& counter);

int main() {
  //std::vector<int> weights = {2, -1, 3};
  std::vector<int> weights = { 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1 };

    ThresholdTest test(weights, 2);
    std::cout << "original threshold:  " << test.get_threshold() << std::endl;
    auto test_1 = test.set_last(1);
    std::cout << "new threshold after set_last:  " << test_1->get_threshold() << std::endl;

    if (test_1->is_trivial_pass()) std::cout << "trivial pass  " << std::endl;
    if (test_1->is_trivial_fail()) std::cout << "trivial fail  " << std::endl;

    std::cout << "dfs" << std::endl;
    Counter counter(test.get_size());
    dfs(test,counter);
    std::cout << "dfs-passes: " << counter.get_passes() << std::endl;
    std::cout << "dfs-fails:  " << counter.get_fails() << std::endl;

    return 0;
}

