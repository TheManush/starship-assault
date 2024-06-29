#include "highScoreFile.h"
#include <iostream>
#include <fstream>
#include <algorithm>

// Number of high scores to store
const int NUM_HIGH_SCORES = 10;

void HighScoreFile::setHighScores(const std::vector<int>& scores) {
    std::ofstream wf("gameData.dat", std::ios::out | std::ios::binary);
    if (!wf) {
        std::cerr << "Cannot open file!" << std::endl;
        return;
    }

    // Write the number of scores first
    int count = scores.size();
    wf.write(reinterpret_cast<const char*>(&count), sizeof(count));

    // Write the scores
    wf.write(reinterpret_cast<const char*>(scores.data()), count * sizeof(int));

    wf.close();
    if (!wf.good()) {
        std::cerr << "Error occurred at writing time!" << std::endl;
    }
}

std::vector<int> HighScoreFile::getHighScores() {
    std::vector<int> highScores;

    std::ifstream rf("gameData.dat", std::ios::in | std::ios::binary);
    if (!rf) {
        std::cerr << "Cannot open file!" << std::endl;
        return highScores;
    }

    // Read the number of scores first
    int count;
    rf.read(reinterpret_cast<char*>(&count), sizeof(count));

    if (count > 0) {
        highScores.resize(count);
        // Read the scores
        rf.read(reinterpret_cast<char*>(highScores.data()), count * sizeof(int));
    }

    rf.close();
    if (!rf.good()) {
        std::cerr << "Error occurred at reading time!" << std::endl;
    }

    return highScores;
}