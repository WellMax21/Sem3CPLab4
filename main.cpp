#include <iostream>
#include <map>
#include <functional>

#include "pool_allocator.h"
#include "forward_list.h"

static unsigned long long factorial(unsigned n) {
    return n <= 1 ? 1ULL : n * factorial(n - 1);
}

int main() {
    std::map<int, unsigned long long> m1;
    for (int i = 0; i < 10; ++i) m1[i] = factorial(i);

    using MapAlloc = PoolAllocator<std::pair<const int, unsigned long long>, 10>;
    std::map<int, unsigned long long, std::less<int>, MapAlloc> m2;
    for (int i = 0; i < 10; ++i) m2[i] = factorial(i);
    
    ForwardList<int> l1;
    for (int i = 0; i < 10; ++i) l1.push_back(i);

    ForwardList<int, PoolAllocator<int, 10>> l2;
    for (int i = 0; i < 10; ++i) l2.push_back(i);

    std::cout << "=== map со стандартным аллокатором ===\n";
    for (const auto& kv : m1) std::cout << kv.first << " " << kv.second << "\n";

    std::cout << "\n=== map с кастомным аллокатором ===\n";
    for (const auto& kv : m2) std::cout << kv.first << " " << kv.second << "\n";

    std::cout << "\n=== ForwardList со стандартным аллокатором ===\n";
    for (int v : l1) std::cout << v << "\n";

    std::cout << "\n=== ForwardList с кастомным аллокатором ===\n";
    for (int v : l2) std::cout << v << "\n";

    return 0;
}
