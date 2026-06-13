#include "Game.h"
#include <SDL2/SDL_mixer.h>
#include "constants.h"
#include <cmath>
#include <cctype>
#include <SDL2/SDL_ttf.h>
#include <random>
#include <SDL2/SDL_image.h>

void Game::updateJudgmentTexture() {
    if (m_judgmentTexture)
        SDL_DestroyTexture(m_judgmentTexture);

    SDL_Surface* s = TTF_RenderText_Solid(m_skins.getActive().l_fontGame, m_judgmentText.c_str(), m_judgementColor);
    m_judgmentTexture = SDL_CreateTextureFromSurface(m_window.getRenderer(), s);
    SDL_QueryTexture(m_judgmentTexture, nullptr, nullptr, &m_judgmentW, &m_judgmentH);
    SDL_FreeSurface(s);
}

void Game::updateComboTexture() {
    if (m_comboTexture)
        SDL_DestroyTexture(m_comboTexture);

    std::string comboStr = "Combo: " + std::to_string(m_combo) + "x";
    SDL_Surface* s = TTF_RenderText_Solid(m_skins.getActive().l_fontGame, comboStr.c_str(), {255,255,255,255});
    m_comboTexture = SDL_CreateTextureFromSurface(m_window.getRenderer(), s);
    SDL_QueryTexture(m_comboTexture, nullptr, nullptr, &m_comboW, &m_comboH);
    SDL_FreeSurface(s);
}

void Game::updatePointsTexture() {
    if (m_scoreTexture)
        SDL_DestroyTexture(m_scoreTexture);

    std::string scoreStr = "Score: " + std::to_string((int)m_displayPoints);
    SDL_Surface* s = TTF_RenderText_Solid(m_skins.getActive().l_fontGame, scoreStr.c_str(), {255,255,255,255});
    m_scoreTexture = SDL_CreateTextureFromSurface(m_window.getRenderer(), s);
    SDL_QueryTexture(m_scoreTexture, nullptr, nullptr, &m_scoreW, &m_scoreH);
    SDL_FreeSurface(s);
}

void Game::renderLoadingScreen() {
    SDL_Renderer* renderer = m_window.getRenderer();
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Surface* s = TTF_RenderText_Solid(m_skins.getActive().l_fontUI, "Loading...", {255,255,255,255});
    SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);

    int w, h;
    SDL_QueryTexture(t, nullptr, nullptr, &w, &h);
    SDL_Rect dst = {960 - w/2, 540 - h/2, w, h};

    SDL_RenderCopy(renderer, t, nullptr, &dst);
    SDL_RenderPresent(renderer);

    SDL_FreeSurface(s);
    SDL_DestroyTexture(t);
}

float Game::getAccuracy(){
    int judged = m_excellentHits + m_goodHits + m_misses;
    if(judged == 0) return 100.0f;
    return (m_excellentHits * 100.0f + m_goodHits * 50.0f) / (judged * 100.0f) * 100.0f;
}

void Game::updateAccuracyTexture(){
    if(m_accuracyTexture) SDL_DestroyTexture(m_accuracyTexture);
    std::string accStr = std::to_string((int)getAccuracy()) + "%";
    SDL_Surface* s = TTF_RenderText_Solid(m_skins.getActive().m_fontUI, accStr.c_str(), {255,255,255,255});
    m_accuracyTexture = SDL_CreateTextureFromSurface(m_window.getRenderer(), s);
    SDL_QueryTexture(m_accuracyTexture, nullptr, nullptr, &m_accuracyW, &m_accuracyH);
    SDL_FreeSurface(s);
}

void Game::renderHPBar(){
    SDL_Renderer* renderer = m_window.getRenderer();
    int barX = 20;
    int barY = 20;
    int barW = 300;
    int barH = 20;

    SDL_Rect barBg = {barX, barY, barW, barH};
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_RenderFillRect(renderer, &barBg);

    int fillW = (int)(m_hp / m_maxHp * barW);
    Uint8 r = (Uint8)(255 * (1.0f - m_hp / m_maxHp));
    Uint8 g = (Uint8)(255 * (m_hp / m_maxHp));
    SDL_Rect barFill = {barX, barY, fillW, barH};
    SDL_SetRenderDrawColor(renderer, r, g, 50, 255);
    SDL_RenderFillRect(renderer, &barFill);

    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderDrawRect(renderer, &barBg);
}

void Game::renderHitErrorBar(){
    SDL_Renderer* renderer = m_window.getRenderer();
    int barW = 400;
    int barH = 6;
    int barX = m_settings.resWidth/2 - barW/2;
    int barY = m_settings.resHeight - 40;

    SDL_Rect bg = {barX, barY, barW, barH};
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 200);
    SDL_RenderFillRect(renderer, &bg);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 180);
    SDL_RenderDrawLine(renderer, barX + barW/2, barY - 4, barX + barW/2, barY + barH + 4);

    int exW = (int)(EXCELLENT_WINDOW_MS / GOOD_WINDOW_MS * barW/2);
    SDL_Rect exZone = {barX + barW/2 - exW, barY, exW*2, barH};
    SDL_SetRenderDrawColor(renderer, 255, 215, 0, 60);
    SDL_RenderFillRect(renderer, &exZone);

    for(auto& e : m_hitErrors){
        Uint8 alpha = (Uint8)(e.lifetime / 2000.0f * 255);
        float normalized = e.offset / GOOD_WINDOW_MS;
        int tx = barX + barW/2 + (int)(normalized * barW/2);
        tx = std::max(barX, std::min(barX + barW, tx));
        SDL_SetRenderDrawColor(renderer,
            e.excellent ? 255 : 100,
            e.excellent ? 215 : 180,
            e.excellent ? 0   : 255,
            alpha);
        SDL_Rect tick = {tx - 2, barY - 2, 4, barH + 4};
        SDL_RenderFillRect(renderer, &tick);
    }
}

std::string Game::getGrade(){
    float acc = getAccuracy();
    if(acc == 100.0f) return "SS";
    if(acc >= 95.0f) return "S";
    if(acc >= 90.0f) return "A";
    if(acc >= 80.0f) return "B";
    if(acc >= 70.0f) return "C";
    return "F";
}

void Game::renderComboMilestone(){
    SDL_Renderer* renderer = m_window.getRenderer();
    if(m_milestoneTimer > 0){
        float t = m_milestoneTimer / 800.0f;
        Uint8 alpha = (Uint8)(t * 180);
        float scale = 1.0f + (1.0f - t) * 0.5f;

        std::string milestoneStr = std::to_string(m_milestoneCombo) + "x COMBO!";
        SDL_Surface* s = TTF_RenderText_Solid(m_skins.getActive().l_fontGame, milestoneStr.c_str(), {255,215,0,255});
        SDL_Texture* t2 = SDL_CreateTextureFromSurface(renderer, s);
        SDL_SetTextureAlphaMod(t2, alpha);
        SDL_SetTextureBlendMode(t2, SDL_BLENDMODE_BLEND);
        int w, h;
        SDL_QueryTexture(t2, nullptr, nullptr, &w, &h);
        int sw = (int)(w * scale);
        int sh = (int)(h * scale);
        SDL_Rect dst = {m_settings.resWidth/2 - sw/2, m_settings.resHeight/2 - sh/2 - 80, sw, sh};
        SDL_RenderCopy(renderer, t2, nullptr, &dst);
        SDL_FreeSurface(s);
        SDL_DestroyTexture(t2);
    }
}

void Game::renderHpPulseEffect(){\
    SDL_Renderer* renderer = m_window.getRenderer();
    if(m_hp < 30.0f){
        float pulse = (std::sin(m_hpPulseTimer / 200.0f) + 1.0f) / 2.0f;
        int layers = 12;
        for(int i = 0; i < layers; i++){
            float t = (float)i / layers;
            Uint8 alpha = (Uint8)(pulse * 80 * (1.0f - t));
            int inset = i * 8;
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, alpha);
            SDL_Rect frame = {
                inset,
                inset,
                m_settings.resWidth  - inset*2,
                m_settings.resHeight - inset*2
            };
            SDL_RenderDrawRect(renderer, &frame);
        }
    }
}

void Game::renderTimerDisplay(){
    SDL_Renderer* renderer = m_window.getRenderer();
    float timeLeft = (m_mapDuration - m_visualTime) / 1000.0f;
    if(timeLeft < 0) timeLeft = 0;
    int minutes = (int)timeLeft / 60;
    int seconds = (int)timeLeft % 60;
    char timerBuf[16];
    snprintf(timerBuf, sizeof(timerBuf), "%d:%02d", minutes, seconds);

    SDL_Surface* ts = TTF_RenderText_Solid(m_skins.getActive().m_fontUI, timerBuf, {255,255,255,255});
    SDL_Texture* tt = SDL_CreateTextureFromSurface(renderer, ts);
    int tw, th;
    SDL_QueryTexture(tt, nullptr, nullptr, &tw, &th);
    SDL_Rect tdst = {m_settings.resWidth/2 - tw/2, 20, tw, th};
    SDL_RenderCopy(renderer, tt, nullptr, &tdst);
    SDL_FreeSurface(ts);
    SDL_DestroyTexture(tt);
}

bool Game::init(){
    m_settings.load("settings.ini");
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
    if(!m_window.init("Lonkstalk", m_settings.resWidth, m_settings.resHeight, m_settings.fpsLock)) return false;
    m_skins.scanSkins("skins");
    m_skins.loadSkin(0, m_window.getRenderer()); //default pirmo
    m_debug.init(m_skins.getActive().m_fontUI, m_window.getRenderer());
    m_debug.hook();
    m_discordRPC.init("1515437675111387178");
    m_discordRPC.update("In Menus", "Browsing songs");
    m_lastTick=SDL_GetPerformanceCounter();
    m_loading=false;
    m_running=true;
    scanBeatmaps();
    return true;
}

void Game::scanBeatmaps(){
    std::string beatmapDir = fs::current_path().string() + "/beatmaps";
    if(!fs::exists(beatmapDir) || !fs::is_directory(beatmapDir)){
        std::cerr<<"Beatmap directory not found: "<<beatmapDir<<std::endl;
        return;
    }
    for(const auto& entry : fs::directory_iterator(beatmapDir)){
        if(entry.path().extension() != ".lk") continue;
        Beatmap temp;
        temp.loadMetaFromLk(entry.path().string().c_str());
        SongEntry song;
        song.lkPath    = entry.path().string();
        song.name      = temp.name;
        song.author    = temp.author;
        song.bpm       = temp.bpm;
        song.difficulties = temp.difficulties;
        m_songList.push_back(song);
    }
    std::cout<<"Found "<<m_songList.size()<<" beatmaps"<<std::endl;
}

void Game::startGame(const std::string &path, const std::string &difficulty){
    //m_beatmap.load(path.c_str()); legacy
    m_visualTime=0.0f;    
    m_beatmap.loadMetaFromLk(path.c_str());
    m_beatmap.loadFromLk(path.c_str(), difficulty);

    m_mapDuration = 0.0f;
    for(auto& note : m_beatmap.notes){
        float end = note.type == NoteType::Slider ? note.endTimestampMs : note.timestampMs;
        if(end > m_mapDuration) m_mapDuration = end;
    }
    m_mapDuration += 3000.0f;

    m_discordRPC.update(
        m_beatmap.name + " [" + difficulty + "]",
        "Playing"
    );

    if(m_previewPlaying){
        Mix_FadeOutMusic(300);
        m_previewPlaying = false;
    }
    if(m_previewMusic){
        Mix_FreeMusic(m_previewMusic);
        m_previewMusic = nullptr;
    }

    SDL_Renderer *renderer=m_window.getRenderer();
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    for(char c='A'; c<='Z'; c++){
        std::string s(1,c);
        SDL_Surface *surface = TTF_RenderText_Solid(m_skins.getActive().l_fontGame, s.c_str(), {0,0,0,255});
        m_letterTextures[c]=SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }
    if(m_music) Mix_FreeMusic(m_music);
    m_music=Mix_LoadMUS(m_beatmap.song_path.c_str());
    if(!m_music){
        std::cerr<<Mix_GetError()<<std::endl;
        return;
    }
    Mix_PlayMusic(m_music, 1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 100 * m_settings.volume);
    if(!m_beatmap.bg_path.empty()){
        if(m_bgTexture) SDL_DestroyTexture(m_bgTexture);
        SDL_Surface* bgSurface = IMG_Load(m_beatmap.bg_path.c_str());
        m_bgTexture = SDL_CreateTextureFromSurface(m_window.getRenderer(), bgSurface);
        SDL_SetTextureBlendMode(m_bgTexture, SDL_BLENDMODE_BLEND);
        SDL_FreeSurface(bgSurface);
        if(!bgSurface) std::cerr << "IMG_Load failed: " << IMG_GetError() << "\n";
    }

    m_combo=0;
    m_maxCombo=0;
    m_points=0;
    m_displayPoints=0.0f;
    m_excellentHits=0;
    m_goodHits=0;
    m_misses=0;
    m_hpPulseTimer = 0.0f;
    m_totalNotes = m_beatmap.notes.size();
    m_changed = true;
    updateComboTexture();
    updatePointsTexture();
    updateAccuracyTexture();
    m_changed = false;

    m_countdownTimer=1000.0f;
    m_countdownValue=3;
    Mix_PauseMusic();
    m_state=GameState::Countdown;
}

void Game::updateCountdown(float deltaMs){
    m_countdownTimer -= deltaMs;
    if(m_countdownTimer <= 0){
        m_countdownValue--;
        m_countdownTimer = 1000.0f;
        if(m_countdownValue <= 0){
            Mix_ResumeMusic();
            m_visualTime = getSongTimeMs();
            m_state = GameState::Gameplay;
        }
    }
}

void Game::renderCountdown(){
    renderGameplay();

    SDL_Renderer* renderer = m_window.getRenderer();
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
    SDL_Rect overlay = {0, 0, m_settings.resWidth, m_settings.resHeight};
    SDL_RenderFillRect(renderer, &overlay);

    std::string num = std::to_string(m_countdownValue);
    SDL_Surface* s = TTF_RenderText_Solid(m_skins.getActive().l_fontGame, num.c_str(), {255,255,255,255});
    SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
    int w, h;
    SDL_QueryTexture(t, nullptr, nullptr, &w, &h);
    SDL_Rect dst = {960 - w/2, 540 - h/2, w, h};
    SDL_RenderCopy(renderer, t, nullptr, &dst);
    SDL_FreeSurface(s);
    SDL_DestroyTexture(t);

    renderHPBar();
}

void Game::updateSongSelectMenu(float deltaMs){
    m_debug.update(deltaMs);

    if(m_selectedSong != m_lastSelectedSong){
        m_lastSelectedSong = m_selectedSong;
        m_previewTimer = 500.0f;
        m_previewFade = 0.0f;
        m_previewPlayTime = 0.0f;
        if(m_previewPlaying){
            Mix_FadeOutMusic(300);
            m_previewPlaying = false;
        }
    }

    if(m_previewTimer > 0){
        m_previewTimer -= deltaMs;
        if(m_previewTimer <= 0){
            if(m_previewMusic) Mix_FreeMusic(m_previewMusic);
            Beatmap temp;
            temp.loadMetaFromLk(m_songList[m_selectedSong].lkPath.c_str());
            temp.loadSongOnly(m_songList[m_selectedSong].lkPath.c_str());
            m_previewMusic = Mix_LoadMUS(temp.song_path.c_str());
            if(m_previewMusic){
                Mix_VolumeMusic(0);
                Mix_PlayMusic(m_previewMusic, 0);
                double duration = Mix_MusicDuration(m_previewMusic);
                double startPos = duration * 0.4;
                Mix_SetMusicPosition(startPos);
                m_previewPlaying = true;
                m_previewFade = 0.0f;
                m_previewPlayTime = 0.0f;
            }
        }
    }

    if(m_previewPlaying){
        m_previewPlayTime += deltaMs;

        if(m_previewFade < 1.0f){
            m_previewFade += deltaMs / 1000.0f;
            if(m_previewFade > 1.0f) m_previewFade = 1.0f;
        }

        float fadeOutStart = 20000.0f;
        float fadeOutDuration = 1500.0f;

        if(m_previewPlayTime >= fadeOutStart + fadeOutDuration){
            Mix_SetMusicPosition(Mix_MusicDuration(m_previewMusic) * 0.4);
            m_previewPlayTime = 0.0f;
            m_previewFade = 0.0f;
        } else if(m_previewPlayTime >= fadeOutStart){
            float t = (m_previewPlayTime - fadeOutStart) / fadeOutDuration;
            m_previewFade = 1.0f - t;
        }

        int vol = (int)(m_previewFade * MIX_MAX_VOLUME / 100 * m_settings.volume);
        Mix_VolumeMusic(vol);
    }
}

void Game::renderSongSelectLeft(){
    SDL_Renderer* renderer = m_window.getRenderer();
    
    SDL_Rect bg = {0, 0, PANEL_SPLIT, m_settings.resHeight};
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderFillRect(renderer, &bg);

    SDL_Color color = {255, 255, 255, 255};
    SDL_Color dimColor = {180, 180, 180, 255};

    std::vector<std::pair<std::string, SDL_Color>> lines = {
        {m_songList[m_selectedSong].name, color},
        {"By: " + m_songList[m_selectedSong].author, dimColor},
        {"BPM: " + std::to_string(m_songList[m_selectedSong].bpm), dimColor},
        {"Notes: " + std::to_string(m_beatmap.notes.size()), dimColor}
    };

    int y = 40;
    for(auto& [text, col] : lines){
        SDL_Surface* s = TTF_RenderText_Solid(m_skins.getActive().m_fontUI, text.c_str(), col);
        SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
        int w, h;
        SDL_QueryTexture(t, nullptr, nullptr, &w, &h);
        SDL_Rect dst = {20, y, w, h};
        SDL_RenderCopy(renderer, t, nullptr, &dst);
        SDL_FreeSurface(s);
        SDL_DestroyTexture(t);
        y += h + 10;
    }
}

void Game::renderSongSelectRight(){
    SDL_Renderer* renderer = m_window.getRenderer();
    int y = LIST_START_Y;
    for(int i=0; i<m_songList.size(); i++){
        bool selected = (i==m_selectedSong);

        SDL_Rect bg = {PANEL_SPLIT, y, m_settings.resWidth - PANEL_SPLIT, ENTRY_HEIGHT};
        SDL_SetRenderDrawColor(renderer, selected ? 60 : 30, selected ? 60 : 30, selected ? 60 : 30, 255);
        SDL_RenderFillRect(renderer, &bg);

        SDL_Color color = selected ? SDL_Color{255,255,0,255} : SDL_Color{255,255,255,255};
        SDL_Surface* s = TTF_RenderText_Solid(m_skins.getActive().m_fontUI, m_songList[i].name.c_str(), color);
        SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
        
        int w,h;
        SDL_QueryTexture(t, nullptr, nullptr, &w, &h);
        SDL_Rect dst = {PANEL_SPLIT + 20, y, w, h};
        SDL_RenderCopy(renderer, t, nullptr, &dst);
        
        SDL_FreeSurface(s);
        SDL_DestroyTexture(t);

        if(selected){
            for(int j=0; j<m_songList[i].difficulties.size(); j++){
                if(j == m_selectedDifficulty){
                    SDL_Rect diffBg = {PANEL_SPLIT + 30, y + ENTRY_HEIGHT + j*DIFF_ENTRY_HEIGHT, 200, DIFF_ENTRY_HEIGHT};
                    SDL_SetRenderDrawColor(renderer, 80, 80, 20, 255);
                    SDL_RenderFillRect(renderer, &diffBg);
                }
                SDL_Color diffColor = (j==m_selectedDifficulty) ? SDL_Color{255,255,0,255} : SDL_Color{200,200,200,255};
                SDL_Surface* ds = TTF_RenderText_Solid(m_skins.getActive().s_fontUI, m_songList[i].difficulties[j].c_str(), diffColor);
                SDL_Texture* dt = SDL_CreateTextureFromSurface(renderer, ds);
                
                int dw, dh;
                SDL_QueryTexture(dt, nullptr, nullptr, &dw, &dh);
                SDL_Rect ddst = {PANEL_SPLIT + 40, y + ENTRY_HEIGHT + j*DIFF_ENTRY_HEIGHT, dw, dh};
                SDL_RenderCopy(renderer, dt, nullptr, &ddst);
                
                SDL_FreeSurface(ds);
                SDL_DestroyTexture(dt);
            }
        }

        y+=ENTRY_HEIGHT;
        if(selected) y+=m_songList[i].difficulties.size()*DIFF_ENTRY_HEIGHT;
    }
}

void Game::renderSongSelectMenu(){
    renderSongSelectLeft();
    renderSongSelectRight();
}

void Game::handleSongSelectInput(SDL_Keycode key){
    if(key == SDLK_UP){
        if(m_selectedSong > 0){ 
            m_selectedSong--;
            m_selectedDifficulty = 0;
        }
    } else if(key == SDLK_DOWN){
        if(m_selectedSong < (int)m_songList.size() - 1){
            m_selectedSong++;
            m_selectedDifficulty = 0;
        }
    } else if(key == SDLK_LEFT){
        if(m_selectedDifficulty > 0) m_selectedDifficulty--;
    } else if(key == SDLK_RIGHT){
        if(m_selectedDifficulty < (int)m_songList[m_selectedSong].difficulties.size() - 1)
            m_selectedDifficulty++;
    } else if(key == SDLK_RETURN || key == SDLK_KP_ENTER){
        startGame(m_songList[m_selectedSong].lkPath, m_songList[m_selectedSong].difficulties[m_selectedDifficulty]);
    }
}

void Game::updateGameplay(float deltaMs){
    if(Mix_GetMusicPosition(m_music) < 0 || !Mix_PlayingMusic()){
        endMap();
        return;
    }
    m_debug.update(deltaMs);
    m_visualTime += deltaMs;
    m_syncTimer -= deltaMs;
    if(m_syncTimer <= 0){
        float audioTime = getSongTimeMs();
        if(audioTime > m_visualTime) m_visualTime = audioTime;
        m_syncTimer = 100.0f;
    }
    float songTime = m_visualTime;
    m_judgmentTimer -= deltaMs;
    if(m_judgmentTimer < 0) m_judgmentTimer = 0;
    m_particles.updateParticles(deltaMs);

    if(m_milestoneTimer > 0) m_milestoneTimer -= deltaMs;

    m_hp-=HP_DRAIN_RATE*(deltaMs/1000.0f);
    if(m_hp < 0.0f) m_hp = 0.0f;
    if(m_hp == 0.0f){
        endMap();
        return;
    }

    if(m_hp < 30.0f){
        m_hpPulseTimer += deltaMs;
    }

    for(auto& e : m_hitErrors) e.lifetime -= deltaMs;
    m_hitErrors.erase(
        std::remove_if(m_hitErrors.begin(), m_hitErrors.end(),
            [](const HitError& e){ return e.lifetime <= 0; }),
        m_hitErrors.end()
    );

    for(auto& note : m_beatmap.notes){
        if(note.state == NoteState::Hit || note.state == NoteState::Missed){
            note.fadeTimer -= deltaMs;
            continue;
        }
        if(note.state == NoteState::Holding) continue;

        if(!note.hasColor){
            std::uniform_int_distribution<> dis(180, 255);
            note.r = dis(m_gen);
            note.g = dis(m_gen);
            note.b = dis(m_gen);
            note.hasColor = true;
        }

        float missDeadline = note.timestampMs + MISS_WINDOW_MS;
        if(songTime > missDeadline){
            m_combo = 0;
            m_judgmentText = "MISS";
            m_judgementColor = {255,0,0,255};
            m_judgmentTimer = 800.0f;
            m_misses++;
            m_hp=std::max(0.0f, m_hp-HP_MISS_SUB);
            note.state = NoteState::Missed;
            note.fadeTimer = 200.0f;
            m_changed = true;
            continue;
        }
    }

    if(m_changed){
        updateJudgmentTexture();
        updateComboTexture();
        updateAccuracyTexture();
        m_changed = false;
    }

    if(m_displayPoints < (float)m_points){
        float diff = (float)m_points - m_displayPoints;
        if(diff < 1.0f)
            m_displayPoints = (float)m_points;
        else
            m_displayPoints += diff * 0.15f;
        updatePointsTexture();
    }
}

void Game::renderGameplay(){
    SDL_Renderer* renderer = m_window.getRenderer();
    float songTime = m_visualTime;
    SDL_Texture* noteTex = m_skins.getActive().noteTexture;

    if(m_bgTexture){
        SDL_SetTextureAlphaMod(m_bgTexture, m_settings.bgAlpha);
        SDL_Rect bgDst = {0, 0, m_settings.resWidth, m_settings.resHeight};
        SDL_RenderCopy(renderer, m_bgTexture, nullptr, &bgDst);
    }

    for(auto& note : m_beatmap.notes){
        float timeLeft = note.timestampMs - songTime;
        float timeSinceAppear = APPROACH_TIME_MS - timeLeft;
        Uint8 alpha = 255;
        if(timeSinceAppear < 200.0f) alpha = (Uint8)((timeSinceAppear / 200.0f) * 255);
        float progress = timeLeft / APPROACH_TIME_MS;
        if(progress < 0.0f) progress = 0.0f;
        int margin = (int)(progress * APPROACH_SIZE);

        if(note.type == NoteType::Slider){
            if(note.state == NoteState::Hit || note.state == NoteState::Missed){
                if(note.fadeTimer <= 0) continue;
                Uint8 fadeAlpha = (Uint8)((note.fadeTimer / 200.0f) * 255);
                SDL_Rect rect = {note.gridCol*CELL_SIZE, note.gridRow*CELL_SIZE, CELL_SIZE, CELL_SIZE};
                SDL_SetTextureColorMod(noteTex, 255, 255, 255);
                SDL_SetTextureAlphaMod(noteTex, fadeAlpha);
                SDL_RenderCopy(renderer, noteTex, nullptr, &rect);
                continue;
            }

            if(songTime < note.timestampMs - APPROACH_TIME_MS) continue;

            int x1 = note.gridCol    * CELL_SIZE + CELL_SIZE/2;
            int y1 = note.gridRow    * CELL_SIZE + CELL_SIZE/2;
            int x2 = note.endGridCol * CELL_SIZE + CELL_SIZE/2;
            int y2 = note.endGridRow * CELL_SIZE + CELL_SIZE/2;

            if(note.state == NoteState::Holding){
                SDL_SetRenderDrawColor(renderer, note.r, note.g, note.b, 255);
                SDL_RenderDrawLine(renderer, x1, y1, x2, y2);

                SDL_Rect endRect = {note.endGridCol*CELL_SIZE + CELL_SIZE/4,
                                    note.endGridRow*CELL_SIZE + CELL_SIZE/4,
                                    CELL_SIZE/2, CELL_SIZE/2};
                SDL_RenderFillRect(renderer, &endRect);

                float duration = note.endTimestampMs - note.timestampMs;
                float t = (songTime - note.timestampMs) / duration;
                if(t < 0.0f) t = 0.0f;
                if(t > 1.0f) t = 1.0f;

                int px = note.gridCol*CELL_SIZE + (int)((note.endGridCol - note.gridCol)*CELL_SIZE*t);
                int py = note.gridRow*CELL_SIZE + (int)((note.endGridRow - note.gridRow)*CELL_SIZE*t);

                SDL_SetTextureColorMod(noteTex, 255, 255, 255);
                SDL_SetTextureAlphaMod(noteTex, 255);
                SDL_Rect indicator = {px + CELL_SIZE/4, py + CELL_SIZE/4, CELL_SIZE/2, CELL_SIZE/2};
                SDL_RenderCopy(renderer, noteTex, nullptr, &indicator);

            } else {
                SDL_SetRenderDrawColor(renderer, note.r, note.g, note.b, alpha);
                SDL_RenderDrawLine(renderer, x1, y1, x2, y2);

                SDL_Rect endRect = {note.endGridCol*CELL_SIZE + CELL_SIZE/4,
                                    note.endGridRow*CELL_SIZE + CELL_SIZE/4,
                                    CELL_SIZE/2, CELL_SIZE/2};
                SDL_RenderFillRect(renderer, &endRect);

                SDL_Rect outerRect = {note.gridCol*CELL_SIZE - margin,
                                      note.gridRow*CELL_SIZE - margin,
                                      CELL_SIZE + margin*2, CELL_SIZE + margin*2};
                SDL_SetRenderDrawColor(renderer, note.r, note.g, note.b, alpha);
                SDL_RenderDrawRect(renderer, &outerRect);

                SDL_Rect startRect = {note.gridCol*CELL_SIZE, note.gridRow*CELL_SIZE, CELL_SIZE, CELL_SIZE};
                SDL_SetTextureColorMod(noteTex, note.r, note.g, note.b);
                SDL_SetTextureAlphaMod(noteTex, alpha);
                SDL_RenderCopy(renderer, noteTex, nullptr, &startRect);

                SDL_Texture* texture = m_letterTextures[note.key];
                int w, h;
                SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);
                SDL_Rect dst = {note.gridCol*CELL_SIZE + CELL_SIZE/2 - w/2,
                                note.gridRow*CELL_SIZE + CELL_SIZE/2 - h/2, w, h};
                SDL_RenderCopy(renderer, texture, nullptr, &dst);
            }

        } else if(note.type == NoteType::Tap){
            if(note.state == NoteState::Hit || note.state == NoteState::Missed){
                if(note.fadeTimer <= 0) continue;
                Uint8 fadeAlpha = (Uint8)((note.fadeTimer / 200.0f) * 255);
                SDL_Rect rect = {note.gridCol*CELL_SIZE, note.gridRow*CELL_SIZE, CELL_SIZE, CELL_SIZE};
                SDL_SetTextureColorMod(noteTex, 255, 255, 255);
                SDL_SetTextureAlphaMod(noteTex, fadeAlpha);
                SDL_RenderCopy(renderer, noteTex, nullptr, &rect);
                continue;
            }

            if(songTime >= note.timestampMs - APPROACH_TIME_MS){
                int x = note.gridCol * CELL_SIZE;
                int y = note.gridRow * CELL_SIZE;

                SDL_Rect outerRect = {x-margin, y-margin, CELL_SIZE+margin*2, CELL_SIZE+margin*2};
                SDL_SetRenderDrawColor(renderer, note.r, note.g, note.b, alpha);
                SDL_RenderDrawRect(renderer, &outerRect);

                SDL_Rect rect = {x, y, CELL_SIZE, CELL_SIZE};
                SDL_SetTextureColorMod(noteTex, note.r, note.g, note.b);
                SDL_SetTextureAlphaMod(noteTex, alpha);
                SDL_RenderCopy(renderer, noteTex, nullptr, &rect);

                SDL_Texture* texture = m_letterTextures[note.key];
                int w, h;
                SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);
                SDL_Rect dst = {x + CELL_SIZE/2 - w/2, y + CELL_SIZE/2 - h/2, w, h};
                SDL_RenderCopy(renderer, texture, nullptr, &dst);
            }
        }
    }

    m_particles.renderParticles(renderer);

    if(m_judgmentTimer > 0 && m_judgmentTexture){
        SDL_Rect dst = {m_settings.resWidth/2 - m_judgmentW/2,
                        m_settings.resHeight/2 - m_judgmentH/2,
                        m_judgmentW, m_judgmentH};
        SDL_RenderCopy(renderer, m_judgmentTexture, nullptr, &dst);
    }

    SDL_Rect comboDst = {20, m_settings.resHeight - m_comboH - 20, m_comboW, m_comboH};
    SDL_RenderCopy(renderer, m_comboTexture, nullptr, &comboDst);

    SDL_Rect scoreDst = {m_settings.resWidth - m_scoreW - 20, 20, m_scoreW, m_scoreH};
    SDL_RenderCopy(renderer, m_scoreTexture, nullptr, &scoreDst);

    SDL_Rect accDst = {m_settings.resWidth - m_accuracyW - 20, 20 + m_scoreH + 5, m_accuracyW, m_accuracyH};
    SDL_RenderCopy(renderer, m_accuracyTexture, nullptr, &accDst);

    renderHPBar();
    renderHitErrorBar();
    renderComboMilestone();
    renderHpPulseEffect();
    renderTimerDisplay();
}

void Game::updateResults(float deltaMs){
    m_debug.update(deltaMs);
}

void Game::renderResults(){
    SDL_Renderer* renderer = m_window.getRenderer();
    int cx = m_settings.resWidth / 2;
    int y = 80;

    auto drawCentered = [&](const std::string& text, TTF_Font* font, SDL_Color color){
        SDL_Surface* s = TTF_RenderText_Solid(font, text.c_str(), color);
        SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
        int w, h;
        SDL_QueryTexture(t, nullptr, nullptr, &w, &h);
        SDL_Rect dst = {cx - w/2, y, w, h};
        SDL_RenderCopy(renderer, t, nullptr, &dst);
        SDL_FreeSurface(s);
        SDL_DestroyTexture(t);
        y += h + 15;
    };

    std::string title = m_beatmap.name + " [" + m_songList[m_selectedSong].difficulties[m_selectedDifficulty] + "]";
    drawCentered(title, m_skins.getActive().m_fontUI, {255,255,255,255});

    y += 20;

    std::string grade = getGrade();
    SDL_Color gradeColor = {255,255,255,255};
    if(grade == "S") gradeColor = {255,215,0,255};
    else if(grade == "A") gradeColor = {100,255,100,255};
    else if(grade == "B") gradeColor = {100,100,255,255};
    else if(grade == "C") gradeColor = {255,165,0,255};
    else gradeColor = {255,50,50,255};
    drawCentered(grade, m_skins.getActive().l_fontGame, gradeColor);

    y += 10;

    drawCentered("Score: " + std::to_string(m_points), m_skins.getActive().m_fontUI, {255,255,255,255});
    drawCentered("Accuracy: " + std::to_string((int)getAccuracy()) + "%", m_skins.getActive().m_fontUI, {255,255,255,255});
    drawCentered("Max Combo: " + std::to_string(m_maxCombo) + "x", m_skins.getActive().m_fontUI, {255,255,255,255});

    y += 20;

    drawCentered("Excellent: " + std::to_string(m_excellentHits), m_skins.getActive().m_fontUI, {255,215,0,255});
    drawCentered("Good: "      + std::to_string(m_goodHits),      m_skins.getActive().m_fontUI, {100,255,100,255});
    drawCentered("Miss: "      + std::to_string(m_misses),        m_skins.getActive().m_fontUI, {255,50,50,255});

    y += 30;

    SDL_Color btnColor = {180,180,180,255};

    drawCentered("Press ENTER to go back", m_skins.getActive().s_fontUI, btnColor);
}

void Game::endMap(){
    Mix_HaltMusic();
    m_beatmap.notes.clear();
    m_judgmentTimer = 0.0f;
    int accInt = (int)getAccuracy();
    std::string grade = getGrade();
    m_discordRPC.update(
        m_beatmap.name + " [" + m_songList[m_selectedSong].difficulties[m_selectedDifficulty] + "]",
        std::to_string(accInt) + "% - " + grade + " Grade"
    );
    m_state = GameState::Results;
}

void Game::run(){
    SDL_Renderer *renderer=m_window.getRenderer();
    while(m_running){
        Uint64 now = SDL_GetPerformanceCounter();
        float deltaMs = (float)(now-m_lastTick)/SDL_GetPerformanceFrequency()*1000.0f;
        m_lastTick=now;
        while(SDL_PollEvent(&m_event)){
            if(m_event.type==SDL_QUIT) m_running=false;
            else if(m_event.type == SDL_KEYDOWN){
                if(m_state == GameState::Gameplay)
                    if(m_event.key.keysym.sym == SDLK_ESCAPE) endMap();
                    else handleInput(m_event.key.keysym.sym, m_visualTime);
                else if(m_state == GameState::SongSelectMenu)
                    handleSongSelectInput(m_event.key.keysym.sym);
                else if(m_state == GameState::Results)
                    if(m_event.key.keysym.sym == SDLK_RETURN || m_event.key.keysym.sym == SDLK_KP_ENTER)
                        backToSongSelect();
            } else if(m_event.type==SDL_KEYUP){
                if(m_state == GameState::Gameplay) handleKeyUp(m_event.key.keysym.sym, m_visualTime);
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        switch(m_state){
            case GameState::SongSelectMenu: updateSongSelectMenu(deltaMs); renderSongSelectMenu(); break;
            case GameState::Countdown: updateCountdown(deltaMs); renderCountdown(); break;
            case GameState::Gameplay: updateGameplay(deltaMs); renderGameplay(); break;
            case GameState::Results: updateResults(deltaMs); renderResults(); break;
            default: break;
        }

        m_debug.render(renderer);
        m_discordRPC.runCallbacks();
        SDL_RenderPresent(renderer);
    }
}

void Game::backToSongSelect(){
    m_judgmentText = "";
    m_judgementColor = {255,255,255,255};
    m_combo = 0;
    m_points = 0;
    m_displayPoints = 0.0f;
    m_hitErrors.clear();
    m_state = GameState::SongSelectMenu;
    m_discordRPC.update("In Menus", "Browsing songs");
}

void Game::shutdown(){
    m_discordRPC.shutdown();
    for(auto& pair:m_letterTextures) SDL_DestroyTexture(pair.second);
    SDL_DestroyTexture(m_judgmentTexture);
    SDL_DestroyTexture(m_comboTexture);
    SDL_DestroyTexture(m_scoreTexture);
    SDL_DestroyTexture(m_accuracyTexture);
    SDL_DestroyTexture(m_bgTexture);
    m_skins.getActive().unload();
    TTF_Quit();
    if(m_previewMusic) Mix_FreeMusic(m_previewMusic);
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
    float adjustedTime = songTime + m_settings.offset;
    char k = (char)std::tolower(key);

    for(auto& note : m_beatmap.notes){
        if(note.type == NoteType::Slider && note.state == NoteState::Holding)
            if(std::tolower(note.key) == k) return;
    }

    for(auto& note : m_beatmap.notes){
        if(note.type != NoteType::Slider) continue;
        if(note.state != NoteState::Waiting) continue;
        if(std::tolower(note.key) != k) continue;
        if(std::abs(adjustedTime - note.timestampMs) <= GOOD_WINDOW_MS){
            note.state = NoteState::Holding;
            m_particles.spawnParticles(
                note.gridCol * CELL_SIZE + CELL_SIZE/2,
                note.gridRow * CELL_SIZE + CELL_SIZE/2,
                note.r, note.g, note.b, 12);
            return;
        }
    }

    Note* bestNote = nullptr;
    float bestDiff = 1e9f;
    for(auto& note : m_beatmap.notes){
        if(note.type != NoteType::Tap) continue;
        if(note.state != NoteState::Waiting) continue;
        if(std::tolower(note.key) != k) continue;
        float diff = std::abs(adjustedTime - note.timestampMs);
        if(diff < bestDiff){ bestDiff = diff; bestNote = &note; }
    }

    if(!bestNote) return;
    float error = adjustedTime - bestNote->timestampMs;
    m_hitErrors.push_back({error, 2000.0f, bestDiff <= EXCELLENT_WINDOW_MS});

    if(songTime < bestNote->timestampMs - APPROACH_TIME_MS) return;

    m_particles.spawnParticles(
        bestNote->gridCol * CELL_SIZE + CELL_SIZE/2,
        bestNote->gridRow * CELL_SIZE + CELL_SIZE/2,
        bestNote->r, bestNote->g, bestNote->b, 12);

    if(bestDiff <= EXCELLENT_WINDOW_MS){
        m_judgmentText = "EXCELLENT";
        m_judgementColor = {255,215,0,255};
        m_combo++;
        if(m_combo>m_maxCombo) m_maxCombo = m_combo;
        m_excellentHits++;
        m_hp=std::min(m_maxHp, m_hp+HP_EXCELLENT_ADD);
        m_points += 300 * m_combo;
    } else if(bestDiff <= GOOD_WINDOW_MS){
        m_judgmentText = "GOOD";
        m_judgementColor = {0,128,0,255};
        m_combo++;
        if(m_combo>m_maxCombo) m_maxCombo = m_combo;
        m_goodHits++;
        m_hp=std::min(m_maxHp, m_hp+HP_GOOD_ADD);
        m_points += 100 * m_combo;
    } else {
        m_judgmentText = "MISS";
        m_judgementColor = {255,0,0,255};
        m_combo = 0;
        m_misses++;
        m_hp=std::max(0.0f, m_hp-HP_MISS_SUB);
    }
    if(m_combo == 50 || m_combo == 100 || m_combo == 200 || m_combo % 100 == 0){
        m_milestoneTimer = 800.0f;
        m_milestoneCombo = m_combo;
    }
    bestNote->state = NoteState::Hit;
    bestNote->fadeTimer = 200.0f;
    m_judgmentTimer = 800.0f;
    m_changed = true;
}

void Game::handleKeyUp(SDL_Keycode key, float songTime){
    float adjustedTime = songTime + m_settings.offset;
    for(auto& note : m_beatmap.notes){
        if(note.type != NoteType::Slider) continue;
        if(note.state != NoteState::Holding) continue;
        if((char)key != std::tolower(note.key)) continue;

        float diff = std::abs(adjustedTime - note.endTimestampMs);
        if(diff <= EXCELLENT_WINDOW_MS){
            m_judgmentText = "EXCELLENT";
            m_judgementColor = {255,215,0,255};
            m_combo++;
            if(m_combo>m_maxCombo) m_maxCombo = m_combo;
            m_excellentHits++;
            m_hp=std::min(m_maxHp, m_hp+HP_EXCELLENT_ADD);
            m_points += 300 * m_combo;
        } else if(diff <= GOOD_WINDOW_MS){
            m_judgmentText = "GOOD";
            m_judgementColor = {0,128,0,255};
            m_combo++;
            m_goodHits++;
            m_hp=std::min(m_maxHp, m_hp+HP_GOOD_ADD);
            m_points += 100 * m_combo;
        } else {
            m_judgmentText = "MISS";
            m_judgementColor = {255,0,0,255};
            m_combo = 0;
            if(m_combo>m_maxCombo) m_maxCombo = m_combo;
            m_misses++;
            m_hp=std::max(0.0f, m_hp-HP_MISS_SUB);
        }
        if(m_combo == 50 || m_combo == 100 || m_combo == 200 || m_combo % 100 == 0){
            m_milestoneTimer = 800.0f;
            m_milestoneCombo = m_combo;
        }
        note.state = NoteState::Hit;
        note.fadeTimer = 200.0f;
        m_judgmentTimer = 800.0f;
        float error = adjustedTime - note.endTimestampMs;
        m_hitErrors.push_back({error, 2000.0f, diff <= EXCELLENT_WINDOW_MS});
        m_changed = true;
        break;
    }
}