PRESET ?= debug

.PHONY: build test scaffold exercise run clean

build:
	cmake --preset $(PRESET)
	cmake --build --preset $(PRESET)

test: build
	ctest --preset $(PRESET)

scaffold exercise: build
	ctest --preset $(PRESET) -L $@

run: build
	./build/$(PRESET)/hft_lab --demo

clean:
	cmake --build --preset $(PRESET) --target clean
