alias b := build
alias r := run

default: build

build preset="debug":
  cmake --build --preset {{preset}} --parallel

run preset="debug": build
  ./build/{{preset}}/datagen

[confirm("Are you sure you want to delete the build directory ?")]
clean:
  rm -rf ./build

configure preset="debug":
  cmake --preset {{preset}}
