@echo off

cd preprocessed
cl ./hashmap_preprocessed.cpp -arch:AVX2 -O2 -Ob1 -Zi -DH_DEBUG=0
cd ..