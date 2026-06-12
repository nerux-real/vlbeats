#pragma once
#include <SDL2/SDL.h>

enum class NoteState {
    Waiting,
    Active,
    Hit,
    Missed
};

struct Note {
    int timestampMs;
    char key;
    int gridCol;
    int gridRow;
    NoteState state = NoteState::Waiting;
    float fadeTimer=200.0f;

    Uint8 r, g, b;
    bool hasColor=false;
};