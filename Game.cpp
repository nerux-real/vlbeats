#include "Game.h"
#include <SDL2/SDL_mixer.h>
#include "constants.h"
#include <cmath>
#include <cctype>
#include <SDL2/SDL_ttf.h>
#include <random>

void Game::updateJudgmentTexture() {
    if (m_judgmentTexture)
        SDL_DestroyTexture(m_judgmentTexture);

    SDL_Surface* s = TTF_RenderText_Solid(m_font, m_judgmentText.c_str(), m_judgementColor);
    m_judgmentTexture = SDL_CreateTextureFromSurface(m_window.getRenderer(), s);
    SDL_QueryTexture(m_judgmentTexture, nullptr, nullptr, &m_judgmentW, &m_judgmentH);
    SDL_FreeSurface(s);
}

void Game::updateComboTexture() {
    if (m_comboTexture)
        SDL_DestroyTexture(m_comboTexture);

    std::string comboStr = "Combo: " + std::to_string(m_combo) + "x";
    SDL_Surface* s = TTF_RenderText_Solid(m_font, comboStr.c_str(), {255,255,255,255});
    m_comboTexture = SDL_CreateTextureFromSurface(m_window.getRenderer(), s);
    SDL_QueryTexture(m_comboTexture, nullptr, nullptr, &m_comboW, &m_comboH);
    SDL_FreeSurface(s);
}

void Game::updatePointsTexture() {
    if (m_scoreTexture)
        SDL_DestroyTexture(m_scoreTexture);

    std::string scoreStr = "Score: " + std::to_string(m_points);
    SDL_Surface* s = TTF_RenderText_Solid(m_font, scoreStr.c_str(), {255,255,255,255});
    m_scoreTexture = SDL_CreateTextureFromSurface(m_window.getRenderer(), s);
    SDL_QueryTexture(m_scoreTexture, nullptr, nullptr, &m_scoreW, &m_scoreH);
    SDL_FreeSurface(s);
}

void Game::renderLoadingScreen() {
    SDL_Renderer* renderer = m_window.getRenderer();
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Surface* s = TTF_RenderText_Solid(m_font, "Loading...", {255,255,255,255});
    SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);

    int w, h;
    SDL_QueryTexture(t, nullptr, nullptr, &w, &h);
    SDL_Rect dst = {960 - w/2, 540 - h/2, w, h};

    SDL_RenderCopy(renderer, t, nullptr, &dst);
    SDL_RenderPresent(renderer);

    SDL_FreeSurface(s);
    SDL_DestroyTexture(t);
}


bool Game::init(){
    m_loading=true;
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)!=0){
        std::cerr<<SDL_GetError()<<std::endl;
        return false;
    }
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048)!=0){
        std::cerr<<Mix_GetError()<<std::endl;
        return false;
    }
    if(TTF_Init()!=0){
        std::cerr<<TTF_GetError()<<std::endl;
        return false;
    }
    m_font=TTF_OpenFont("../font.ttf", 48);
    if(!m_font){
        std::cerr<<TTF_GetError()<<std::endl;
        return false;
    }
    if(!m_window.init("Lonkstalk", 1920, 1080)) return false;
    SDL_Renderer *renderer=m_window.getRenderer();
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    m_beatmap.load("../beatmaps/firstbeatmap");
    for(char c='A'; c<='Z'; c++){
        std::string s(1,c);
        SDL_Surface *surface = TTF_RenderText_Solid(m_font, s.c_str(), {0,0,0,255});
        m_letterTextures[c]=SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }
    m_music=Mix_LoadMUS(m_beatmap.song_path.c_str());
    if(!m_music){
        std::cerr<<Mix_GetError()<<std::endl;
        return false;
    }
    Mix_PlayMusic(m_music, 1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 2);

    m_changed = true;
    updateComboTexture();
    updatePointsTexture();
    m_changed = false;

    m_lastTick=SDL_GetPerformanceCounter();
    m_loading=false;
    m_running=true;
    return true;
}

void Game::run(){
    std::random_device rd;
    std::mt19937 gen(rd());
    SDL_Renderer *renderer=m_window.getRenderer();
    while(m_running){
        Uint64 now = SDL_GetPerformanceCounter();
        float deltaMs = (float)(now-m_lastTick)/SDL_GetPerformanceFrequency()*1000.0f;
        m_lastTick=now;
        while(SDL_PollEvent(&m_event)){
            if(m_event.type==SDL_QUIT) m_running=false;
            else if(m_event.type==SDL_KEYDOWN) handleInput(m_event.key.keysym.sym, getSongTimeMs());
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        float songTime = getSongTimeMs();
        m_judgmentTimer -= deltaMs;
        if(m_judgmentTimer < 0) m_judgmentTimer = 0;

        for(auto& note : m_beatmap.notes){
            float timeLeft = note.timestampMs - songTime;
            float timeSinceAppear = APPROACH_TIME_MS - timeLeft;
            Uint8 alpha=255;
            if(timeSinceAppear<200.0f) alpha=(Uint8)((timeSinceAppear/200.0f)*255);
            float progress = timeLeft / APPROACH_TIME_MS;
            if(progress<0.0f) progress=0.0f;
            int margin = (int)(progress*APPROACH_SIZE);

            if(note.state == NoteState::Hit || note.state == NoteState::Missed){
                note.fadeTimer -= deltaMs;
                if(note.fadeTimer <= 0) continue;
                Uint8 fadeAlpha = (Uint8)((note.fadeTimer / 200.0f) * 255);
                int x = note.gridCol * CELL_SIZE;
                int y = note.gridRow * CELL_SIZE;
                SDL_Rect rect = {x, y, CELL_SIZE, CELL_SIZE};
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, fadeAlpha);
                SDL_RenderFillRect(renderer, &rect);
                continue;
            }

            if(songTime > note.timestampMs + MISS_WINDOW_MS){
                m_combo=0;
                m_judgmentText="MISS";
                m_judgementColor={255,0,0,255};
                m_judgmentTimer=800.0f;
                note.state = NoteState::Missed;
                note.fadeTimer = 200.0f;
                m_changed = true;
                continue;
            }

            std::uniform_int_distribution<> dis(180, 255);
            if (!note.hasColor) {
                note.r = dis(gen);
                note.g = dis(gen);
                note.b = dis(gen);
                note.hasColor = true;
            }
            SDL_SetRenderDrawColor(renderer, note.r, note.g, note.b, alpha);

            if(songTime >= note.timestampMs - APPROACH_TIME_MS){
                int x = note.gridCol * CELL_SIZE;
                int y = note.gridRow * CELL_SIZE;
                SDL_Rect outerRect = {x-margin, y-margin, CELL_SIZE+margin*2, CELL_SIZE+margin*2};
                //SDL_SetRenderDrawColor(renderer, 255, 255, 255, alpha);
                SDL_RenderDrawRect(renderer, &outerRect);
                SDL_Rect rect = {x, y, CELL_SIZE, CELL_SIZE};
                //SDL_SetRenderDrawColor(renderer, 255, 255, 255, alpha);
                SDL_RenderFillRect(renderer, &rect);

                SDL_Texture* texture = m_letterTextures[note.key];
                int w, h;
                SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);
                SDL_Rect dst = {x + CELL_SIZE/2 - w/2, y + CELL_SIZE/2 - h/2, w, h};
                SDL_RenderCopy(renderer, texture, nullptr, &dst);
            }
        }

        if (m_changed) {
            updateJudgmentTexture();
            updateComboTexture();
            updatePointsTexture();
            m_changed = false;
        }

        if (m_judgmentTimer > 0 && m_judgmentTexture) {
            SDL_Rect dst = {960 - m_judgmentW/2, 540 - m_judgmentH/2, m_judgmentW, m_judgmentH};
            SDL_RenderCopy(renderer, m_judgmentTexture, nullptr, &dst);
        }

        SDL_Rect comboDst = {20, 1080 - m_comboH - 20, m_comboW, m_comboH};
        SDL_RenderCopy(renderer, m_comboTexture, nullptr, &comboDst);

        SDL_Rect scoreDst = {1920 - m_scoreW - 20, 20, m_scoreW, m_scoreH};
        SDL_RenderCopy(renderer, m_scoreTexture, nullptr, &scoreDst);

        SDL_RenderPresent(renderer);
    }
}

void Game::shutdown(){
    for(auto& pair:m_letterTextures) SDL_DestroyTexture(pair.second);
    SDL_DestroyTexture(m_judgmentTexture);
    SDL_DestroyTexture(m_comboTexture);
    SDL_DestroyTexture(m_scoreTexture);
    TTF_CloseFont(m_font);
    TTF_Quit();
    Mix_FreeMusic(m_music);
    Mix_CloseAudio();
    m_window.destroy();
    SDL_Quit();
    m_running=false;
}

float Game::getSongTimeMs(){
    return (float)Mix_GetMusicPosition(m_music)*1000.0f;
}

void Game::handleInput(SDL_Keycode key, float songTime){
    auto* bestNote = static_cast<decltype(&m_beatmap.notes[0])>(nullptr);
    float bestDiff = 1e9f;

    for(auto& note : m_beatmap.notes){
        if(note.state == NoteState::Hit || note.state == NoteState::Missed) continue;
        if((char)key != std::tolower(note.key)) continue;
        if(songTime < note.timestampMs - APPROACH_TIME_MS) continue;
        float diff = std::abs(songTime - note.timestampMs);
        if(diff < bestDiff){
            bestDiff = diff;
            bestNote = &note;
        }
    }

    if(!bestNote) return;

    if(bestDiff <= EXCELLENT_WINDOW_MS){
        m_judgmentText="EXCELLENT";
        m_judgementColor={255,215,0,255};
        m_judgmentTimer=800.0f;
        m_combo++;
        m_points+=300*m_combo;
    } else if(bestDiff <= GOOD_WINDOW_MS){
        m_judgmentText="GOOD";
        m_judgementColor={0,128,0,255};
        m_judgmentTimer=800.0f;
        m_combo++;
        m_points+=100*m_combo;
    } else {
        m_judgmentText="MISS";
        m_judgementColor={255,0,0,255};
        m_judgmentTimer=800.0f;
        m_combo=0;
    }
    bestNote->state = NoteState::Hit;
    bestNote->fadeTimer = 200.0f;
    m_changed = true;
}