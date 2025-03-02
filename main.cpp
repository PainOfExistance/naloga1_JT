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

vector<Classification> classifyParagraphs(const vector<Paragraph> &paragraphs) {
    vector<Classification> classifications;
    vector<Classification> segments;
    vector<Classification> chapters;

    regex titlePageRegex(R"(UNIVERZA|EKONOMSKA FAKULTETA|DIPLOMSKO DELO)");
    regex forewordRegex(R"(PREDGOVOR|FOREWORD)");
    regex tocRegex(R"(KAZALO|CONTENTS|\.\.\.\.\.\.\.\.\.\.)");
    regex abstractSloRegex(R"(POVZETEK)");
    regex abstractEnRegex(R"(ABSTRACT)");
    regex chapterRegex(R"(>\d+ [Aa-Zz]<)");
    regex conclusionRegex(R"(SKLEP|CONCLUSION)");
    regex bibliographyRegex(R"(LITERATURA|BIBLIOGRAPHY|\[\d+\])");
    regex acronymRegex(R"(KLJUČNE|LaTeX|FERI)");

    string prevID = "";
    string prevClassType = "";

    for (const auto &paragraph: paragraphs) {
        string classType = "body";

        if (regex_search(paragraph.text, titlePageRegex)) {
            classType = "titlePage";
        } else if (regex_search(paragraph.text, forewordRegex)) {
            classType = "foreword";
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

        if (classType == "body" && prevID.substr(0, prevID.find('.')) == paragraph.id.
            substr(0, paragraph.id.find('.'))) {
            classifications.push_back({paragraph.id, prevClassType});
            prevClassType = classType;
        } else {
            classifications.push_back({paragraph.id, classType});
            prevClassType = classType;
        }


        if (prevID.substr(0, prevID.find('.')) == paragraph.id.substr(0, paragraph.id.find('.'))) {
            continue;
        }

        if (prevID.substr(0, prevID.find('.')) != paragraph.id.substr(0, paragraph.id.find('.'))) {
            if (classType == "titlePage" || classType == "toc" || classType == "foreword") {
                segments.push_back({paragraph.id.substr(0, paragraph.id.find('.')), "front"});
            } else if (classType == "abstractSlo" || classType == "abstractEn" || classType == "conclusion" || classType
                       == "bibliography" || classType == "acronym") {
                segments.push_back({paragraph.id.substr(0, paragraph.id.find('.')), "back"});
            } else {
                segments.push_back({paragraph.id.substr(0, paragraph.id.find('.')), "body"});
            }
        }


        if (regex_search(paragraph.text, chapterRegex)) {
            chapters.push_back({
                paragraph.id.substr(0, paragraph.id.find('.')),
                paragraph.text.substr(paragraph.id.find('>'), paragraph.id.find('<'))
            });
        }

        prevID = paragraph.id;
    }


    for (const auto &seg: segments) {
        classifications.push_back({seg.id, seg.classType});
    }

    for (const auto &chapter: chapters) {
        classifications.push_back({chapter.id, chapter.classType});
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
            paragraphs.push_back({id, text});
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
