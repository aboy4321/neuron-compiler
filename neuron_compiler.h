#include <iostream>
#include <vector>
#include <deque>
#include <array>
#include <set>
#include <list>
#include <memory>
#include <algorithm>
#include <utility>

#ifndef NEURON_COMPILER_H_
#define NEURON_COMPILER_H_

class IntPair {
    public:
        int a;
        int b;

        void accumulate(const IntPair& count) {
            this->a += count.a;
            this->b += count.b;
        }

        IntPair(int one, int two) : a(one), b(two) {}
        IntPair() : a(0), b(0) {}
};

class Bounds {
    private:
        int lower_bound, upper_bound;

    public:
        Bounds(int lb, int ub) : lower_bound(lb), upper_bound(ub) {}

        static Bounds from_weights(const std::vector<int>& weights, int size) {
            int lb = 0;
            int ub = 0;
            int end_pt = std::min(size, static_cast<int>(weights.size()));

            for (int i = 0; i < end_pt; i++) {
                int weight = weights[i];
                if (weight < 0) {
                    lb += weight;
                }
                if (weight > 0) {
                    ub += weight;
                }
            }
            return Bounds(lb, ub);
        }

        IntPair get_bounds() const {
            return IntPair(this->lower_bound,this->upper_bound);
        }
};

class ThresholdTest {
    private:
        static int id_counter;
        std::vector<int> weights;
        std::vector<int> indices;
        int threshold;
        int size;
        Bounds bounds;

        // creates new id for object
        static int new_id() {
            return id_counter++;
        }

    public:
        int id;
        std::vector<ThresholdTest> parents;
        IntPair _data;
        IntPair test_counts;

        ThresholdTest(const std::vector<int>& weights, int threshold,
                 const std::vector<int>& indices, int size, const Bounds& bounds)
            : weights(weights), threshold(threshold), indices(indices),
              size(size), bounds(bounds), id(ThresholdTest::new_id()) {}

        ThresholdTest(const std::vector<int>& weights, int threshold)
            : weights(weights), threshold(threshold), indices(),
              size(weights.size()),
              bounds(Bounds::from_weights(weights, weights.size())),
              id(ThresholdTest::new_id()) {}

        // gets last weight from list
        int get_last() const {
            return weights[size - 1];
        }
        
        // sets last weight to either 0 or 1 and creates updated test 
        std::shared_ptr<ThresholdTest> set_last(int value) const {
            int last_weight = get_last();
            int nu_threshold = threshold;

            if (value == 1) {
                nu_threshold = threshold - last_weight;
            }

            auto [lb, ub] = bounds.get_bounds();

            if (last_weight > 0) {
                ub -= last_weight;
            } else {
                lb -= last_weight;
            }
            Bounds nu_bounds(lb, ub);

            return std::make_shared<ThresholdTest>(weights, nu_threshold, indices, size - 1, nu_bounds);
        }
        
        // sorts weights by magnitude        
        static std::pair<std::vector<int>, std::vector<int>> sort_weights(const std::vector<int>& weights) {
            std::vector<std::pair<int, int>> indexed_weights;
            for (size_t i = 0; i < weights.size(); i++) {
                indexed_weights.emplace_back(i, weights[i]);
            }

            sort(indexed_weights.begin(), indexed_weights.end(),
                    [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
                        return abs(a.second) < abs(b.second);
                    });

            std::vector<int> indices;
            std::vector<int> sorted_weights;
            for (const auto& pair : indexed_weights) {
                indices.push_back(pair.first);
                sorted_weights.push_back(pair.second);
            }

            return std::make_pair(indices, sorted_weights);
        }

        bool is_trivial_pass() const {
            auto [lower, upper] = bounds.get_bounds();
            return threshold <= lower /* && lower <= upper */;
        }

        bool is_trivial_fail() const {
            auto [lower, upper] = bounds.get_bounds();
            return /*lower <= upper && */ upper < threshold;
        }

        int get_threshold() const { return threshold; }
        int get_size() const { return size; }
};


class Counter {
    private:
        int size;
        int passes = 0;
        int fails = 0;
        int count = 0;
        double start_time;
        std::vector<double> count_times;
        
        int propagate_count(ThresholdTest* test, IntPair* counts) {
            std::deque<ThresholdTest*> queue;
            std::list<ThresholdTest*> visited_tests;
            std::set<int> visited_ids;

            visited_tests.push_back(test);
            visited_ids.insert(test->id);

            test->_data.accumulate(*counts);
            test->test_counts.accumulate(*counts);

            for (auto& parent : test->parents) {
                parent._data.accumulate(test->_data);
                parent.test_counts.accumulate(test->_data);

                queue.push_back(&parent);
            }
            
            ThresholdTest* current_test = nullptr;
            IntPair root_count;


            while (!queue.empty()) {
               current_test = queue.front();
               queue.pop_front();

               if (visited_ids.count(current_test->id)) {
                   continue;
               }

               visited_tests.push_back(current_test);
               visited_ids.insert(current_test->id);

               root_count = current_test->_data;

               for (auto& parent : current_test->parents) {
                   parent._data.accumulate(current_test->_data);
                   parent.test_counts.accumulate(current_test->_data);

                   queue.push_back(&parent);
               }

            }
            int count;
            return count;
        }
    public:
        std::vector<int> pass_count;
        std::vector<int> fail_count;

    Counter(int test_size) : size(test_size) {}

    bool is_trivial_and_count_dfs(ThresholdTest& test) {
      int total_counts = 1 << test.get_size(); // 2^size
      bool is_trivial = false;
      if (test.is_trivial_pass()) {
        this->passes += total_counts;
        is_trivial = true;
      }
      if (test.is_trivial_fail()) {
        this->fails += total_counts;
        is_trivial = true;
      }
      this->pass_count.push_back(this->passes);
      this->fail_count.push_back(this->fails);
      return is_trivial;
    }

    int get_passes() { return passes; }
    int get_fails() { return fails; }
  
};     

//int ThresholdTest::id_counter = 0;

#endif // NEURON_COMPILER_H_
