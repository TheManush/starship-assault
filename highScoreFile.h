#pragma once

#include <vector>

class HighScoreFile {
public:
    void setHighScores(const std::vector<int>& scores);
    std::vector<int> getHighScores();
};