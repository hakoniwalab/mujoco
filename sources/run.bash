export DYLD_LIBRARY_PATH=`pwd`/../lib:$DYLD_LIBRARY_PATH
export DYLD_LIBRARY_PATH=`pwd`/lib:$DYLD_LIBRARY_PATH

./cmake-build/examples/mujoco_capi_call/main
