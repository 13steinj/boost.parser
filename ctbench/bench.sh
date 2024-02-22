# requires a decent version of git, cmake, ninja, hyperfine, and I chose mold (since I'm caring about compile times here)
# also requires libc++ and libc++abi, preferrably of the same clang version

git clone https://github.com/13steinj/boost.parser.git || true
git clone -j10 --recurse-submodules https://github.com/boostorg/boost.git || true
git -C boost checkout boost-1.84.0 --recurse-submodules || true

pushd boost
./bootstrap.sh --prefix=../boost-prefix
# looks about right. Can't get the headers by themselves for whatever reason
./b2 install headers
popd

cd boost.parser
mkdir -p ctbench

set -efo pipefail

export LDFLAGS="-fuse-ld=mold"

function bench() {
    compiler=$1
    extra_tag="$3"
    export CXXFLAGS="-Wno-error $2"
    export CXX=$compiler
    for flag in TRUE FALSE; do
        mkdir build-${compiler}-${flag}${extra_tag}
        cmake -S . -B build-${compiler}-${flag}${extra_tag} -DBOOST_ROOT=../boost-prefix -DBUILD_WITHOUT_HANA=${flag} -G "Ninja Multi-Config"
        pushd build-${compiler}-${flag}${extra_tag}
        # You can't say that I am not thorough ;)
        for config in Debug Release; do
            cmake --build . --config ${config} -j
            lscpu >> ../ctbench/hyperfine-${compiler}-${flag}-${config}${extra_tag}.log
            $compiler --version >> ../ctbench/hyperfine-${compiler}-${flag}-${config}${extra_tag}.log
            hyperfine --warmup 3 \
                --prepare "cmake --build . --config ${config} -t clean" \
                "cmake --build . --config ${config} -j" >> ../ctbench/hyperfine-${compiler}-${flag}-${config}${extra_tag}.log
        done
        popd
    done
}

bench g++-12
bench clang++-15 "-ftime-trace"
bench clang++-15 "-stdlib=libc++ -ftime-trace" "-libcpp"
