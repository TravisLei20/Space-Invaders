#include "HighScores.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <unistd.h>


std::string getExecutablePath();

#define PATH_MAX 1000


// Constructor
HighScores::HighScores(uint32_t score, std::string username) {
  this->score = score;
  this->userEntry = username;
}

void HighScores::init() {

  // Get the executable path
  // printf("Getting executable path\n");
  filePath = getExecutablePath();

  // Now, you can modify the filePath to include the high scores file name
  filePath += "/high_scores.txt";

  // Read the high scores file into the highScores vector
  std::ifstream inputFile(filePath);

  if (inputFile.is_open()) {
    std::string line;
    while (std::getline(inputFile, line)) {
      std::istringstream iss(line);
      std::string initials;
      uint32_t highScore;

      // Assuming each line in the file has the format "initials score"
      if (iss >> initials >> highScore) {
        all_high_scores.emplace_back(std::make_pair(initials, highScore));
      } else {
        // Handle a line with incorrect format (if any)
        std::cerr << "Error parsing line: " << line << std::endl;
      }
    }

    inputFile.close();
  } else {
    // Inform the user that the file does not exist.
    std::cerr << "High scores file does not exist yet. Creating a new one."
              << std::endl;

    // Create a new high scores file
    std::ofstream outputFile(filePath);
    if (!outputFile.is_open()) {
      // Handle error creating the file
      std::cerr << "Error creating high scores file: " << filePath << std::endl;
    } else {
      // Close the file after creating it
      outputFile.close();
    }
  }
}

// This should return the symbolic path where the exectuble is, which will be
// used to create/locate the high_scores.txt
std::string getExecutablePath() {

  // Get the path of the executable
  char getExecutablePath[PATH_MAX];
  ssize_t len = readlink("/proc/self/exe", getExecutablePath,
                         sizeof(getExecutablePath) - 1);
  if (len != -1) {
    getExecutablePath[len] = '\0'; // Null-terminate the string
    // Extract the directory from the path
    // lastSlash contains the index of the last '/' in the string produced by
    // find_last_of()
    size_t lastSlash = std::string(getExecutablePath).find_last_of('/');
    if (lastSlash != std::string::npos) {
      return std::string(getExecutablePath, 0, lastSlash + 1);
    } else {
      std::cerr << "Error getting executable path; lastSlash = npos"
                << std::endl; // Print an error if unable to get file path
      return "";
    }
  } else {
    std::cerr << "Error getting executable path; readlink returned -1"
              << std::endl; // Print an error if unable to get file path
    return "";
  }
}


// Update the highScores vector with the user initials and score
void HighScores::updateHighScores() {

  // Add the full userEntry and score pair into the vector
  all_high_scores.emplace_back(std::make_pair(userEntry, score));

  // Sort the high scores vector in ascending order based on the score
  std::sort(all_high_scores.begin(), all_high_scores.end(),
            [](const std::pair<std::string, uint32_t> &a,
               const std::pair<std::string, uint32_t> &b) {
              return a.second > b.second;
            });

  // Keep only the top 8 high scores
  if (all_high_scores.size() > HIGH_SCORES_MAX_ENTRIES) {
    all_high_scores.resize(HIGH_SCORES_MAX_ENTRIES);
  }
  // If the current list of scores is less than 8, fill in the list with default names and scores
  while (all_high_scores.size() < HIGH_SCORES_MAX_ENTRIES) {
    all_high_scores.emplace_back(std::make_pair("AAA", 0));
  }
}


// Save the high scores vector to the high_scores.txt file
void HighScores::save() {
  std::ofstream outputFile(filePath);

  if (outputFile.is_open()) {
    for (const auto &entry : all_high_scores) {
      outputFile << entry.first << " " << entry.second << std::endl;
    }

    outputFile.close();
  } else {
    // Handle error opening the file for writing
    std::cerr << "Error opening high scores file for writing: " << filePath
              << std::endl;
  }
}
