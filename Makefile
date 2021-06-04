flags:
	clang++ -std=c++20 test_flag_macros.cpp -lpthread -lgtest -lgtest_main

smoke:
	clang++ -I./libcds -L./libcds/build/bin -std=c++20 tests/smoke_test.cpp -lcds -lpthread -lgtest -lgtest_main -o smoke_test -g -Wl,-rpath="./libcds/build/bin" -Wall -Wextra

smoke_sane:
	clang++ -I./libcds -L./libcds/build/bin -std=c++20 tests/smoke_test.cpp -lcds -lpthread -lgtest -lgtest_main -o smoke_test -g -Wl,-rpath="./libcds/build/bin" -Wall -Wextra -fsanitize=thread

smoke3:
	clang++ -I./libcds -L./libcds/build/bin -std=c++20 tests/smoke_test.cpp -lcds -lpthread -lgtest -lgtest_main -o smoke_test -Wl,-rpath="./libcds/build/bin" -Wall -Wextra -O3

install:
	git clone https://github.com/khizmax/libcds; \
	cd libcds; \
	mkdir build; \
	cmake -DCMAKE_BUILD_TYPE=RELEASE -Bbuild; \
	cd build; \
	make -j4

