#ifndef VENTANA_H
#define VENTANA_H

#include <SFML/Graphics.hpp>
#include <functional>
#include <string>
#include <windows.h>

class Ventana {
private:
    sf::RenderWindow window;
    sf::Font font;
    sf::Text inputText;
    sf::Text promptText;
    sf::RectangleShape button;
    sf::Text buttonText;
    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;
    std::wstring inputString;
    bool isActive;
    std::function<void(const std::wstring&)> onSubmit;

    void handleFolderPath(const std::wstring& path) {
        std::wstring finalPath = path;

        if (finalPath.front() == L'"' && finalPath.back() == L'"') {
            finalPath = finalPath.substr(1, finalPath.length() - 2);
        }

        DWORD attrs = GetFileAttributesW(finalPath.c_str());
        if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
            onSubmit(finalPath);
        }
        else {
            promptText.setString("Carpeta no valida. Intente de nuevo:");
            promptText.setFillColor(sf::Color::Red);
        }
    }

    void handlePaste() {
        if (OpenClipboard(NULL)) {
            HANDLE hData = GetClipboardData(CF_UNICODETEXT);
            if (hData) {
                wchar_t* text = static_cast<wchar_t*>(GlobalLock(hData));
                if (text) {
                    inputString += text;
                    inputText.setString(sf::String(inputString));
                    GlobalUnlock(hData);
                }
            }
            CloseClipboard();
        }
    }

public:
    Ventana(const std::function<void(const std::wstring&)>& submitCallback)
        : window(sf::VideoMode(800, 200), "Buscador de archivos duplicados"),
        isActive(false),
        onSubmit(submitCallback) {

        window.setKeyRepeatEnabled(true);
        window.setFramerateLimit(60);
        window.clear(sf::Color::Black);

        if (!backgroundTexture.loadFromFile("background.png")) {
            throw std::runtime_error("No se pudo cargar la imagen de fondo");
        }

        backgroundSprite.setTexture(backgroundTexture);
        float scaleX = 800.0f / backgroundTexture.getSize().x;
        float scaleY = 200.0f / backgroundTexture.getSize().y;
        backgroundSprite.setScale(scaleX, scaleY);

        if (!font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf")) {
            throw std::runtime_error("No se pudo cargar la fuente");
        }

        promptText.setFont(font);
        promptText.setString("Ingrese la ubicacion de la carpeta:");
        promptText.setCharacterSize(20);
        promptText.setFillColor(sf::Color::White);
        promptText.setPosition(10, 20);

        inputText.setFont(font);
        inputText.setCharacterSize(20);
        inputText.setFillColor(sf::Color::White);
        inputText.setPosition(10, 60);

        button.setSize(sf::Vector2f(100, 40));
        button.setPosition(690, 150);
        button.setFillColor(sf::Color::Red);

        buttonText.setFont(font);
        buttonText.setString("Salir");
        buttonText.setCharacterSize(20);
        buttonText.setFillColor(sf::Color::White);
        buttonText.setPosition(720, 160);
    }

    void run() {
        while (window.isOpen()) {
            processEvents();
            window.clear(sf::Color::Black);
            draw();
            window.display();
        }
    }

    bool isRunning() const {
        return window.isOpen();
    }

    void processEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            else if (event.type == sf::Event::TextEntered && isActive) {
                handleTextInput(event.text.unicode);
            }
            else if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);

                if (button.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                    window.close();
                }
                else if (mousePos.y >= 60 && mousePos.y <= 90) {
                    isActive = true;
                }
                else {
                    isActive = false;
                }
            }
            else if (event.type == sf::Event::KeyPressed) {
                if (event.key.control && event.key.code == sf::Keyboard::V && isActive) {
                    handlePaste();
                }
                else if (event.key.code == sf::Keyboard::Return && isActive) {
                    if (!inputString.empty()) {
                        handleFolderPath(inputString);
                        inputString.clear();
                        inputText.setString("");
                    }
                }
            }
        }
    }

    void handleTextInput(sf::Uint32 unicode) {
        if (unicode == '\b') {
            if (!inputString.empty()) {
                inputString.pop_back();
            }
        }
        else if (unicode >= 32) {
            inputString += static_cast<wchar_t>(unicode);
        }
        inputText.setString(sf::String(inputString));
    }

    void draw() {
        window.draw(backgroundSprite);
        window.draw(promptText);
        window.draw(inputText);
        window.draw(button);
        window.draw(buttonText);

        if (isActive) {
            sf::RectangleShape cursor(sf::Vector2f(2, 24));
            cursor.setPosition(inputText.getGlobalBounds().width + 12, 60);
            cursor.setFillColor(sf::Color::White);
            window.draw(cursor);
        }
    }
};

#endif