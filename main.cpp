#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <cmath>
#include <algorithm>
#include <string>

using namespace std;
using namespace sf;

RectangleShape makeRect(Vector2f size, Vector2f origin01, Color fill) {
    RectangleShape r(size);
    r.setFillColor(fill);
    r.setOrigin(size.x * origin01.x, size.y * origin01.y);
    return r;
}

// ---------------------- Налаштування гри ----------------------
struct Config {
    Vector2u windowSize{1280, 720};
    string windowTitle = "Night Racer";
    Color background = Color(50, 20, 30);
    RectangleShape playerShape = makeRect({200.f, 120.f}, {0.2f, 0.8f}, Color(80, 170, 220));
    Color playerColor = Color(220, 90, 90);
    float angelSpeed = 250.f;
    float moveSpeed = 600.f;
    float acceleration = 100.f;
    Keyboard::Key pauseKey = Keyboard::P;
    Keyboard::Key exitKey = Keyboard::Escape;
};

// ---------------------- Стан вводу ----------------------
struct InputState {
    bool up = false, down = false, left = false, right = false;
};

// ---------------------- Основний клас гри ----------------------
class Game {
public:
    Game(const Config& cfg)
    : cfg(cfg),
      window(VideoMode(cfg.windowSize.x, cfg.windowSize.y), cfg.windowTitle, Style::Titlebar | Style::Close),
      player(cfg.playerShape)
    {
        window.setVerticalSyncEnabled(true);
        player.setFillColor(cfg.playerColor);
        player.setPosition(400.f, 300.f);
        player.setRotation(0.f);
        player.setOrigin(cfg.playerShape.getSize().x,cfg.playerShape.getSize().y);
        player.setPosition(static_cast<float>(cfg.windowSize.x) / 2.f,
                           static_cast<float>(cfg.windowSize.y) / 2.f);
    }

    void run() {
        float speed = 0.f;
        float angel = 0.f;
        float acceleration = 0.f;

        while (window.isOpen()) {
            processEvents();

            float dt = clock.restart().asSeconds();
            if (dt > 0.1f) dt = 0.1f;

            if (!paused && hasFocus) {
                update(&dt, &speed, &angel, &acceleration);
            }

            render();
        }
    }

private:
    void processEvents() {
        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed) {
                window.close();
            } else if (e.type == Event::KeyPressed) {
                if (e.key.code == cfg.exitKey) {
                    window.close();
                } else if (e.key.code == cfg.pauseKey) {
                    paused = !paused;
                    clock.restart();
                }
            } else if (e.type == Event::LostFocus) {
                hasFocus = false;
            } else if (e.type == Event::GainedFocus) {
                hasFocus = true;
                clock.restart();
            }
        }

        input.up    = Keyboard::isKeyPressed(Keyboard::W);
        input.down  = Keyboard::isKeyPressed(Keyboard::S);
        input.left  = Keyboard::isKeyPressed(Keyboard::A);
        input.right = Keyboard::isKeyPressed(Keyboard::D);
    }


    void update(const float * dt, float * speed, float * acceleration, float * angel) {
        Vector2f dir(0.f, 0.f);
        float isSpeed = 0.f;
        float isAngel = 0.f;
        if (input.up)    isSpeed += 1.f;
        if (input.down)  isSpeed -= 1.f;
        if (input.left)  isAngel -= 1.f;
        if (input.right) isAngel += 1.f;

        // if (isSpeed != 0.f || isAngel != 0.f) {
            float dan = 30 * *dt;
            *angel += dan * isAngel;
            *angel = *angel > cfg.angelSpeed ? cfg.angelSpeed : *angel < -cfg.angelSpeed ? -cfg.angelSpeed : *angel; //limiting max angel speed
            float da = cfg.acceleration * *dt;
            *speed += da * isSpeed;
            *speed = *speed > cfg.moveSpeed ? cfg.moveSpeed : *speed < -cfg.moveSpeed ? -cfg.moveSpeed : *speed; //limiting max speed

            printf("speed: %f, angel: %f\n", *speed, *angel);


            dir.x = cos(*angel);
            dir.y = sin(*angel);

            Vector2f pos = player.getPosition();
            pos += dir * *speed;

            Vector2f ps =  cfg.playerShape.getSize();
            pos.x = max(ps.x, min(pos.x, (float)cfg.windowSize.x + ps.x));
            pos.y = max(ps.y, min(pos.y, (float)cfg.windowSize.y + ps.y));

            player.setPosition(pos);
            player.setRotation(*angel);
        // }
    }

    void render() {
        window.clear(cfg.background);
        window.draw(player);
        window.display();
    }

private:
    Config cfg;
    RenderWindow window;
    RectangleShape player;
    InputState input;
    bool paused = false;
    bool hasFocus = true;
    Clock clock;
};

int main() {
    Config cfg;
    Game game(cfg);
    game.run();
    return 0;
}
