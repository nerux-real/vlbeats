cd ..
sudo rm -rf build
mkdir build
cp -r beatmaps build/beatmaps
cp -r skins build/skins
cd build
cmake ..
make 
./Lonkstalk
