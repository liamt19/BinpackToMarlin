// BinpackToMarlin.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <iostream>
#include <filesystem>

#include "binpack_stream.h"
using namespace binpack;

using std::cin;
using std::cout;
using std::endl;

#define HIGH_SCORE_SKIP 5000    // Skip evaluations above 5000 or below -5000
#define WIN_BAD_SCORE 400       // Skip positions where the winning side was losing by more than 400

int main(int argc, char* argv[])
{
    const long printEvery = 10000000;   //  Show a "Progress" message every # positions
    const long flushEvery = 100000000;  //  Total positions per outputted text file

    std::string DataFolder;
    if (argc >= 2) {
        DataFolder = argv[1];
    }
    else {
        cout << "The first parameter needs to be an input path! Trying from current directory" << endl;
        DataFolder = ".\\";
    }

    std::string inputFolder = DataFolder;

    int outputFileIndex = 0;

    long long totalPositions = 0;
    for (const auto& entry : std::filesystem::directory_iterator(inputFolder)) {
        if (entry.path().string().find(".binpack") == std::string::npos) {
            continue;
        }

        cout << "Processing " << entry.path().string() << endl;

        std::function<bool(const TrainingDataEntry&)> skips([](const TrainingDataEntry& entry) -> bool {
            return (entry.isInCheck() || entry.isCapturingMove() || entry.ply < 16);
        });

        BinpackSfenInputStream binpackStream(entry.path().string(), false, skips);

        if (argc > 2) {
			std::string arg = argv[2];
            outputFileIndex = std::stoi(arg);
            cout << "Skipping " << (flushEvery * outputFileIndex) << " positions" << endl;
		}

        long initialSkips = flushEvery * outputFileIndex;

        long badWScores = 0;
        long badBScores = 0;
        long mateScores = 0;

        long long positionNumber = 0;
        std::optional<TrainingDataEntry> next = binpackStream.next();

        while (initialSkips > 0) {
            next = binpackStream.next();
            initialSkips--;
            positionNumber++;
        }

        std::ofstream outputStream(DataFolder + "output_" + std::to_string(outputFileIndex) + ".txt");

        while ((next = binpackStream.next()).has_value()) {
            const auto& val = next.value();

            auto fen = val.pos.fen();
            auto score = val.score;
            auto result = val.result;

            if (val.pos.sideToMove() == chess::Color::Black) {
                score *= -1;
                result *= -1;
            }

            if (result == 1 && score < -WIN_BAD_SCORE) {
                badWScores++;
                continue;
            }

            if (result == -1 && score > WIN_BAD_SCORE) {
                badBScores++;
                continue;
            }

            if (score > HIGH_SCORE_SKIP || score < -HIGH_SCORE_SKIP) {
                mateScores++;
                continue;
            }

            outputStream << fen << " | " << score;

            switch (result) {
            case 1:
                outputStream << " | 1.0\n";
                break;
            case 0:
                outputStream << " | 0.5\n";
                break;
            case -1:
                outputStream << " | 0.0\n";
                break;
            }

            positionNumber++;
            if (positionNumber % printEvery == 0) {
                cout << "Progress: " << positionNumber << " positions.\t";
                cout << " " << badWScores << " " << badBScores << " " << mateScores << endl;
            }

            if (positionNumber % flushEvery == 0) {
                outputStream.flush();
                outputStream = std::ofstream(DataFolder + "output_" + std::to_string(++outputFileIndex) + ".txt");
            }
        }

        cout << "binpackStream done " << positionNumber << " in " << entry.path().string() << endl;
        totalPositions += positionNumber;
    }

    cout << "finished " << totalPositions << endl;
    cin.get();
}
