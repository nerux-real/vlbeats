#pragma once

#include <string>
#include <vector>
#include "Note.h"

class Beatmap {
public:
    std::string name;
    std::string author;
    std::string song_path;
    std::string song_original_name;
    int bpm;
    std::vector<Note> notes;
    void load(const char *folder);
};