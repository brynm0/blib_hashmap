@echo off

cd preprocessed
cl ./hashmap_preprocessed.cpp -Od -Ob0  -DH_DEBUG=0
@REM -Zi -arch:AVX2 
cd ..