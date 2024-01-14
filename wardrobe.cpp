#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <random>
#include <unordered_map>
#include <vector>

#include <curl/curl.h>

#include "json.hpp"

using namespace std;
using json = nlohmann::json;

struct ClothingItem {
    string name;
    string type;
    string color;
    string lastWornTime = "01/01/2023";
    string lastWornTimeToString() const { return lastWornTime; }

    void setLastWornTimeFromString(const string& str) { lastWornTime = str; }
};


class WeatherApiClient {
private:
    string apiKey;

    static size_t WriteCallBack(void* contents, size_t size, size_t nmemb, string* output) {
        size_t totalSize = size * nmemb;
        output->append(static_cast<char*>(contents), totalSize);
        return totalSize;
    }

public:
    WeatherApiClient(const string& apiKey)
        : apiKey(apiKey) {}

    string getWeatherData(const string& city) {
        string apiUrl = "http://api.weatherapi.com/v1/current.json?key=" + apiKey + "&q=" + city;

        CURL* curl = curl_easy_init();
        if (!curl) {
            cerr << "Error initializing cURL." << endl;
            return "";
        }

        curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());

        string response;

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallBack);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "cURL request failed: " << curl_easy_strerror(res) << std::endl;
            return "";
        }

        curl_easy_cleanup(curl);

        return response;
    }
};

class Wardrobe {
public:
    Wardrobe(const string& apiKey)
        : weatherApiClient(apiKey) {
        vector<string> defaultKeys = { "Shoes", "Bottoms", "Shirts", "Sweaters", "Outerwear", "Hat" };
        for (const auto& key : defaultKeys) { clothes[key] = vector<ClothingItem>(); }

        colorwheel["orange"] = { "green", "blue", "navy", "white", "black" };
        colorwheel["red"] = { "blue", "navy", "gray", "white", "black" };
        colorwheel["beige"] = { "navy", "brown", "white", "black" };
        colorwheel["green"] = { "orange", "brown", "white", "black" };
        colorwheel["blue"] = { "red", "orange", "white", "black" };
        colorwheel["navy"] = { "red", "gray", "white", "black" };
        colorwheel["brown"] = { "green", "white", "beige", "black" };
        colorwheel["gray"] = { "red", "navy", "black", "white" };
        colorwheel["black"] = { "red", "orange", "beige", "green", "blue", "navy", "brown", "gray", "white" };
        colorwheel["white"] = { "red", "orange", "beige", "green", "blue", "navy", "brown", "gray", "black" };
    }

    double getCurrentTemp(const string& city) {
        auto jsonData = json::parse(weatherApiClient.getWeatherData(city));
        return jsonData["current"]["temp_f"].get<double>();
    }

    int layerNum() {
        double currentTemp = getCurrentTemp("rochester-hills");

        if (currentTemp < 30.0) { return 2; }

        if (currentTemp > 30 && currentTemp < 55) { return 1; }
        return 0;
    }


    void addClothingItem(string itemNameIn, const string& itemTypeIn, string itemColorIn) {
        ClothingItem article;
        article.name = itemNameIn;
        article.type = itemTypeIn;
        article.color = itemColorIn;
        clothes[itemTypeIn].push_back(article);
    }

    void removeClothingItem(const string& itemName, const string& itemType, const string& itemColor) {
        auto& typeVector = clothes[itemType];
        auto it = std::remove_if(typeVector.begin(), typeVector.end(), [&](const ClothingItem& item) {
            return (item.name == itemName && item.color == itemColor);
        });

        if (it != typeVector.end()) {
            typeVector.erase(it, typeVector.end());
            cout << "Item removed: " << itemName << " (" << itemColor << ")\n";
        } else {
            cout << "Item not found: " << itemName << " (" << itemColor << ") in type: " << itemType << "\n";
        }
    }


    void outfitGenerator() {
        vector<ClothingItem> monochrome;
        vector<ClothingItem> complement = complementaryOutfit();

        bool sameColor = false;
        size_t counter = 0;


        while (!sameColor) {
            monochrome = monochromeOutfit();
            string colorMain = monochrome[0].color;
            for (auto& i : monochrome) {
                if (i.color == colorMain) { counter++; }
            }
            if (counter == monochrome.size()) { sameColor = true; }
        }


        cout << "Outfit 1: \n";
        for (auto& item : monochrome) { cout << "   " << item.color << " " << item.name << "\n"; }

        cout << "\n";

        cout << "Outfit 2: \n";
        for (auto& item : complement) { cout << "   " << item.color << " " << item.name << "\n"; }

        string word;
        cout << "Enter which outfit you would like to wear today (1 or 2) or press another key for none: \n";
        cin >> word;
        if (word == "1") {
            for (auto& i : monochrome) { updateLastWornTime(i.name, i.type, i.color); }
        } else if (word == "2") {
            for (auto& i : complement) { updateLastWornTime(i.name, i.type, i.color); }
        }

        // check if theres at least one article of clothing in everything
        //  monochrome, contrast, neutral
    }

    void updateLastWornTime(const string& itemName, const string& itemType, const string& itemColor) {
        auto& typeVector = clothes[itemType];
        for (auto& item : typeVector) {
            if (item.name == itemName && item.color == itemColor) {
                timespec ts;
                timespec_get(&ts, TIME_UTC);
                char buf[100];
                strftime(buf, sizeof buf, "%m/%d/%y", std::gmtime(&ts.tv_sec));
                string dateString(buf);
                item.lastWornTime = dateString;
                cout << "Last worn time updated for " << itemName << "\n";
                return;
            }
        }

        cout << "Item not found: " << itemName << " in type: " << itemType << "\n";
    }

    ClothingItem getRandomItem(const string& itemType) {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(0, clothes[itemType].size() - 1);
        int randomIndex = dis(gen);

        return clothes[itemType][randomIndex];
    }
    ClothingItem getOldestColor(const string& itemType, const string& color) {
        auto it = clothes.find(itemType);
        vector<ClothingItem> itemsWithColor;
        for (const auto& item : it->second) {
            if (item.color == color) { itemsWithColor.push_back(item); }
        }
        if (!itemsWithColor.empty()) {
            auto oldestItem = min_element(itemsWithColor.begin(), itemsWithColor.end(),
                                          [](const ClothingItem& item1, const ClothingItem& item2) {
                                              return item1.lastWornTime < item2.lastWornTime;
                                          });
            return *oldestItem;
        } else {
            cout << "No complementary bottom found, generating random.\n";
            return getRandomItem("itemType");
        }
    }

    string getComplementaryColor(string colorIn) {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(0, colorwheel[colorIn].size() - 1);
        int randomIndex = dis(gen);
        return colorwheel[colorIn][randomIndex];
    }

    // assumes there is clothing in the wardrobe
    vector<ClothingItem> monochromeOutfit() {
        vector<ClothingItem> outfit;

        int numLayers = layerNum();
        ClothingItem mainTop = getRandomItem("Shirts");
        outfit.push_back(mainTop);
        string mainColor = mainTop.color;
        ClothingItem bottom = getOldestColor("Bottoms", mainColor);
        outfit.push_back(bottom);

        if (numLayers == 1) {
            ClothingItem mainSweater = getOldestColor("Sweaters", mainColor);
            outfit.push_back(mainSweater);
            return outfit;
        }

        if (numLayers == 2) {
            ClothingItem mainJacket = getOldestColor("Outerwear", mainColor);
            outfit.push_back(mainJacket);
            return outfit;
        }

        return outfit;
    }

    // assumes there is clothing in the wardrobe
    vector<ClothingItem> complementaryOutfit() {
        vector<ClothingItem> outfit;
        int numLayers = layerNum();

        ClothingItem mainTop = getRandomItem("Shirts");
        outfit.push_back(mainTop);
        string mainColor = mainTop.color;
        string complementaryColor = getComplementaryColor(mainColor);
        ClothingItem bottom = getOldestColor("Bottoms", complementaryColor);
        outfit.push_back(bottom);

        if (numLayers == 1) {
            ClothingItem mainSweater = getOldestColor("Sweaters", mainColor);
            outfit.push_back(mainSweater);
            return outfit;
        }

        if (numLayers == 2) {
            ClothingItem mainJacket = getOldestColor("Outerwear", mainColor);
            outfit.push_back(mainJacket);
            return outfit;
        }

        return outfit;
    }

    void displayWardrobe() const {
        cout << "Wardrobe Contents:\n";

        for (const auto& entry : clothes) {
            const string& itemType = entry.first;
            const vector<ClothingItem>& items = entry.second;

            cout << "=== " << itemType << " ===\n";

            for (const auto& item : items) { cout << "- " << item.name << " (" << item.color << ")\n"; }

            cout << "\n";
        }
    }

    void saveWardrobeToFile(const string& filename) const {
        ofstream file(filename);

        if (file.is_open()) {
            for (const auto& entry : clothes) {
                const string& itemType = entry.first;
                const vector<ClothingItem>& items = entry.second;

                file << "===" << endl;
                file << itemType << endl;

                for (const auto& item : items) {
                    file << item.name << "," << item.color << "," << item.lastWornTimeToString() << endl;
                }
            }

            file.close();
            cout << "Wardrobe data saved to " << filename << endl;
        } else {
            cerr << "Unable to open file for saving." << endl;
        }
    }

    void loadWardrobeFromFile(const string& filename) {
        ifstream file(filename);

        if (file.is_open()) {
            string line;
            string itemType;

            while (getline(file, line)) {
                if (line == "===") {
                    if (getline(file, itemType)) {
                        continue;
                    } else {
                        break;
                    }
                }

                size_t pos1 = line.find(',');
                size_t pos2 = line.find(',', pos1 + 1);

                if (pos1 != string::npos && pos2 != string::npos) {
                    ClothingItem item;
                    item.name = line.substr(0, pos1);
                    item.color = line.substr(pos1 + 1, pos2 - pos1 - 1);
                    item.setLastWornTimeFromString(line.substr(pos2 + 1));
                    item.type = itemType;
                    clothes[itemType].push_back(item);
                }
            }

            file.close();
            cout << "Wardrobe data loaded from " << filename << endl;
        } else {
            cerr << "Unable to open file for loading." << endl;
        }
    }

private:
    unordered_map<string, vector<ClothingItem>> clothes;
    unordered_map<string, vector<string>> colorwheel;
    WeatherApiClient weatherApiClient;
};


int main() {
    cout << "hello";
    string apiKey = "4fad1a43ec27498fa9d223949232112";
    Wardrobe myWardrobe(apiKey);
    myWardrobe.loadWardrobeFromFile("wardrobedata.txt");
    char userInput;
    do {
        cout << "Options:\n";
        cout << "1. Add clothing item\n";
        cout << "2. Remove clothing item\n";
        cout << "3. Display wardrobe\n";
        cout << "4. Generate outfit\n";
        cout << "5. Update worn time for clothing item\n";
        cout << "q. Quit\n";
        cout << "Enter your choice: ";
        cin >> userInput;

        switch (userInput) {
        case '1': {
            string itemName;
            string itemType;
            string itemColor;
            cout << "Enter the type of the clothing item: (Bottoms, Shirts, Outerwear, Hat, Shoes, Sweaters)\n";
            cin >> itemType;
            if (itemType == "Bottoms" || itemType == "Shirts" || itemType == "Outerwear" || itemType == "Hat"
                || itemType == "Shoes" || itemType == "Sweaters") {
                cout << "Enter the name of the clothing item: ";
                cin >> itemName;
                cout << "Enter the main color of the clothing item: ";
                cin >> itemColor;
                myWardrobe.addClothingItem(itemName, itemType, itemColor);
                break;
            } else {
                cout << "Invalid type, try again. \n";
                break;
            }
        }
        case '2': {
            string itemName;
            string itemType;
            string itemColor;
            cout << "Enter the type of the clothing item: (Bottoms, Shirts, Outerwear, Hat, Shoes, Sweaters)\n";
            cin >> itemType;
            if (itemType == "Bottoms" || itemType == "Shirts" || itemType == "Outerwear" || itemType == "Hat"
                || itemType == "Shoes" || itemType == "Sweaters") {
                cout << "Enter the name of the clothing item: ";
                cin >> itemName;
                cout << "Enter the main color of the clothing item: ";
                cin >> itemColor;
                myWardrobe.removeClothingItem(itemName, itemType, itemColor);
                break;
            } else {
                cout << "Invalid type, try again. \n";
                break;
            }
        }
        case '3':
            myWardrobe.displayWardrobe();
            break;
        case '4': {
            string word;
            cout << "Three outfits will be generated: \n";
            myWardrobe.outfitGenerator();
            break;
        }
        case '5': {
            string itemName;
            string itemType;
            string itemColor;
            cout << "Enter the type of the clothing item: (Bottoms, Shirts, Outerwear, Hat, Shoes, Sweaters)\n";
            cin >> itemType;
            if (itemType == "Bottoms" || itemType == "Shirts" || itemType == "Outerwear" || itemType == "Hat"
                || itemType == "Shoes" || itemType == "Sweaters") {
                cout << "Enter the name of the clothing item: ";
                cin >> itemName;
                cout << "Enter the main color of the clothing item: ";
                cin >> itemColor;
                myWardrobe.updateLastWornTime(itemName, itemType, itemColor);
                break;
            } else {
                cout << "Invalid type, try again. \n";
                break;
            }
        }
        case 'q':
            cout << "Exiting the wardrobe interface.\n";
            break;
        default:
            cout << "Invalid choice. Try again.\n";
            break;
        }


    } while (userInput != 'q');

    myWardrobe.saveWardrobeToFile("wardrobedata.txt");

    return 0;
}

// new test test lol