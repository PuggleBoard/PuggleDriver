sudo cp -f ./puggle_dma.ko /lib/modules/3.8.13xenomai-bone53/build/drivers/dma
sudo depmod -a 
sudo insmod /lib/modules/3.8.13xenomai-bone53/build/drivers/dma/puggle_dma.ko
sleep 5
sudo rmmod /lib/modules/3.8.13xenomai-bone53/build/drivers/dma/puggle_dma.ko
dmesg | tail
