# 1，Sample Plug manual
## 1.1 Features
The plug-in realizes the capture of downstream data and the injection of upstream data, which can be dynamically adjusted through adb shell commands when the program is running.
## 1.2 Steps for usage
step1: adb root \
step2: adb remount \
step3: adb shell setenforce 0 \
step4: adb shell "setprop config.pcm.plug.enable true"
To stop the plug-in function, set this property to false\
step5: dump downlink pcmC0D0p data:adb shell touch /data/dump_pcm_c0d0.pcm \
step6: dump downlink pcmC1D0p data:adb shell touch /data/dump_pcm_c1d0.pcm \
step7: After that, the sounds of operations will be stored in their respective files.  \
step8：Taking it out, select the appropriate player to play \
step9: inject uplink pcmC0D0c data:adb push xxx_tts.pcm /data/semidrive_tts.pcm \
xxx_tts.pcm is the data you injected. After the execution is completed, you will get the voiceprint information in the pcm file using any APP recording. This case provides a version of the demo "semidrive" pcm file.