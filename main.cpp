#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <cmath>
#include <algorithm>
#include <string>
#include <filesystem>

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
    float textureAngleOffset = 90.f;

    float angelSpeed = 520.f;
    float moveSpeed = 3600.f;
    float acceleration = 450.f;
    float carScale = 0.7f;

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
      // window(VideoMode(cfg.windowSize.x, cfg.windowSize.y), cfg.windowTitle, Style::Titlebar | Style::Close),
        window(sf::VideoMode::getDesktopMode(), cfg.windowTitle, sf::Style::Fullscreen),
        car(cfg.playerShape)
    {
        window.setVerticalSyncEnabled(true);
        car.setFillColor(cfg.playerColor);
        car.setPosition(400.f, 300.f);

        car.setRotation(0.f);
        car.setPosition(static_cast<float>(cfg.windowSize.x) / 2.f,
                           static_cast<float>(cfg.windowSize.y) / 2.f);

        // texturing car
        string path = (filesystem::current_path().parent_path() / "textures/cars/Ferrari_FXX_2005_(Amalgam_Models).png").string();
        if (path.empty()) throw runtime_error("Cannot find texture for " + path);

        if (!carTexture.loadFromFile(path)) throw runtime_error("Cannot load " + path);
        carTexture.setSmooth(true);

        Vector2u ts = carTexture.getSize();
        float aspect = static_cast<float>(ts.y) / static_cast<float>(ts.x);
        float targetWidth = cfg.playerShape.getSize().x * cfg.carScale;
        float targetHeight = targetWidth * aspect;

        car.setSize({targetWidth, targetHeight});
        car.setTexture(&carTexture);
        car.setFillColor(Color::White);
        car.setOrigin(car.getSize().x * 0.5f, car.getSize().y * 0.8f);

        string pathBG = (filesystem::current_path().parent_path() / "textures/bg/asphalt_road.png").string();
        if (pathBG.empty()) throw runtime_error("Cannot find texture for " + pathBG);

        if (!backgroundTexture.loadFromFile(pathBG)) throw runtime_error("Cannot load " + pathBG);
        backgroundTexture.setRepeated(true);

        background.setSize(Vector2f(20000.f, 20000.f));
        background.setTexture(&backgroundTexture);
        background.setTextureRect(IntRect(0, 0, 10000, 10000));
        background.setPosition(0.f, 0.f);

        view.setSize(cfg.windowSize.x, cfg.windowSize.y);
        view.setCenter(car.getPosition());
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
        input.up    = Keyboard::isKeyPressed(Keyboard::W);
        input.down  = Keyboard::isKeyPressed(Keyboard::S);
        input.left  = Keyboard::isKeyPressed(Keyboard::A);
        input.right = Keyboard::isKeyPressed(Keyboard::D);
        input.space = Keyboard::isKeyPressed(Keyboard::Space);
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
    }

    void update(const float * dt, float * speed, float * angelWheel, float * angel) {
        Vector2f dir(0.f, 0.f);
        float isAcceleration = 0.f;
        float isAngel = 0.f;
        float isHandBrake = 0.f;
        if (input.up)    isAcceleration += 1.f;
        if (input.down)  isAcceleration -= 2.f;
        if (input.left)  isAngel -= 1.f;
        if (input.right) isAngel += 1.f;
        if (input.space) isHandBrake -= 1.f;

        float dan = 800 * *dt;
        *angelWheel += dan * isAngel;
        *angelWheel = *angelWheel > cfg.angelSpeed ? cfg.angelSpeed : *angelWheel < -cfg.angelSpeed ? -cfg.angelSpeed : *angelWheel; //limiting max angel speed
        float da = cfg.acceleration * *dt;
        *speed += da * isAcceleration;
        *speed = *speed > cfg.moveSpeed ? cfg.moveSpeed : *speed < -cfg.moveSpeed ? -cfg.moveSpeed : *speed; //limiting max speed

        float angleRad = *angel * 3.14159265358979323846f / 180.f;
        dir.x = cos(angleRad);
        dir.y = sin(angleRad);

        Vector2f pos = car.getPosition();
        pos += dir * *speed * *dt;

        *angel += (*speed/cfg.moveSpeed * *angelWheel * *dt);
        *angel = fmod(*angel, 360.f);
        if (*angel < 0.f) *angel += 360.f;


        Vector2f ps =  cfg.playerShape.getSize();
        pos.x = max(ps.x, min(pos.x, background.getSize().x + ps.x));
        pos.y = max(ps.y, min(pos.y, background.getSize().y + ps.y));

        car.setPosition(pos);
        car.setRotation(*angel + cfg.textureAngleOffset);

        Vector2f localCenter(car.getSize().x * 0.5f, car.getSize().y * 0.5f);
        Vector2f worldCenter = car.getTransform().transformPoint(localCenter);
        view.setCenter(worldCenter);
        // view.setRotation(*angel+90); // Camera rotation
        window.setView(view);
    }

    void render() {
        window.clear(cfg.background);
        window.clear(cfg.background);
        window.draw(background);
        window.draw(car);
        window.display();
    }

private:
    View view;
    Config cfg;
    RectangleShape background;
    RenderWindow window;
    RectangleShape car;
    InputState input;
    Texture carTexture;
    Texture backgroundTexture;
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