#include <iostream>
#include "neuron_compiler.h"

void dfs(ThresholdTest& test, Counter& counter) {
  if ( counter.is_trivial_and_count_dfs(test) )
    return;

  auto test1 = test.set_last(1);
  dfs(*test1,counter);

  auto test0 = test.set_last(0);
  dfs(*test0,counter);
  return;
}

