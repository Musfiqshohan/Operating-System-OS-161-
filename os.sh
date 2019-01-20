cd ~/asst2/src/
./configure
cd kern/conf/
./config ASST2
cd ../compile/ASST2
bmake depend
bmake
bmake install
cd ~/asst2/src/
bmake depend
bmake
bmake install
cd ~/os161/root
sys161 kernel-ASST2 "p testbin/asst2"
