

cd ../build

adb push memory_test /data/local/tmp
adb shell "chmod 777 /data/local/tmp/memory_test"
adb shell "su -c /data/local/tmp/memory_test %1"

