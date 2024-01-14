#include <iostream>
#include <string>
#include <curl/curl.h>
#include <json/json.h>
#include "imgui.h"
#include <imgui-SFML.h>
#include <SFML/Graphics.hpp>


// Функция для получения погоды по названию города
std::string getWeather(const std::string& city)
{
    // Создание URL для API погоды с использованием названия города
    std::string url = "https://api.weatherapi.com/v1/forecast.json?key=53abcf2db1e74ecf844220304241301&q=" + city + "&days=5";

    // Инициализация библиотеки curl
    curl_global_init(CURL_GLOBAL_DEFAULT);

    // Создание объекта CURL
    CURL* curl = curl_easy_init();

    // Установка URL для запроса
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // Включение передачи данных в виде строки
    std::string response_string;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](char* ptr, size_t size, size_t nmemb, std::string* userdata) {
        userdata->append(ptr, size * nmemb);
        return size * nmemb;
        });
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

    // Выполнение запроса
    CURLcode res = curl_easy_perform(curl);

    // Проверка на успешное выполнение запроса
    if (res != CURLE_OK)
    {
        // Обработка ошибки при запросе
        std::cerr << "Failed to get weather data: " << curl_easy_strerror(res) << std::endl;
    }
    else
    {
        // Парсинг полученных данных JSON
        Json::Value root;
        Json::CharReaderBuilder builder;
        std::istringstream response_stream(response_string);
        std::string parse_errors;
        if (!Json::parseFromStream(builder, response_stream, &root, &parse_errors))
        {
            std::cerr << "Failed to parse JSON data: " << parse_errors << std::endl;
        }
        else
        {
            // Обработка полученных данных и формирование строки с погодой
            std::string weather = "";

            // Получение погоды на сегодня
            Json::Value current_weather = root["current"];
            std::string today = current_weather["condition"]["text"].asString();
            std::string today_temp = current_weather["temp_c"].asString();
            weather += "Сегодня (" + today + "): " + today_temp + "°C\n";

            // Получение погоды на следующие 4 дня
            Json::Value forecast = root["forecast"]["forecastday"];
            for (Json::Value day : forecast)
            {
                std::string date = day["date"].asString();
                std::string day_temp = day["day"]["avgtemp_c"].asString();
                std::string night_temp = day["night"]["avgtemp_c"].asString();
                std::string condition = day["day"]["condition"]["text"].asString();
                // TODO: Добавить символ обозначающий тип осадков или их отсутствие

                weather += date + ": " + day_temp + "°C / " + night_temp + "°C (" + condition + ")\n";
            }

            return weather;
        }
    }

    // Освобождение ресурсов
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return "";
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "Weather App");
    ImGui::SFML::Init(window);

    std::string city = "";
    std::string weather = "";

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(event);

            if (event.type == sf::Event::Closed)
                window.close();
        }

        ImGui::SFML::Update(window, sf::milliseconds(16));

        ImGui::Begin("Weather App");

        ImGui::Text("Введите название города:");
        ImGui::InputText("##CityInput", &city);

        if (ImGui::Button("Получить погоду"))
        {
            weather = getWeather(city);
            if (weather.empty())
            {
                ImGui::Text("Не удалось найти информацию");
            }
        }

        ImGui::Text(weather.c_str());

        ImGui::End();

        window.clear();
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();

    return 0;
}