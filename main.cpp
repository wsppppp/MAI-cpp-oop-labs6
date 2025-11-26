#include "./include/factory/NPCFactory.h"
#include "./include/npc/NPC.h"
#include "./include/visitor/battleVisitor.h"
#include "./include/observer/battleLogger.h"
#include "./include/observer/consoleLogger.h"
#include "./include/observer/fileLogger.h"
#include <memory>
#include <vector>
#include <iostream>
#include <limits>
#include <algorithm>
#include <cmath> // Добавлено для вычисления расстояния

class InteractiveSimulator {
private:
    NPCFactory factory;
    std::vector<std::unique_ptr<INPC>> activeNPCs;
    std::shared_ptr<BattleLogger> battleLogger;
    int attackRange; // Глобальная дальность боя

public:
    InteractiveSimulator() : attackRange(10) {
        // Инициализация системы логирования
        battleLogger = std::make_shared<BattleLogger>();
        battleLogger->attach(std::make_shared<ConsoleLogger>());
        
        // Пробуем создать файл лога, если папка data существует
        try {
            battleLogger->attach(std::make_shared<FileLogger>("../data/battle_log.txt"));
        } catch (...) {
            std::cerr << "Предупреждение: Не удалось создать файл лога (проверьте папку data)" << std::endl;
        }
    }

    void createNPC() {
        std::string name, type;
        int x, y, range;

        std::cout << "\n=== СОЗДАНИЕ ПЕРСОНАЖА ===" << std::endl;
        
        std::cout << "Введите имя: ";
        std::cin.ignore();
        std::getline(std::cin, name);

        std::cout << "Доступные типы: Эльф, Дракон, Друид" << std::endl;
        std::cout << "Введите тип: ";
        std::cin >> type;

        if (type != "Эльф" && type != "Дракон" && type != "Друид") {
            std::cout << "Ошибка: неверный тип персонажа!" << std::endl;
            return;
        }

        std::cout << "Введите координату X (0-500): ";
        std::cin >> x;
        std::cout << "Введите координату Y (0-500): ";
        std::cin >> y;
        std::cout << "Введите индивидуальную дальность (для справки): ";
        std::cin >> range;

        try {
            auto npc = factory.createNPC(name, type, x, y, range);
            activeNPCs.push_back(std::move(npc));
            std::cout << "Персонаж создан успешно!" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Ошибка создания персонажа: " << e.what() << std::endl;
        }
    }

    void displayActiveNPCs() {
        std::cout << "\n=== АКТИВНЫЕ ПЕРСОНАЖИ ===" << std::endl;
        
        if (activeNPCs.empty()) {
            std::cout << "Нет активных персонажей." << std::endl;
            return;
        }

        std::cout << "Тип | Имя | Координаты | Статус" << std::endl;
        std::cout << "--------------------------------------------------" << std::endl;
        
        for (const auto& npc : activeNPCs) {
            const Point& pos = npc->getPosition();
            std::cout << npc->getType() << " | " 
                    << npc->getName() << " | " 
                    << "(" << pos.getX() << ", " << pos.getY() << ") | "
                    << (npc->isAlive() ? "ЖИВ" : "МЕРТВ") << std::endl;
        }
    }

    void saveToFile() {
        std::string filename;
        std::cout << "\n=== СОХРАНЕНИЕ В ФАЙЛ ===" << std::endl;
        std::cout << "Введите имя файла (например: dungeon.txt): ";
        std::cin >> filename;

        try {
            factory.saveToFile("../data/" + filename, activeNPCs);
            std::cout << "Персонажи успешно сохранены в файл: ../data/" << filename << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Ошибка сохранения: " << e.what() << std::endl;
        }
    }

    void loadFromFile() {
        std::string filename;
        std::cout << "\n=== ЗАГРУЗКА ИЗ ФАЙЛА ===" << std::endl;
        std::cout << "Введите имя файла (например: dungeon.txt): ";
        std::cin >> filename;

        try {
            auto loadedNPCs = factory.loadFromFile("../data/" + filename);
            for (auto& npc : loadedNPCs) {
                activeNPCs.push_back(std::move(npc));
            }
            std::cout << "Успешно загружено " << loadedNPCs.size() << " персонажей." << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Ошибка загрузки: " << e.what() << std::endl;
        }
    }

    // --- ИСПРАВЛЕННАЯ ЛОГИКА БОЯ ---
    void conductBattle() {
        std::cout << "\n=== ПРОВЕДЕНИЕ СРАЖЕНИЯ ===" << std::endl;
        std::cout << "Текущая глобальная дальность боя: " << attackRange << std::endl;
        
        if (activeNPCs.empty()) {
            std::cout << "Нет активных персонажей для боя." << std::endl;
            return;
        }

        battleLogger->logBattleStart(activeNPCs.size());
        bool battleOccurred = false;
        
        for (size_t i = 0; i < activeNPCs.size(); ++i) {
            for (size_t j = 0; j < activeNPCs.size(); ++j) {
                if (i == j) continue;
                
                auto& attacker = activeNPCs[i];
                auto& defender = activeNPCs[j];
                
                if (!attacker->isAlive() || !defender->isAlive()) continue;
                
                // ИСПРАВЛЕНИЕ: Вычисляем расстояние вручную
                double dist = std::sqrt(std::pow(attacker->getPosition().getX() - defender->getPosition().getX(), 2) +
                                      std::pow(attacker->getPosition().getY() - defender->getPosition().getY(), 2));

                // ИСПРАВЛЕНИЕ: Сравниваем с глобальной переменной attackRange
                if (dist <= static_cast<double>(attackRange)) {
                    
                    // Используем Visitor для проверки "Кто кого бьет" (Камень-Ножницы-Бумага)
                    BattleVisitor visitor(attacker.get(), defender.get(), battleLogger);
                    attacker->accept(visitor);
                    
                    if (visitor.didFightOccur()) {
                        battleOccurred = true;
                        visitor.getLoser()->markDead();
                        // Сообщение о победе выводится через Logger внутри Visitor
                    }
                }
            }
        }

        if (!battleOccurred) {
            std::cout << "Боев не произошло (никто не подошел по дистанции или типу)." << std::endl;
        }

        int survivors = 0;
        for (const auto& npc : activeNPCs) {
            if (npc->isAlive()) survivors++;
        }
        
        battleLogger->logBattleEnd(survivors);
        std::cout << "Сражение завершено. Выжило: " << survivors << " из " << activeNPCs.size() << std::endl;
    }
    // --------------------------------

    void setAttackRange() {
        std::cout << "\n=== УСТАНОВКА ДАЛЬНОСТИ АТАКИ ===" << std::endl;
        std::cout << "Текущая дальность атаки: " << attackRange << std::endl;
        std::cout << "Введите новую дальность атаки: ";
        std::cin >> attackRange;
        std::cout << "Дальность атаки установлена: " << attackRange << std::endl;
    }

    void clearDeadNPCs() {
        std::cout << "\n=== ОЧИСТКА МЕРТВЫХ ПЕРСОНАЖЕЙ ===" << std::endl;
        size_t initialSize = activeNPCs.size();
        activeNPCs.erase(
            std::remove_if(activeNPCs.begin(), activeNPCs.end(),
                [](const std::unique_ptr<INPC>& npc) {
                    return !npc->isAlive();
                }),
            activeNPCs.end()
        );
        std::cout << "Удалено мертвых персонажей: " << initialSize - activeNPCs.size() << std::endl;
    }

    void run() {
        int choice;
        do {
            std::cout << "\n=== СИМУЛЯТОР ПОДЗЕМЕЛЬЯ ===" << std::endl;
            std::cout << "1. Создать персонажа" << std::endl;
            std::cout << "2. Показать активных персонажей" << std::endl;
            std::cout << "3. Сохранить персонажей в файл" << std::endl;
            std::cout << "4. Загрузить персонажей из файла" << std::endl;
            std::cout << "5. Провести сражение (Дальность: " << attackRange << ")" << std::endl;
            std::cout << "6. Установить дальность атаки" << std::endl;
            std::cout << "7. Очистить мертвых персонажей" << std::endl;
            std::cout << "0. Выход" << std::endl;
            std::cout << "Выберите действие: ";
            
            std::cin >> choice;
            
            switch (choice) {
                case 1: createNPC(); break;
                case 2: displayActiveNPCs(); break;
                case 3: saveToFile(); break;
                case 4: loadFromFile(); break;
                case 5: conductBattle(); break;
                case 6: setAttackRange(); break;
                case 7: clearDeadNPCs(); break;
                case 0: std::cout << "Выход..." << std::endl; break;
                default: std::cout << "Неверный выбор!" << std::endl; break;
            }
            
            if (std::cin.fail()) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }
            
        } while (choice != 0);
    }
};

int main() {
    std::cout << "=== ИНТЕРАКТИВНЫЙ СИМУЛЯТОР ПОДЗЕМЕЛЬЯ ===" << std::endl;
    try {
        InteractiveSimulator simulator;
        simulator.run();
    } catch (const std::exception& e) {
        std::cerr << "Критическая ошибка: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}