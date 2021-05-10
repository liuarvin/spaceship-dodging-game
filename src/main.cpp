#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <ncurses.h>
#include <string>
#include <unistd.h>
#include <vector>
#include <fstream>

// Q16 - members in struct are public by default
struct Position_struct {
    Position_struct(int x, int y) : x(x), y(y) {}
    int x;
    int y;
};

// Q18 - Class template
template<class T>
class Position {
private:
    T x;
    T y;

public:
    Position(T x, T y) : x(x), y(y) {}
    T getX() {return x;}
    T getY() {return y;}
    void setX(T x) {this->x = x;}
    void setY(T y) {this->y = y;}

    // Q11 - Overloaded operators
    bool operator == (Position<T>);
    Position<T> operator + (Position<T>);
    Position<T> operator - (Position<T>);
};

template<class T>
bool Position<T>::operator == (Position<T> p) {
    return this->x == p.getX() && this->y == p.getY();
}

template<class T>
Position<T> Position<T>::operator + (Position<T> p) {
    return Position<T>(this->x + p.getX(), this->y + p.getY());
}

template<class T>
Position<T> Position<T>::operator - (Position<T> p) {
    return Position<T>(this->x - p.getX(), this->y - p.getY());
}

class Boundary {
private:
    int top;
    int bottom;
    int left;
    int right;

public:
    Boundary(int t, int b, int l, int r) : top(t), bottom(b), left(l), right(r) {}
    int getWidth() const;
    int getHeight() const;

    // for demonstrating const_cast
    void setTop(int t) {this->top = t;}

    bool isInside(Position<int>);
};

int Boundary::getWidth() const {
    return right - left;
}

int Boundary::getHeight() const {
    return bottom - top;
}

bool Boundary::isInside(Position<int> sprite_pos) {
    return sprite_pos.getX() >= left 
    && sprite_pos.getX() <= right
    && sprite_pos.getY() <= bottom
    && sprite_pos.getY() >= top;
}

// Q3 - Abstract class
class Sprite {
public:
    virtual void draw() = 0;
    virtual void clear() = 0;
    virtual Position<int> getPosition() = 0;
    virtual int getWidth() = 0;
    virtual int getHeight() = 0;
};

class Ship : public Sprite {
// Q1 - Correct use of access specifiers
private:
    std::vector<std::vector<char>> body;

protected:
    Position<int> pos;

public:
    Ship(std::vector<std::vector<char>> b, Position<int> p) : Sprite(), body(b), pos(p) {}

    // Q9 - Modified copy constructor
    Ship(const Ship &sourceShip);
    
    void draw();
    void clear();
    void moveLeft(int);
    void moveRight(int);
    Position<int> getPosition();
    int getWidth();
    int getHeight();
};

Ship::Ship(const Ship &sourceShip) : body(sourceShip.body), pos(0, 0) {
    std::cout << "In copy constructor" << std::endl;
}

// Q5 - Over-riding of a method from an abstract class
void Ship::draw() {
    for(unsigned int row=0; row<body.size(); row++) {
        for(unsigned int col=0; col<body[row].size(); col++) {
            mvaddch(pos.getY()+row, pos.getX()+col, body[row][col]);
        }
    }
}

void Ship::clear() {
    for(unsigned int row=0; row<body.size(); row++) {
        for(unsigned int col=0; col<body[row].size(); col++) {
            mvaddch(pos.getY()+row, pos.getX()+col, ' ');
        }
    }
}

Position<int> Ship::getPosition() {
    return pos;
}

void Ship::moveRight(int amount) {
    pos.setX(pos.getX() + amount);
}

void Ship::moveLeft(int amount) {
    pos.setX(pos.getX() - amount);
}

int Ship::getWidth() {
    return body.at(0).size();
}

int Ship::getHeight() {
    return body.size();
}

// Q6 - Multiple inheritance
class Rock : public Ship {
public:
    Rock(std::vector<std::vector<char>> b, Position<int> p) : Ship(b, p) {}
    void moveDown(int);
    virtual ~Rock();
};

void Rock::moveDown(int amount) {
    pos.setY(pos.getY() + amount);
}

// Q10 - Destructor
Rock::~Rock() {
    std::cout << "Rock destroyed" << std::endl;
}

class RockManager {
private:
    Boundary boundary;
    // Q19 - Use of vector container to store objects
    std::vector<Rock*> active_rocks;

public:
    RockManager(Boundary b) : boundary(b) {}
    void updateScreen();
    void removeRock(size_t);
    std::vector<Rock*> getRocks();
    Boundary getBoundary();
};

std::vector<Rock*> RockManager::getRocks() {
    return active_rocks;
}

Boundary RockManager::getBoundary() {
    return boundary;
}

void RockManager::removeRock(size_t i) {
    // Q14 - use of delete operator
    delete active_rocks.at(i);
    active_rocks.erase(active_rocks.begin() + i);
}

void drawRock(Rock* rock) {
    rock->draw();
}

void clearRock(Rock* rock) {
    rock->clear();
}

// Q8 - An object passing itself to a function (not part of a class), which modifies that object
void moveRockDown(RockManager* rock_manager, Rock* rock, int i) {
    if(rock_manager->getBoundary().isInside(rock->getPosition())) {
        rock->moveDown(1);
    } else {
        rock_manager->removeRock(i);
    }
}

void RockManager::updateScreen() {
    
    // Q20 - Algorithm on vector container
    std::for_each(active_rocks.begin(), active_rocks.end(), clearRock);

    for(size_t i=0; i<active_rocks.size(); i++) {
        moveRockDown(this, active_rocks.at(i), i);
    }

    std::vector<std::vector<char>> rock_body = {{'O'}};
    Position<int> rock_pos(rand() % boundary.getWidth(), 0);

    // Q14 - use of new operator
    active_rocks.push_back(new Rock(rock_body, rock_pos));

    std::for_each(active_rocks.begin(), active_rocks.end(), drawRock);
}

class Game {
private:
    // Q15 - Use of static states
    static int score;
    WINDOW* wnd;
    void displayScore();
    void displayScore(Boundary);

public:
	Game();
	void run();
	virtual ~Game();
};

Game::Game() {
    wnd = initscr();
    cbreak();
    noecho();
    clear();
    refresh();
    keypad(wnd, true);
    nodelay(wnd, true);
    curs_set(0);
}

int Game::score = 0;

void Game::displayScore() {
    std::string score_string = "SCORE: " + std::to_string(score);
    for(unsigned int i=0; i<score_string.size(); i++) {
        mvaddch(2, 5+i, ' ');
        mvaddch(2, 5+i, score_string[i]);
    }
}

// Q2 - Overloading the displayScore function
// Q17 - Passing object by const reference
void Game::displayScore(const Boundary game_area) {
    clear();
    
    std::string score_string = "SCORE: " + std::to_string(score);
    
    // Q12 - static_cast
    float x = game_area.getWidth()/2;
    float y = game_area.getHeight()/2;

    int x_pos = static_cast<int>(x);
    int y_pos = static_cast<int>(y);

    for(unsigned int i=0; i<score_string.size(); i++) {
        mvaddch(y_pos, x_pos+i, ' ');
        mvaddch(y_pos, x_pos+i, score_string[i]);
    }

    // Q12 - const_cast
    // can't do this because const
    // game_area.setTop(5);

    // cast away const
    Boundary *new_game_area = const_cast<Boundary*>(&game_area);
    new_game_area->setTop(5);

    refresh();
    while(1);
}

void Game::run() {
    int bottom, right;
    getmaxyx(wnd, bottom, right);

    Boundary game_area(0, bottom, 0, right);
    RockManager rock_manager(game_area);

    Position<int> ship_pos(right/2, bottom-10);
    std::vector<std::vector<char>> ship_body = {
        {'*'},
    };
    
    Ship ship(ship_body, ship_pos);

    // Q12 - dynamic_cast
    std::vector<std::vector<char>> rock_body = {{'-'}};
    Position<int> rock_pos(0, 0);
    Ship *dynamic_rock = new Rock(rock_body, rock_pos);
    Rock *ship_to_rock = dynamic_cast<Rock*>(dynamic_rock);
    std::cout << "x_pos = " << ship_to_rock->getPosition().getX() << ", y_pos = " << ship_to_rock->getPosition().getY() << std::endl;

    while(1) {
        bool exit = false;
        int keyboard_in = wgetch(wnd);

        if(score % 10 == 0) {
            rock_manager.updateScreen();
            displayScore();
        }

        ship.clear();

        switch(keyboard_in) {
            case 'q':
            exit = true;
            break;
            case KEY_LEFT:
            ship.moveLeft(2);
            break;
            case KEY_RIGHT:
            ship.moveRight(2);
            break;
            default:
            break;
        }

        ship.draw();

        // Q4 - Operations on pointers to an object
        std::vector<Rock*> rocks = rock_manager.getRocks();
        
        for(size_t i=0; i<rocks.size(); i++) {
            for(int j=0; j<ship.getWidth(); j++) {
                for(int k=0; k<ship.getHeight(); k++) {
                    Position<int> offset(j, k);

                    // Q11 - Overloaded operators + and ==
                    if(rocks.at(i)->getPosition() == ship.getPosition() + offset) {
                        exit = true;
                    }
                }
            }
        }

        refresh();

        if(exit) displayScore(game_area);

        score++;
        usleep(10000);
    }
}

Game::~Game() {
    endwin();
}

void Q9PassByValue(Ship ship) {
    std::cout << "ship passed by value" << std::endl;
}

void Q9PassByReference(Ship &ship) {
    std::cout << "ship passed by reference" << std::endl;
}

int main() {
    std::ofstream out("stdout.txt");
    std::cout.rdbuf(out.rdbuf());

    Position<int> pos(5, 5);
    Position_struct pos_struct(0, 0);

    // Q16 - Difference between class and struct
    pos_struct.x = 10;
    pos_struct.y = 10;

    std::cout << "x_pos = " << pos.getX() << ", y_pos = " << pos.getY() << std::endl;
    std::cout << "struct x_pos = " << pos_struct.x << ", struct y_pos = " << pos_struct.y << std::endl;

    // Q12 - reinterpret_cast
    Position<int> *pos_struct_to_pos_class = reinterpret_cast<Position<int>*>(&pos_struct);
    
    std::cout << "pos_struct_to_pos_class_x = " << pos_struct_to_pos_class->getX() << ", pos_struct_to_pos_class_y = " << pos_struct_to_pos_class->getY() << std::endl;

    Position<int> ship_pos(10, 10);
    std::vector<std::vector<char>> ship_body = {{'X'}};
    Ship test_ship(ship_body, ship_pos);

    // Q9
    Q9PassByValue(test_ship);
    Q9PassByReference(test_ship);

    Game game;
    game.run();
    return 0;
}