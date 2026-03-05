#include <iostream>
#include <string>
#include <unordered_map>
#include "txtrotw.h"

// Main takes first parameter as treasure class name
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <item code>" << std::endl;
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        std::string itemCode = argv[i];
    
        std::unordered_map<std::string, D2::ROTW::t_armor> armor;
        std::unordered_map<std::string, D2::ROTW::t_weapons> weapons;
        std::unordered_map<std::string, D2::ROTW::t_misc> misc;
    
        for (auto &entry : D2::ROTW::t_armor::readfile("txt/armor.txt")) {
            armor[entry.code] = entry;
        }
    
        for (auto &entry : D2::ROTW::t_weapons::readfile("txt/weapons.txt")) {
            weapons[entry.code] = entry;
        }
    
        for (auto &entry : D2::ROTW::t_misc::readfile("txt/misc.txt")) {
            misc[entry.code] = entry;
        }
    
        if (armor.find(itemCode) != armor.end()) {
            std::cout << "Found armor: " << armor[itemCode].name << std::endl;
            std::cout << "Level: " << armor[itemCode].level << std::endl;
    
            if (armor[itemCode].levelreq) {
                std::cout << "Level Requirement: " << armor[itemCode].levelreq << std::endl;
            }
    
            std::cout << "AC: " << armor[itemCode].minac << "-" << armor[itemCode].maxac << std::endl;
        } else if (weapons.find(itemCode) != weapons.end()) {
            std::cout << "Found weapon: " << weapons[itemCode].name << std::endl;
            std::cout << "Level: " << weapons[itemCode].level << std::endl;
    
            if (weapons[itemCode].levelreq) {
                std::cout << "Level Requirement: " << weapons[itemCode].levelreq << std::endl;
            }
    
            if (weapons[itemCode].mindam || weapons[itemCode].maxdam) {
                std::cout << "Damage: " << weapons[itemCode].mindam << "-" << weapons[itemCode].maxdam << std::endl;
            }
    
            if (weapons[itemCode]._2handmindam || weapons[itemCode]._2handmaxdam) {
                std::cout << "2h Damage: " << weapons[itemCode]._2handmindam << "-" << weapons[itemCode]._2handmaxdam << std::endl;
            }
    
            std::cout << "Speed: " << weapons[itemCode].speed << std::endl;
        } else if (misc.find(itemCode) != misc.end()) {
            std::cout << "Found misc item: " << misc[itemCode].name << std::endl;
            std::cout << "Level: " << misc[itemCode].level << std::endl;
    
            if (misc[itemCode].levelreq) {
                std::cout << "Level Requirement: " << misc[itemCode].levelreq << std::endl;
            }
    
            if (misc[itemCode].stackable) {
                std::cout << "Stack Size: " << misc[itemCode].minstack << "-" << misc[itemCode].maxstack << std::endl;
            }
        } else {
            std::cout << "Item code not found: " << itemCode << std::endl;
        }

        if (i < argc - 1) {
            std::cout << std::endl;
        }
    }
}