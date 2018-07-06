:backup
move /y "\\RASPBERRYPI\share\initial.bmp" "C:\Users\mikey\Desktop\Test"
move /y "\\RASPBERRYPI\share\update.bmp" "C:\Users\mikey\Desktop\Test"
timeout 3
move /y "C:\Users\mikey\Desktop\Test\update.bmp" "C:\Users\mikey\Desktop\Junk"
ren "C:\Users\mikey\Desktop\Junk\update.bmp" "Update (%date:/=-%) (%time::=-%).bmp"
timeout 3
goto backup