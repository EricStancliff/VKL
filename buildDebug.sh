CPP20COMPILER=clang++

mkdir build
mkdir buildInstall
cd build
cmake -DCMAKE_INSTALL_PREFIX=../buildInstall -DCMAKE_TOOLCHAIN_FILE=$VC_PKG_TOOLCHAIN  -DVCPKG_TOOLCHAIN_FILE=$VC_PKG_TOOLCHAIN -DCMAKE_CXX_COMPILER=$CPP20COMPILER ../


cmake --build  . --target all
#cmake --build  . --target install
#cd ../buildInstall
#./bin/program
