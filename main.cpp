#include <iostream>
#include <fstream>
#include <filesystem>
#include <regex>
#include <vector>
#include <map>
#include <algorithm>

using namespace std;
namespace fs = std::filesystem;

struct Paragraph {
    string id;
    string text;
};

struct Classification {
    string id;
    string classType;
};

bool contains(vector<Classification> paragraphs, string classType) {
    for (int i = 0; i < paragraphs.size(); i++) {
        if (paragraphs[i].classType == classType) {
            return true;
        }
    }
    return false;
}

vector<Classification> classifyParagraphs(const vector<Paragraph> &paragraphs) {
    vector<Classification> classifications;
    vector<Classification> segments;
    vector<Classification> chapters;

    regex titlePageRegex(R"(UNIVERZA|EKONOMSKA FAKULTETA|DIPLOMSKO DELO)", std::regex_constants::icase);
    regex tocRegex(R"(KAZALO|CONTENTS|\.\.\.\.\.\.\.\.\.\.)", std::regex_constants::icase);
    regex toaRegex(R"(KAZALO KRATIC|KRATICE)", std::regex_constants::icase);
    regex abstractSloRegex(R"(POVZETEK)", std::regex_constants::icase);
    regex abstractEnRegex(R"(ABSTRACT)", std::regex_constants::icase);
    regex abstractDeRegex(R"(ABSTRAKT|ZUSAMMENFASSUNG)", std::regex_constants::icase);
    regex chapterRegex(R"(>\d+ [A-Za-z \s]+<)", std::regex_constants::icase);
    regex conclusionRegex(R"(>\d+ [SKLEP \s]+<|>\d+ [CONCLUSION \s]+<)", std::regex_constants::icase);

    string prevID = "";
    string prevClassType = "";

    for (const auto &paragraph: paragraphs) {
        string classType = "body";

        if (regex_search(paragraph.text, titlePageRegex)) {
            classType = "titlePage";
        } else if (regex_search(paragraph.text, tocRegex)) {
            classType = "toc";
        } else if (regex_search(paragraph.text, toaRegex)) {
            classType = "toa";
        } else if (regex_search(paragraph.text, abstractSloRegex)) {
            classType = "abstractSlo";
        } else if (regex_search(paragraph.text, abstractEnRegex)) {
            classType = "abstractEn";
        } else if (regex_search(paragraph.text, abstractDeRegex)) {
            classType = "abstractDe";
        } else if (regex_search(paragraph.text, conclusionRegex)) {
            classType = "conclusion";
        } else if (regex_search(paragraph.text, chapterRegex)) {
            classType = "chapter";
        }

        if (classType == "body" && prevID.substr(0, prevID.find('.')) == paragraph.id.
            substr(0, paragraph.id.find('.'))) {
            classifications.push_back({paragraph.id, prevClassType});
        } else if (!contains(classifications, "chapter") && classType == "body") {
            classifications.push_back({paragraph.id, "front"});
            prevClassType = "front";
        } else if (classifications.size() > 0 && contains(classifications, "chapter") && !contains(
                       classifications, "conclusion") && classType == "body") {
            classifications.push_back({paragraph.id, "body"});
            prevClassType = "body";
        } else if (classifications.size() > 0 && classifications.end()->classType != "chapter" && classType == "body") {
            classifications.push_back({paragraph.id, "back"});
            prevClassType = "back";
        } else {
            classifications.push_back({paragraph.id, classType});
            prevClassType = classType;
        }

        prevID = paragraph.id;
    }

    return classifications;
}

vector<Paragraph> readXML(const string &filename) {
    ifstream file(filename);
    vector<Paragraph> paragraphs;
    string line;
    regex paragraphRegex(R"(<p xml:id="([^"]+)[^>]*>([^<]+)</p>)");

    while (getline(file, line)) {
        smatch match;
        if (regex_search(line, match, paragraphRegex)) {
            string id = match[1];
            string text = match[2];
            paragraphs.push_back({id, line});
        }
    }

    return paragraphs;
}

void writeResults(const string &filename, const vector<Classification> &classifications) {
    ofstream file(filename);
    file << "ID CLASS\n";
    for (const auto &classification: classifications) {
        file << classification.id << " " << classification.classType << "\n";
    }
}

vector<string> listFilesInDirectory(const string &directoryPath) {
    vector<string> fileList;
    for (const auto &entry: fs::directory_iterator(directoryPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".xml") {
            fileList.push_back(entry.path().string());
        }
    }
    return fileList;
}

int main() {
    vector<string> filenames = listFilesInDirectory("korpus/");

    for (const auto &filename: filenames) {
        vector<Paragraph> paragraphs = readXML(filename);
        vector<Classification> classifications = classifyParagraphs(paragraphs);
        writeResults("results" + filename.substr(6, filename.length()) + ".res", classifications);
        cout << "Classification completed. Results written to results" + filename.substr(6, filename.length()) + ".res"
                << endl;
    }

    return 0;
}
