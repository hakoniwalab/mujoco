export DYLD_LIBRARY_PATH=`pwd`/../lib:$DYLD_LIBRARY_PATH
export DYLD_LIBRARY_PATH=`pwd`/lib:$DYLD_LIBRARY_PATH

./cmake-build/examples/forklift/forklift_sim
