#pragma once
#include <string>
#include <vector>
#include "../deps/sqlite3.h"
#include <SDL2/SDL.h>

struct ScoreEntry {
    std::string playerName="Guest";
    std::string lkHash;
    std::string difficulty;
    int score;
    float accuracy;
    int maxCombo;
    std::string grade;
    int excellentCounts;
    int goodCounts;
    int missCounts;
    bool noFail;
    long timestamp;

    std::string displayText;
    SDL_Texture* tex = nullptr;
    int texW = 0;
    int texH = 0;
};

class Database {
    sqlite3 *m_db=nullptr;
    std::vector<ScoreEntry> m_scores;
public:
    bool init(const std::string &path);
    void saveScore(const ScoreEntry &entry);
    void getScores(const std::string &lkHash, const std::string &difficulty, std::vector<ScoreEntry> &outScores);
    std::vector<ScoreEntry> getScores(const std::string &lkHash, const std::string &difficulty);
    void close();
};