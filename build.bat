@echo off

mkdir .build 2> NUL
pushd .build
cmake .. -DQT5_ROOT=E:\sources\third_party\Qt\5.15.1\msvc2019_64
popd
