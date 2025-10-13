#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <cmath>
#include <algorithm>
#include <string>

using namespace std;
using namespace sf;

// --------------------- Створення "Машинки" ------------------
RectangleShape makeRect(Vector2f size, Vector2f origin01, Color fill) {
    RectangleShape r(size);
    r.setFillColor(fill);
    r.setOrigin(size.x * origin01.x, size.y * origin01.y);
    return r;
}

// ---------------------- Налаштування гри ----------------------
struct Config {
    Vector2u windowSize{1800, 1000};
    string windowTitle = "Night Racer";
    Color background = Color(50, 20, 30);
    RectangleShape playerShape = makeRect({160.f, 60.f}, {0.2f, 0.8f}, Color(80, 170, 220));
    Color playerColor = Color(220, 90, 90);
    float angelSpeed = 150.f;
    float moveSpeed = 400.f;
    float acceleration = 250.f;
    Keyboard::Key pauseKey = Keyboard::P;
    Keyboard::Key exitKey = Keyboard::Escape;
};

inline void validateConfig(const Config& c) {
    if (c.windowSize.x == 0 || c.windowSize.y == 0) throw runtime_error("Window size is zero");
    if (c.moveSpeed < 0.f || c.acceleration < 0.f || c.angelSpeed < 0.f) throw runtime_error("Negative speeds/acceleration");
}

// ---------------------- Стан вводу ----------------------
struct InputState {
    bool up = false, down = false, left = false, right = false, space = false;
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
        player.setOrigin(cfg.playerShape.getSize().x*0.2f,cfg.playerShape.getSize().y/2);
        player.setPosition(static_cast<float>(cfg.windowSize.x) / 2.f,
                           static_cast<float>(cfg.windowSize.y) / 2.f);
    }

    void run() {
        float speed = 0.f;
        float angel = 0.f;
        float angelWheel = 0.f;
        float acceleration = 0.f;
        float timer = 0.f;
        int frames = 0;

        while (window.isOpen()) {
            processEvents();

            float dt = clock.restart().asSeconds();
            if (dt > 0.1f) dt = 0.1f;
            timer += dt;
            frames++;
            if (timer >= 1) {
                printf("FPS: %i\n", frames);
                timer = 0.f;
                frames = 0;
            }

            if (!paused && hasFocus) {
                update(&dt, &speed, &angelWheel, &angel);
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
        input.space = Keyboard::isKeyPressed(Keyboard::Space);
    }

    void update(const float * dt, float * speed, float * angelWheel, float * angel) {
        Vector2f dir(0.f, 0.f);
        float isAcceleration = 0.f;
        float isAngel = 0.f;
        float isHandBrake = 0.f;
        if (input.up)    isAcceleration += 1.f;
        if (input.down)  isAcceleration -= 1.f;
        if (input.left)  isAngel -= 1.f;
        if (input.right) isAngel += 1.f;
        if (input.space) isHandBrake -= 1.f;

        float dan = 180 * *dt;
        *angelWheel += dan * isAngel;
        *angelWheel = *angelWheel > cfg.angelSpeed ? cfg.angelSpeed : *angelWheel < -cfg.angelSpeed ? -cfg.angelSpeed : *angelWheel; //limiting max angel speed
        float da = cfg.acceleration * *dt;
        *speed += da * isAcceleration;
        *speed = *speed > cfg.moveSpeed ? cfg.moveSpeed : *speed < -cfg.moveSpeed ? -cfg.moveSpeed : *speed; //limiting max speed

        dir.x = cos((*angel + atan(30/110) * isAngel) * *dt);
        dir.y = sin((*angel + atan(30/110) * isAngel) * *dt);

        Vector2f pos = player.getPosition();
        pos += dir * *speed * *dt;

        *angel += (*speed/cfg.moveSpeed * *angelWheel * *dt);
        *angel -= *angel > 360.f ? 360.f : *angel < 0.f ? -360 : 0;

        Vector2f ps =  cfg.playerShape.getSize();
        pos.x = max(ps.x, min(pos.x, (float)cfg.windowSize.x + ps.x));
        pos.y = max(ps.y, min(pos.y, (float)cfg.windowSize.y + ps.y));

        player.setPosition(pos);
        player.setRotation(*angel);

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
    try {
        Config cfg;
        validateConfig(cfg);
        Game game(cfg);
        game.run();
        return 0;
    } catch (const exception& e) {
        fprintf(stderr, "Fatal error: %s\n", e.what());
        return 1;
    } catch (...) {
        fprintf(stderr, "Fatal error: unknown exception\n");
        return 2;
    }
}