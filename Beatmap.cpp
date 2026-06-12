#include "Beatmap.h"
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>

void Beatmap::load(const char *folder){
    std::string path=std::string(folder)+"/metadata.txt";
    std::ifstream fin(path);
    if(!fin.is_open()){
        std::cerr<<"Could not open "<<path<<std::endl;
        return;
    }

    std::string line;
    while(getline(fin, line)){
        size_t colon = line.find(':');
        if(colon==std::string::npos) continue;
        std::string key = line.substr(0, colon);
        std::string value = line.substr(colon+2);

        if(key=="name") name=value;
        else if(key=="author") author=value;
        else if(key=="song") song_path=std::string(folder)+"/"+value;
        else if(key=="song_original_name") song_original_name=value;
        else if(key=="bpm") bpm=std::stoi(value);
    }
    fin.close();

    path=std::string(folder)+"/map.txt";
    fin.open(path);
    if(!fin.is_open()){
        std::cerr<<"Could not open "<<path<<std::endl;
        return;
    }
    while(getline(fin, line)){
        if(line.empty()) continue;
        std::istringstream iss(line);
        Note note;
        iss>>note.timestampMs>>note.key>>note.gridCol>>note.gridRow;
        notes.push_back(note);
    }
    std::cout << "Loaded: " << name << " by " << author << " BPM:" << bpm << "\n";
    std::cout << "Notes: " << notes.size() << "\n";
}