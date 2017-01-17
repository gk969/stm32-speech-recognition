cd D:\STM32\stm32-speech-recognition
"C:\Program Files (x86)\SEGGER\JLinkARM_V434\JFlashARM.exe" -openprj"download.jflash" -open"Object\stm32-speech-recognition.hex",0x0 -auto -exit

cd D:\STM32\stm32-speech-recognition\Src
git add -A
git commit -m "Auto"