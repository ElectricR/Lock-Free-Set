#include <gtest/gtest.h>
#include <thread>
#include <ranges>

void addition_routine(int& x) {
    for (auto i : std::views::iota(0, 1000000)) {
        synchronized {
            ++x;
        }
    }
}

TEST(HTMTest, BasicAddition) {
    int x = 0;
    std::thread th1{addition_routine, std::ref(x)};
    std::thread th2{addition_routine, std::ref(x)};
    
    th1.join();
    th2.join();

    std::cout << x << std::endl;

}
