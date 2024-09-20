sudo dmesg --clear
echo "\n----------Attempt 1----------" 
sudo insmod audio_codec.ko
ls -la /dev/audio_codec
sudo rmmod audio_codec
dmesg

sudo dmesg --clear
echo "\n----------Attempt 2----------"
sudo insmod audio_codec.ko
ls -la /dev/audio_codec
sudo rmmod audio_codec
dmesg 