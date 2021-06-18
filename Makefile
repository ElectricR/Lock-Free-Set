all:
	cmake -Ddebug:BOOL=0 -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -Bbuild; cd build; make

sane:
	cmake -Ddebug:BOOL=1 -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -Bbuild; cd build; make

run:
	./LockFreeSet

ping:
	./LockFreeSet --gtest_filter=Load.Ping*

install:
	git clone https://github.com/khizmax/libcds; \
	cd libcds; \
	mkdir build; \
	cmake -DCMAKE_BUILD_TYPE=RELEASE -Bbuild; \
	cd build; \
	make -j4

