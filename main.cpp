#include <iostream>
#include <fstream>
#include <filesystem>
#include <regex>
#include <vector>
#include <map>
#include <algorithm>

using namespace std;
namespace fs = std::filesystem;

// Define the structure to hold the page and paragraph information
struct Paragraph {
    string id;
    string text;
};

// Define the structure to hold the classification results
struct Classification {
    string id;
    string classType;
};

// Function to classify the paragraphs
vector<Classification> classifyParagraphs(const vector<Paragraph> &paragraphs) {
    vector<Classification> classifications;
    map<string, string> classMap;

    // Regular expressions for different sections
    regex titlePageRegex(R"(UNIVERZA|EKONOMSKA FAKULTETA|DIPLOMSKO DELO)");
    regex tocRegex(R"(KAZALO|CONTENTS)");
    regex abstractSloRegex(R"(POVZETEK)");
    regex abstractEnRegex(R"(ABSTRACT)");
    regex chapterRegex(R"(POGLAVJE|CHAPTER)");
    regex conclusionRegex(R"(SKLEP|CONCLUSION)");
    regex bibliographyRegex(R"(LITERATURA|BIBLIOGRAPHY)");
    regex acronymRegex(R"(HTML|LaTeX|FERI)");

    for (const auto &paragraph: paragraphs) {
        string classType = "body"; // Default classification

        if (regex_search(paragraph.text, titlePageRegex)) {
            classType = "titlePage";
        } else if (regex_search(paragraph.text, tocRegex)) {
            classType = "toc";
        } else if (regex_search(paragraph.text, abstractSloRegex)) {
            classType = "abstractSlo";
        } else if (regex_search(paragraph.text, abstractEnRegex)) {
            classType = "abstractEn";
        } else if (regex_search(paragraph.text, chapterRegex)) {
            classType = "chapter";
        } else if (regex_search(paragraph.text, conclusionRegex)) {
            classType = "conclusion";
        } else if (regex_search(paragraph.text, bibliographyRegex)) {
            classType = "bibliography";
        } else if (regex_search(paragraph.text, acronymRegex)) {
            classType = "acronym";
        }

        classifications.push_back({paragraph.id, classType});
    }

    return classifications;
}

// Function to read the XML file and extract paragraphs
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
            paragraphs.push_back({id, text});
        }
    }

    return paragraphs;
}

// Function to write the classification results to a file
void writeResults(const string &filename, const vector<Classification> &classifications) {
    ofstream file("results/" + filename);
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

vector<string> getFileNames(const vector<string>& filePaths) {
    vector<string> fileNames;
    for (const auto& filePath : filePaths) {
        fileNames.push_back(fs::path(filePath).stem().string());
    }
    return fileNames;
}

int main() {
    vector<string> filenames = listFilesInDirectory("korpus/");

    for (const auto &filename: filenames) {
        vector<Paragraph> paragraphs = readXML(filename);
        vector<Classification> classifications = classifyParagraphs(paragraphs);
        //replace(filename.find("korpus"), 6, "results");
        writeResults(filename + ".res", classifications);
        cout << "Classification completed. Results written to results/" + filename + ".res" << endl;
    }

    return 0;
}
