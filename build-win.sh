[ -f build-win/prod/scores.db ] && cp build-win/prod/scores.db scores.db
[ -f build-win/prod/settings.ini ] && cp build-win/prod/settings.ini settings.ini
rm -rf build-win
mkdir build-win
cp -r beatmaps build-win/beatmaps
cp -r skins build-win/skins
[ -f scores.db ] && cp scores.db build-win/scores.db
[-f settings.ini ] && cp settings.ini build-win/settings.ini
cd build-win
cmake .. -G "MinGW Makefiles"
mingw32-make
mkdir prod
cp Lonkstalk.exe prod/VLBeats.exe
cp -r skins prod/skins
cp -r beatmaps prod/beatmaps
ldd Lonkstalk.exe | grep '/mingw64/bin/' | awk '{print $3}' | while read -r dll; do
    cp -u "$dll" prod/
done
cp LKPacker.exe prod/LKPacker.exe
ldd LKPacker.exe | grep '/mingw64/bin/' | awk '{print $3}' | while read -r dll; do
    cp -u "$dll" prod/
done
[ -f scores.db ] && cp scores.db build-win/prod/scores.db
[ -f settings.ini ] && cp settings.ini build-win/prod/settings.ini
