#pragma once

#include <SDL2/SDL.h>
#include "Window.h"
#include "Beatmap.h"
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <unordered_map>

class Game {
public:
    bool init();
    void run();
    void shutdown();
    float getSongTimeMs();
    void handleInput(SDL_Keycode key, float songTime);

    void updateJudgmentTexture();
    void updateComboTexture();
    void updatePointsTexture();
    void renderLoadingScreen();

private:
    bool m_loading=true;
    Window m_window;
    bool m_running=false;
    SDL_Event m_event;
    Uint64 m_lastTick = 0l;
    Beatmap m_beatmap;
    Mix_Music *m_music=nullptr;
    std::string m_judgmentText="";
    SDL_Color m_judgementColor={255,255,255,255};
    float m_judgmentTimer=0.0f;
    TTF_Font *m_font=nullptr;
    int m_combo=0;
    int m_points=0;
    std::unordered_map<char, SDL_Texture*> m_letterTextures;

    SDL_Texture* m_judgmentTexture = nullptr;
    SDL_Texture* m_comboTexture = nullptr;
    SDL_Texture* m_scoreTexture = nullptr;

    int m_judgmentW, m_judgmentH;
    int m_comboW, m_comboH;
    int m_scoreW, m_scoreH;
    bool m_changed = false;

    int m_minColor=0;
    int m_maxColor=255;
};