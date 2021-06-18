#include <gtest/gtest.h>
#include "../src/set.h"
#include <cds/init.h>
#include <cds/gc/hp.h>

#include <vector>
#include <thread>
#include <functional>
#include <algorithm>

struct Person {
    std::string name;
    std::string lastname;

    auto operator<=>(const Person& o) const = default;
};

TEST(Smoke, ComplexType) {
    cds::Initialize();
    cds::gc::HP hpGC{3};
    cds::threading::Manager::attachThread();

    std::vector<Person> people;

    for (int i = 0; i != 1000; ++i) {
        std::string name;
        std::string lastname;
        int j_max = rand() % 20;
        for (int j = 0; j != j_max; ++j) {
            name += std::string(1, 'a' + rand() % 25);
        }
        j_max = rand() % 20;
        for (int j = 0; j != j_max; ++j) {
            lastname += std::string(1, 'a' + rand() % 25);
        }
        people.push_back({std::move(name), std::move(lastname)});
    }
    
    std::sort(people.begin(), people.end());
    auto last = std::unique(people.begin(), people.end());
    people.erase(last, people.end());

    Set<Person> set;
    for (auto &person : people) {
        EXPECT_TRUE(set.add(person));
        EXPECT_TRUE(set.contains(person));
        EXPECT_TRUE(set.remove(person));
        EXPECT_FALSE(set.contains(person));
    }
    ASSERT_TRUE(set.isEmpty());


    cds::threading::Manager::detachThread();
    cds::Terminate();
}

TEST(Smoke, OneThreadNonRemoving) {
    cds::Initialize();
    cds::gc::HP hpGC{3};
    cds::threading::Manager::attachThread();

    Set<int> set = Set<int>();
    std::vector<int> vec;
    std::vector<bool> used;
    for (int i = 1; i != 1001; ++i) {
        vec.push_back(i);
        used.push_back(false);
    }
    for (int i = 0; i != 10000; ++i) {
        int index = rand() % 1000;
        if (used[index]) {
            ASSERT_TRUE(set.contains(vec[index]));
        }
        else {
            int curr = vec[index];
            used[index] = true;
            set.add(curr);
            ASSERT_TRUE(set.contains(curr));
        }
    }
    cds::threading::Manager::detachThread();
    cds::Terminate();
}

TEST(Smoke, OneThreadWithRemoves) {
    cds::Initialize();
    cds::gc::HP hpGC{3};
    cds::threading::Manager::attachThread();
    Set<int> set;
    std::vector<int> ints;
    for (int i = 0; i != 4000; ++i) {
        ints.emplace_back(rand() % 100000);
    }
    for (int i = 0; i != 4000; ++i) {
        set.add(ints[i]);
        ASSERT_TRUE(set.contains(ints[i]));
    }
    for (int i = 0; i != 4000; ++i) {
        set.remove(ints[i]);
        ASSERT_FALSE(set.contains(ints[i]));
    }
    ASSERT_TRUE(set.isEmpty());


    cds::threading::Manager::detachThread();
    cds::Terminate();
}

void non_removing_test_routine(Set<size_t>& set, unsigned id) {
    cds::threading::Manager::attachThread();
    for (size_t i = id * 4000; i != id * 4000 + 4000; ++i) {
        EXPECT_TRUE(set.add(i));
        EXPECT_TRUE(set.contains(i));
    }
    cds::threading::Manager::detachThread();
}

TEST(Smoke, NonRemoving) {
    cds::Initialize();
    cds::gc::HP hpGC{3};
    cds::threading::Manager::attachThread();
    Set<size_t> set;
    std::vector<std::thread> threads;
    for (unsigned i = 0; i != std::thread::hardware_concurrency(); ++i) {
        threads.emplace_back(non_removing_test_routine, std::ref(set), i);
    }

    for (unsigned i = 0; i != std::thread::hardware_concurrency(); ++i) {
        threads[i].join();
    }

    for (size_t i = 0; i != std::thread::hardware_concurrency() * 4000; ++i) {
        EXPECT_TRUE(set.contains(i));
    }

    cds::threading::Manager::detachThread();
    cds::Terminate();
}

void ping_pong_test_routine(Set<unsigned>& set1, Set<unsigned>& set2, unsigned id, size_t load) {
    cds::threading::Manager::attachThread();
    for (size_t i = 0; i != load; ++i) {
        EXPECT_TRUE(set1.add(id));
        EXPECT_TRUE(set1.contains(id));
        EXPECT_TRUE(set1.remove(id));
        EXPECT_FALSE(set1.contains(id));

        EXPECT_TRUE(set2.add(id));
        EXPECT_TRUE(set2.contains(id));
        EXPECT_TRUE(set2.remove(id));
        EXPECT_FALSE(set2.contains(id));
    }
    cds::threading::Manager::detachThread();
}

TEST(Smoke, PingPong) {
    cds::Initialize();
    cds::gc::HP hpGC{3};

    Set<unsigned> set1;
    Set<unsigned> set2;

    std::vector<std::thread> threads;

    for (unsigned i = 0; i != std::thread::hardware_concurrency(); ++i) {
        threads.emplace_back(ping_pong_test_routine, std::ref(set1), std::ref(set2), i, 100000);
    }
    for (unsigned i = 0; i != std::thread::hardware_concurrency(); ++i) {
        threads[i].join();
    }

    EXPECT_TRUE(set1.isEmpty());
    EXPECT_TRUE(set2.isEmpty());
    cds::Terminate();
}

TEST(Load, PingPong) {
    cds::Initialize();
    cds::gc::HP hpGC{3};

    Set<unsigned> set1;
    Set<unsigned> set2;

    std::vector<std::thread> threads;

    for (unsigned i = 0; i != std::thread::hardware_concurrency(); ++i) {
        threads.emplace_back(ping_pong_test_routine, std::ref(set1), std::ref(set2), i, 100000000);
    }
    for (unsigned i = 0; i != std::thread::hardware_concurrency(); ++i) {
        threads[i].join();
    }

    EXPECT_TRUE(set1.isEmpty());
    EXPECT_TRUE(set2.isEmpty());
    cds::Terminate();
}

void random_test_routine(Set<int>& set, size_t load) {
    cds::threading::Manager::attachThread();
    for (size_t i = 0; i != load; ++i) {
        int key = rand() % 1000;
        set.add(key);
        set.remove(key);
    }
    cds::threading::Manager::detachThread();
}

TEST(Smoke, Random) {
    cds::Initialize();
    cds::gc::HP hpGC{3};

    Set<int> set;

    std::vector<std::thread> threads;

    for (unsigned i = 0; i != std::thread::hardware_concurrency(); ++i) {
        threads.emplace_back(random_test_routine, std::ref(set), 100000);
    }
    for (unsigned i = 0; i != std::thread::hardware_concurrency(); ++i) {
        threads[i].join();
    }

    ASSERT_TRUE(set.isEmpty());
    cds::Terminate();
}

TEST(Load, Random) {
    cds::Initialize();
    cds::gc::HP hpGC{3};

    Set<int> set;

    std::vector<std::thread> threads;

    for (unsigned i = 0; i != std::thread::hardware_concurrency(); ++i) {
        threads.emplace_back(random_test_routine, std::ref(set), 100000000);
    }
    for (unsigned i = 0; i != std::thread::hardware_concurrency(); ++i) {
        threads[i].join();
    }

    ASSERT_TRUE(set.isEmpty());
    cds::Terminate();
}

static std::atomic<unsigned> trailing_finish_count = 0;

void trailing_test_routine(Set<int>& set, size_t load, unsigned id) {
    cds::threading::Manager::attachThread();
    for (size_t i = 0; i != load; ++i) {
        int key = 100 * id + rand() % 100;
        EXPECT_TRUE(set.add(key));
        EXPECT_TRUE(set.contains(key));
        EXPECT_TRUE(set.remove(key));
        EXPECT_FALSE(set.contains(key));
    }
    ++trailing_finish_count;
    cds::threading::Manager::detachThread();
}

TEST(Smoke, Trailing) {
    cds::Initialize();
    cds::gc::HP hpGC{3};

    Set<int> set;

    cds::threading::Manager::attachThread();

    set.add(10001);

    std::vector<std::thread> threads;

    for (unsigned i = 0; i != 98; ++i) {
        threads.emplace_back(trailing_test_routine, std::ref(set), 10000, i);
    }

    while (trailing_finish_count != 98) {
        EXPECT_TRUE(set.contains(10001));
    }

    for (unsigned i = 0; i != 98; ++i) {
        threads[i].join();
    }

    cds::threading::Manager::detachThread();
    cds::Terminate();
}

TEST(Load, Trailing) {
    cds::Initialize();
    cds::gc::HP hpGC{3};

    Set<int> set;

    cds::threading::Manager::attachThread();

    set.add(10001);

    std::vector<std::thread> threads;

    for (unsigned i = 0; i != 98; ++i) {
        threads.emplace_back(trailing_test_routine, std::ref(set), 4000000, i);
    }

    while (trailing_finish_count != 98) {
        EXPECT_TRUE(set.contains(10001));
    }

    for (unsigned i = 0; i != 98; ++i) {
        threads[i].join();
    }

    cds::threading::Manager::detachThread();
    cds::Terminate();
}
