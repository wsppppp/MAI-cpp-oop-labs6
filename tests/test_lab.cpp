#include <gtest/gtest.h>
#include <memory>
#include <fstream>
#include <filesystem>
#include <cmath>

// Подключаем все необходимые хедеры
#include "../include/geom/point.h"
#include "../include/npc/elf.h"
#include "../include/npc/dragon.h"
#include "../include/npc/druid.h"
#include "../include/factory/NPCFactory.h"
#include "../include/visitor/battleVisitor.h"
#include "../include/observer/battleLogger.h"
#include "../include/observer/fileLogger.h"

// --- ТЕСТЫ ГЕОМЕТРИИ (Point) ---
TEST(GeomTest, ConstructorAndGetters) {
    Point p(10, 20);
    EXPECT_EQ(p.getX(), 10);
    EXPECT_EQ(p.getY(), 20);
}

TEST(GeomTest, DistanceCalculation) {
    Point p1(0, 0);
    Point p2(3, 4);
    double dist = std::sqrt(std::pow(p1.getX() - p2.getX(), 2) + 
                          std::pow(p1.getY() - p2.getY(), 2));
    EXPECT_DOUBLE_EQ(dist, 5.0);
}

// --- ТЕСТЫ ПЕРСОНАЖЕЙ (NPC) ---
// ИСПРАВЛЕНИЕ: Передаем Point в конструктор
TEST(NPCTest, ElfProperties) {
    Elf elf("Legolas", Point(10, 20), 100);
    EXPECT_EQ(elf.getName(), "Legolas");
    EXPECT_EQ(elf.getType(), "Эльф");
    EXPECT_EQ(elf.getPosition().getX(), 10);
    EXPECT_TRUE(elf.isAlive());
}

TEST(NPCTest, DragonProperties) {
    Dragon d("Smaug", Point(50, 50), 200);
    EXPECT_EQ(d.getType(), "Дракон");
}

TEST(NPCTest, DruidProperties) {
    Druid dr("Malfurion", Point(0, 0), 50);
    EXPECT_EQ(dr.getType(), "Друид");
}

TEST(NPCTest, DeathStatus) {
    Elf elf("DeadElf", Point(0, 0), 10);
    elf.markDead();
    EXPECT_FALSE(elf.isAlive());
}

// --- ТЕСТЫ ФАБРИКИ (Factory) ---
TEST(FactoryTest, CreateValidNPCs) {
    NPCFactory factory;
    
    auto npc1 = factory.createNPC("Name1", "Эльф", 0, 0, 10);
    EXPECT_NE(npc1, nullptr);
    EXPECT_EQ(npc1->getType(), "Эльф");

    auto npc2 = factory.createNPC("Name2", "Дракон", 0, 0, 10);
    EXPECT_EQ(npc2->getType(), "Дракон");
}

TEST(FactoryTest, CreateInvalidType) {
    NPCFactory factory;
    EXPECT_THROW(factory.createNPC("Name", "Orc", 0, 0, 10), std::exception);
}

TEST(FactoryTest, SaveAndLoad) {
    NPCFactory factory;
    std::string filename = "../data/test_save.txt";
    
    std::vector<std::unique_ptr<INPC>> originalList;
    originalList.push_back(factory.createNPC("SavedElf", "Эльф", 10, 10, 100));
    originalList.push_back(factory.createNPC("SavedDragon", "Дракон", 20, 20, 200));

    // Убедимся, что папка data существует
    std::filesystem::create_directories("../data");

    ASSERT_NO_THROW(factory.saveToFile(filename, originalList));

    std::vector<std::unique_ptr<INPC>> loadedList;
    ASSERT_NO_THROW(loadedList = factory.loadFromFile(filename));

    ASSERT_EQ(loadedList.size(), 2);
    EXPECT_EQ(loadedList[0]->getName(), "SavedElf");
}

// --- ТЕСТЫ БОЕВОЙ СИСТЕМЫ (Visitor) ---
// ИСПРАВЛЕНИЕ: Передаем Point в конструктор
TEST(BattleVisitorTest, ElfBeatsDruid) {
    Elf attacker("Attacker", Point(0, 0), 10);
    Druid defender("Defender", Point(0, 0), 10);
    
    BattleVisitor v(&attacker, &defender, nullptr);
    attacker.accept(v);
    
    EXPECT_TRUE(v.didFightOccur());
    EXPECT_EQ(v.getWinner(), &attacker);
}

TEST(BattleVisitorTest, DruidBeatsDragon) {
    Druid attacker("Attacker", Point(0, 0), 10);
    Dragon defender("Defender", Point(0, 0), 10);
    
    BattleVisitor v(&attacker, &defender, nullptr);
    attacker.accept(v);
    
    EXPECT_TRUE(v.didFightOccur());
    EXPECT_EQ(v.getWinner(), &attacker);
}

TEST(BattleVisitorTest, DragonBeatsElf) {
    Dragon attacker("Attacker", Point(0, 0), 10);
    Elf defender("Defender", Point(0, 0), 10);
    
    BattleVisitor v(&attacker, &defender, nullptr);
    attacker.accept(v);
    
    EXPECT_TRUE(v.didFightOccur());
    EXPECT_EQ(v.getWinner(), &attacker);
}

TEST(BattleVisitorTest, SameTypeNoFight) {
    Elf e1("E1", Point(0, 0), 10);
    Elf e2("E2", Point(0, 0), 10);
    
    BattleVisitor v(&e1, &e2, nullptr);
    e1.accept(v);
    
    EXPECT_FALSE(v.didFightOccur());
}

TEST(BattleVisitorTest, WrongOrderNoFight) {
    Elf attacker("Elf", Point(0, 0), 10);
    Dragon defender("Dragon", Point(0, 0), 10);
    
    BattleVisitor v(&attacker, &defender, nullptr);
    attacker.accept(v);
    
    EXPECT_FALSE(v.didFightOccur());
}

// --- ТЕСТЫ LOGGING (Observer) ---
TEST(ObserverTest, FileLogging) {
    std::string logFile = "../data/test_log.txt";
    std::filesystem::create_directories("../data");

    if (std::filesystem::exists(logFile)) {
        std::filesystem::remove(logFile);
    }

    auto logger = std::make_shared<BattleLogger>();
    auto fileObs = std::make_shared<FileLogger>(logFile);
    logger->attach(fileObs);

    Elf winner("Winner", Point(0, 0), 10);
    Druid loser("Loser", Point(0, 0), 10);
    
    logger->logKill(&winner, &loser);

    EXPECT_TRUE(std::filesystem::exists(logFile));
}

// --- ТЕСТЫ НОВОЙ ЛОГИКИ ДИСТАНЦИИ ---
TEST(RangeLogicTest, InsideRange) {
    Point p1(0, 0);
    Point p2(10, 0);
    
    double dist = std::sqrt(std::pow(p1.getX() - p2.getX(), 2) + 
                          std::pow(p1.getY() - p2.getY(), 2));
    
    EXPECT_TRUE(dist <= 10.0);
    EXPECT_TRUE(dist <= 15.0);
}