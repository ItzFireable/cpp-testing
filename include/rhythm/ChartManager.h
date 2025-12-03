#ifndef CHART_MANAGER_H
#define CHART_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include <algorithm>

enum NoteType { TAP, HOLD_START, HOLD_END };

struct Note {
    int time;       // Time in milliseconds
    int column;     // Column index (0 to keyCount-1)
    NoteType type;  // Type of note

    // Comparison operator for sorting
    bool operator<(const Note& other) const {
        return time < other.time;
    }
};

struct TimingPoint {
    int time;
    double bpm;
};

struct ChartData {
    std::string filename;
    std::string filePath;

    std::vector<Note> notes;
    std::vector<TimingPoint> timingPoints;
    std::map<std::string, std::string> metadata;
    int keyCount = 4;
};

class ChartManager {
public:
    static ChartData parseChart(const std::string& filename, const std::string& content);
    
private:
    static ChartData parseVsc(const std::string& content);
    static ChartData convertOsuToChartData(const std::string& osuContent);
};

#endif