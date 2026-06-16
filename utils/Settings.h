#pragma once
#include <string>

struct Settings {
    int master_volume=50;
    int music_volume=100;
    int hitsound_volume=50;
    int ui_volume=50;
    int offset=0;
    int resWidth=1920;
    int resHeight=1080;
    std::string videoMode="windowed";
    int fpsLock=0;
    int bgAlpha=64;
    bool debug=true;
    std::string selectedSkin="default";

    void load(const std::string &path);
    void save(const std::string &path);
};