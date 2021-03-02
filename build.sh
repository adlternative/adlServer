cd build/
cmake CMakeLists.txt
make
cd ..

if test -n "$1" &&
    test "$1" == "--run"
then
  exec build/adlServer
fi