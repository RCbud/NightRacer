#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <cmath>
#include <algorithm>
#include <string>

using namespace std;
using namespace sf;

// ---------------------- Налаштування гри ----------------------
struct Config {
    Vector2u windowSize{1280, 720};
    string windowTitle = "Night Racer";
    Color background = Color(50, 20, 30);
    float playerRadius = 20.f;
    Color playerColor = Color(220, 90, 90);
    float moveSpeed = 320.f;
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
      player(cfg.playerRadius)
    {
        window.setVerticalSyncEnabled(true);
        player.setFillColor(cfg.playerColor);
        player.setOrigin(cfg.playerRadius, cfg.playerRadius);
        player.setPosition(static_cast<float>(cfg.windowSize.x) / 2.f,
                           static_cast<float>(cfg.windowSize.y) / 2.f);
    }

    void run() {
        while (window.isOpen()) {
            processEvents();

            float dt = clock.restart().asSeconds();
            if (dt > 0.1f) dt = 0.1f;

            if (!paused && hasFocus) {
                update(dt);
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

    void update(float dt) {
        Vector2f dir(0.f, 0.f);
        if (input.up)    dir.y -= 1.f;
        if (input.down)  dir.y += 1.f;
        if (input.left)  dir.x -= 1.f;
        if (input.right) dir.x += 1.f;

        if (dir.x != 0.f || dir.y != 0.f) {
            float len = sqrt(dir.x * dir.x + dir.y * dir.y);
            dir.x /= len;
            dir.y /= len;

            Vector2f pos = player.getPosition();
            pos += dir * cfg.moveSpeed * dt;

            float r = cfg.playerRadius;
            pos.x = max(r, min(pos.x, (float)cfg.windowSize.x - r));
            pos.y = max(r, min(pos.y, (float)cfg.windowSize.y - r));

            player.setPosition(pos);
        }
    }

    void render() {
        window.clear(cfg.background);
        window.draw(player);
        window.display();
    }

private:
    Config cfg;
    RenderWindow window;
    CircleShape player;
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
