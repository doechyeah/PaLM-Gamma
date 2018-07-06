move /y "\\RASPBERRYPI\share\initial.bmp" "C:\Users\Daniel Song\Desktop\PaLM\Program\images"
:backup
move /y "\\RASPBERRYPI\share\update.bmp" "C:\Users\Daniel Song\Desktop\PaLM\Program\images"
timeout 5
move /y "C:\Users\Daniel Song\Desktop\PaLM\Program\images\update.bmp" "archive"
ren "C:\Users\Daniel Song\Desktop\PaLM\Program\archive\update.bmp" "Update (%date:/=-%) (%time::=-%).bmp"
timeout 3
goto backup