cmake --build build
echo "compiling shaders"
cd shaders/
compile.bat
cd ..
bin\SPRITE_SEER.exe