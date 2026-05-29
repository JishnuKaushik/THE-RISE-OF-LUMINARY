#include "StoryManager.hpp"
#include <fstream>
#include <iostream>
#include <cctype>

// ─── Minimal JSON helpers ─────────────────────────────────────────────────────

static std::pair<std::string, size_t> smReadStr(const std::string& s, size_t p) {
    std::string out;
    while (p < s.size()) {
        if (s[p] == '"') return {out, p};
        if (s[p] == '\\' && p + 1 < s.size()) {
            char esc = s[p + 1];
            if      (esc == '"')  { out += '"';  p += 2; }
            else if (esc == '\\') { out += '\\'; p += 2; }
            else if (esc == 'n')  { out += '\n'; p += 2; }
            else if (esc == 't')  { out += '\t'; p += 2; }
            else if (esc == 'u' && p + 5 < s.size()) {
                std::string hex = s.substr(p + 2, 4);
                try {
                    unsigned int cp = std::stoul(hex, nullptr, 16);
                    if      (cp < 0x80)  { out += static_cast<char>(cp); }
                    else if (cp < 0x800) {
                        out += static_cast<char>(0xC0 | (cp >> 6));
                        out += static_cast<char>(0x80 | (cp & 0x3F));
                    } else {
                        out += static_cast<char>(0xE0 | (cp >> 12));
                        out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                        out += static_cast<char>(0x80 | (cp & 0x3F));
                    }
                } catch (...) { out += '?'; }
                p += 6;
            } else {
                out += s[p++];
            }
        } else {
            out += s[p++];
        }
    }
    return {out, p};
}

static std::string smStr(const std::string& obj, const std::string& key) {
    std::string sk = "\"" + key + "\"";
    size_t kp = obj.find(sk);
    if (kp == std::string::npos) return "";
    size_t cp = obj.find(':', kp + sk.size());
    if (cp == std::string::npos) return "";
    size_t q = obj.find('"', cp + 1);
    if (q == std::string::npos) return "";
    return smReadStr(obj, q + 1).first;
}

static std::vector<std::string> smArr(const std::string& obj, const std::string& key) {
    std::vector<std::string> results;
    std::string sk = "\"" + key + "\"";
    size_t kp = obj.find(sk);
    if (kp == std::string::npos) return results;
    size_t as = obj.find('[', kp + sk.size());
    if (as == std::string::npos) return results;
    size_t ae = obj.find(']', as + 1);
    if (ae == std::string::npos) return results;
    size_t pos = as + 1;
    while (pos < ae) {
        size_t q = obj.find('"', pos);
        if (q == std::string::npos || q >= ae) break;
        auto [val, endPos] = smReadStr(obj, q + 1);
        results.push_back(val);
        pos = endPos + 1;
    }
    return results;
}

static int smInt(const std::string& obj, const std::string& key, int def = 0) {
    std::string sk = "\"" + key + "\"";
    size_t kp = obj.find(sk);
    if (kp == std::string::npos) return def;
    size_t cp = obj.find(':', kp + sk.size());
    if (cp == std::string::npos) return def;
    size_t p = cp + 1;
    while (p < obj.size() && (obj[p] == ' ' || obj[p] == '\t' ||
                               obj[p] == '\n' || obj[p] == '\r')) ++p;
    std::string numStr;
    while (p < obj.size() && (std::isdigit(static_cast<unsigned char>(obj[p])) ||
                               obj[p] == '-')) numStr += obj[p++];
    if (numStr.empty()) return def;
    try { return std::stoi(numStr); } catch (...) { return def; }
}

static std::string smNestedObj(const std::string& obj, const std::string& key) {
    std::string sk = "\"" + key + "\"";
    size_t kp = obj.find(sk);
    if (kp == std::string::npos) return "";
    size_t cp = obj.find(':', kp + sk.size());
    if (cp == std::string::npos) return "";
    size_t ob = obj.find('{', cp + 1);
    if (ob == std::string::npos) return "";
    int depth = 1;
    size_t pos = ob + 1;
    while (pos < obj.size() && depth > 0) {
        if      (obj[pos] == '{') ++depth;
        else if (obj[pos] == '}') --depth;
        ++pos;
    }
    return obj.substr(ob, pos - ob);
}

// ─────────────────────────────────────────────────────────────────────────────

StoryManager::StoryManager() {}
StoryManager::~StoryManager() {}

void StoryManager::loadStoriesFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[StoryManager] Cannot open: " << filepath << "\n";
        return;
    }
    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    file.close();

    std::string subject = smStr(content, "subject");
    if (subject.empty()) {
        std::cerr << "[StoryManager] No 'subject' key in: " << filepath << "\n";
        return;
    }

    size_t chapKey = content.find("\"chapters\"");
    if (chapKey == std::string::npos) return;
    size_t arrStart = content.find('[', chapKey);
    if (arrStart == std::string::npos) return;

    std::vector<StoryChapter> chapters;
    size_t pos = arrStart + 1;

    while (pos < content.size()) {
        size_t objStart = content.find('{', pos);
        if (objStart == std::string::npos) break;

        int depth = 1;
        size_t objEnd = objStart + 1;
        while (objEnd < content.size() && depth > 0) {
            if      (content[objEnd] == '{') ++depth;
            else if (content[objEnd] == '}') --depth;
            ++objEnd;
        }

        std::string obj = content.substr(objStart, objEnd - objStart);

        StoryChapter ch;
        ch.chapterNumber      = smInt(obj, "chapterNumber");
        ch.chapterName        = smStr(obj, "chapterName");
        ch.storyIntro         = smStr(obj, "storyIntro");
        ch.conceptExplanation = smStr(obj, "conceptExplanation");
        ch.realLifeExample    = smStr(obj, "realLifeExample");

        std::string qcStr = smNestedObj(obj, "quickCheck");
        if (!qcStr.empty()) {
            ch.quickCheck.question    = smStr(qcStr, "question");
            ch.quickCheck.options     = smArr(qcStr, "options");
            ch.quickCheck.correct     = smInt(qcStr, "correct");
            ch.quickCheck.explanation = smStr(qcStr, "explanation");
        }

        if (!ch.chapterName.empty())
            chapters.push_back(std::move(ch));

        pos = objEnd;
    }

    storiesBySubject[subject] = std::move(chapters);
    std::cout << "[StoryManager] Loaded " << storiesBySubject[subject].size()
              << " chapters for '" << subject << "'\n";
}

void StoryManager::loadAllStories() {
    storiesBySubject.clear();
    static const std::vector<std::string> files = {
        "data/stories/Mathematics_Stories.json",
        "data/stories/Science_Stories.json",
        "data/stories/History_Stories.json",
        "data/stories/Literature_Stories.json",
        "data/stories/Arts_Stories.json"
    };
    for (const auto& f : files) loadStoriesFromFile(f);
}

std::vector<StoryChapter> StoryManager::getStoriesBySubject(const std::string& subject) {
    auto it = storiesBySubject.find(subject);
    if (it != storiesBySubject.end()) return it->second;
    return {};
}

int StoryManager::getTotalChapters(const std::string& subject) {
    auto it = storiesBySubject.find(subject);
    if (it != storiesBySubject.end()) return static_cast<int>(it->second.size());
    return 0;
}

void StoryManager::printStats() {
    for (const auto& [subj, chaps] : storiesBySubject)
        std::cout << "[StoryManager] " << subj << ": " << chaps.size() << " chapters\n";
}
