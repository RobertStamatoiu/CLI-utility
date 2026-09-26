#pragma once

/**
 * @file term.hpp
 * @brief Header-only terminal colors, styles, themes, and output helpers.
 *
 * The public output API is term::print() and term::println(). Preset styles
 * are mutable and are replaced by style::load() when a theme is selected.
 * Licenses to nlohmann/json.hpp, included in this header
 */

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

#include "./json.hpp"

using json = nlohmann::json;

namespace term
{
    namespace detail
    {
        template <typename T>
        concept Clampable = requires(T a, T b) {
            { std::min(a, b) };
            { std::max(a, b) };
        };

        template <Clampable T>
        constexpr T clamp(T minimum, T value, T maximum)
        {
            return std::max(minimum, std::min(value, maximum));
        }

        inline std::string upper(std::string value)
        {
            for (char& character : value)
                character = static_cast<char>(
                    std::toupper(static_cast<unsigned char>(character)));
            return value;
        }

        inline int parse_hex_digit(char value)
        {
            const unsigned char digit =
                static_cast<unsigned char>(std::toupper(static_cast<unsigned char>(value)));
            if (digit >= '0' && digit <= '9')
                return digit - '0';
            if (digit >= 'A' && digit <= 'F')
                return digit - 'A' + 10;
            throw std::invalid_argument("Hex color must contain digits 0-9 or letters A-F.");
        }

        inline int parse_hex_pair(char high, char low)
        {
            return parse_hex_digit(high) * 16 + parse_hex_digit(low);
        }
    }

    namespace color
    {
        /** An RGB color that can be serialized as a 24-bit ANSI color. */
        class RGBObject
        {
        private:
            std::uint8_t red_;
            std::uint8_t green_;
            std::uint8_t blue_;

        public:
            constexpr RGBObject(int red = 0, int green = 0, int blue = 0)
                : red_(detail::clamp(0, red, 255)),
                  green_(detail::clamp(0, green, 255)),
                  blue_(detail::clamp(0, blue, 255))
            {
            }

            constexpr int red() const noexcept { return red_; }
            constexpr int green() const noexcept { return green_; }
            constexpr int blue() const noexcept { return blue_; }

            std::string serialize(bool foreground = true) const
            {
                return std::string(foreground ? "\x1b[38;2;" : "\x1b[48;2;") +
                       std::to_string(red_) + ";" + std::to_string(green_) + ";" +
                       std::to_string(blue_) + "m";
            }
        };

        inline RGBObject rgb(int red, int green, int blue)
        {
            return RGBObject(red, green, blue);
        }

        inline RGBObject hsv(double hue, double saturation, double value)
        {
            const double h = detail::clamp(0.0, hue, 360.0);
            const double s = detail::clamp(0.0, saturation, 1.0);
            const double v = detail::clamp(0.0, value, 1.0);
            const double chroma = v * s;
            const double minimum = v - chroma;
            const double x = chroma * (1 - std::abs(std::fmod(h / 60, 2) - 1));
            double red = 0;
            double green = 0;
            double blue = 0;

            if (h < 60) red = chroma, green = x;
            else if (h < 120) red = x, green = chroma;
            else if (h < 180) green = chroma, blue = x;
            else if (h < 240) green = x, blue = chroma;
            else if (h < 300) red = x, blue = chroma;
            else red = chroma, blue = x;

            return RGBObject(
                static_cast<int>(std::round((red + minimum) * 255)),
                static_cast<int>(std::round((green + minimum) * 255)),
                static_cast<int>(std::round((blue + minimum) * 255)));
        }

        inline RGBObject hex(std::string value)
        {
            if (value.size() != 4 && value.size() != 7)
                throw std::invalid_argument("Hex color must be #RGB or #RRGGBB.");
            if (value[0] != '#')
                throw std::invalid_argument("Hex color must begin with '#'.");

            if (value.size() == 4)
                return RGBObject(
                    detail::parse_hex_digit(value[1]) * 17,
                    detail::parse_hex_digit(value[2]) * 17,
                    detail::parse_hex_digit(value[3]) * 17);
            return RGBObject(
                detail::parse_hex_pair(value[1], value[2]),
                detail::parse_hex_pair(value[3], value[4]),
                detail::parse_hex_pair(value[5], value[6]));
        }

        inline RGBObject red = rgb(210, 75, 75);
        inline RGBObject orange = rgb(220, 135, 65);
        inline RGBObject yellow = rgb(215, 190, 75);
        inline RGBObject green = rgb(80, 175, 100);
        inline RGBObject cyan = rgb(70, 175, 180);
        inline RGBObject blue = rgb(75, 125, 200);
        inline RGBObject purple = rgb(140, 90, 190);
        inline RGBObject magenta = rgb(195, 85, 165);
        inline RGBObject pink = rgb(215, 125, 155);
        inline RGBObject brown = rgb(155, 105, 70);
        inline RGBObject bright_red = rgb(240, 105, 105);
        inline RGBObject bright_orange = rgb(245, 165, 90);
        inline RGBObject bright_yellow = rgb(240, 220, 105);
        inline RGBObject bright_green = rgb(105, 215, 125);
        inline RGBObject bright_cyan = rgb(95, 215, 220);
        inline RGBObject bright_blue = rgb(105, 160, 230);
        inline RGBObject bright_purple = rgb(175, 120, 225);
        inline RGBObject bright_magenta = rgb(225, 115, 195);
        inline RGBObject bright_pink = rgb(240, 155, 180);
        inline RGBObject bright_brown = rgb(190, 135, 90);
        inline RGBObject black = hex("#000");
        inline RGBObject dark_gray = hex("#48484F");
        inline RGBObject gray = hex("#85858D");
        inline RGBObject light_gray = hex("#BDBDC5");
        inline RGBObject white = hex("#E8E8EC");
    }

    namespace style
    {
        enum class TextStyle : std::uint8_t
        {
            None = 0,
            Bold = 1 << 0,
            Dim = 1 << 1,
            Italic = 1 << 2,
            Underline = 1 << 3,
            Blink = 1 << 4,
            Inverse = 1 << 5,
            Hidden = 1 << 6,
            Strikethrough = 1 << 7
        };

        constexpr TextStyle operator|(TextStyle left, TextStyle right)
        {
            return static_cast<TextStyle>(
                static_cast<std::uint8_t>(left) | static_cast<std::uint8_t>(right));
        }

        constexpr TextStyle operator&(TextStyle left, TextStyle right)
        {
            return static_cast<TextStyle>(
                static_cast<std::uint8_t>(left) & static_cast<std::uint8_t>(right));
        }

        constexpr TextStyle& operator|=(TextStyle& left, TextStyle right)
        {
            left = left | right;
            return left;
        }

        constexpr bool has(TextStyle value, TextStyle flag)
        {
            return (value & flag) != TextStyle::None;
        }

        inline std::string serialize(TextStyle value)
        {
            std::string result;
            if (has(value, TextStyle::Bold)) result += "\x1b[1m";
            if (has(value, TextStyle::Dim)) result += "\x1b[2m";
            if (has(value, TextStyle::Italic)) result += "\x1b[3m";
            if (has(value, TextStyle::Underline)) result += "\x1b[4m";
            if (has(value, TextStyle::Blink)) result += "\x1b[5m";
            if (has(value, TextStyle::Inverse)) result += "\x1b[7m";
            if (has(value, TextStyle::Hidden)) result += "\x1b[8m";
            if (has(value, TextStyle::Strikethrough)) result += "\x1b[9m";
            return result;
        }

        class Style
        {
        public:
            color::RGBObject foreground = color::white;
            color::RGBObject background = color::black;
            std::variant<TextStyle, std::string> textStyle = TextStyle::None;

            std::string serialize() const
            {
                std::string result = foreground.serialize() + background.serialize(false);
                result += std::visit(
                    [](const auto& value)
                    {
                        using Value = std::decay_t<decltype(value)>;
                        if constexpr (std::is_same_v<Value, TextStyle>)
                            return style::serialize(value);
                        else
                        {
                            TextStyle combined = TextStyle::None;
                            std::stringstream stream(value);
                            for (std::string token; stream >> token;)
                            {
                                token = detail::upper(token);
                                if (token == "BOLD") combined |= TextStyle::Bold;
                                else if (token == "DIM") combined |= TextStyle::Dim;
                                else if (token == "ITALIC") combined |= TextStyle::Italic;
                                else if (token == "UNDERLINE") combined |= TextStyle::Underline;
                                else if (token == "BLINK") combined |= TextStyle::Blink;
                                else if (token == "INVERSE") combined |= TextStyle::Inverse;
                                else if (token == "HIDDEN") combined |= TextStyle::Hidden;
                                else if (token == "STRIKETHROUGH")
                                    combined |= TextStyle::Strikethrough;
                                else
                                    throw std::invalid_argument(
                                        "Unsupported text style: " + token);
                            }
                            return style::serialize(combined);
                        }
                    },
                    textStyle);
                return result;
            }
        
            static Style BuildFromJson(json data){
                if(!(data.contains("foreground") && data["foreground"].is_string())){
                    throw new std::invalid_argument("Passed json object does not contain the required field \"foreground\" or may contain an invalid format");
                } else if (!(data.contains("background") && data["background"].is_string())){
                    throw new std::invalid_argument("Passed json object does not contain the required field \"background\" or may contain an invalid format");
                } else if (!(data.contains("style") && data["style"].is_string())){
                    throw new std::invalid_argument("Passed json object does not contain the required field \"style\" or may contain an invalid format");
                }
                color::RGBObject fg, bg;
                // try to parse the forground and background
                // we currently dont support named colors like "red"
                try{
                    fg = color::hex(data["foreground"]);
                } catch (std::invalid_argument &e) {
                    throw new std::invalid_argument("Fields \"foreground\" and \"background\" must contain valid hex values");
                }
                try{
                    bg = color::hex(data["background"]);
                } catch (std::invalid_argument &e) {
                    throw new std::invalid_argument("Fields \"foreground\" and \"background\" must contain valid hex values");
                }
                return Style{.foreground = fg, .background = bg, .textStyle = data["style"].get<std::string>()};

            }
        };

        /** A named collection of semantic CLI styles. */
        struct Theme
        {
            Style success;
            Style error;
            Style warning;
            Style info;
            Style muted;

            static Theme BuildFromJson(json data){
                if(!data.is_object()){
                    throw new std::invalid_argument("Passed json object must be an object type");
                } else if(!(data.contains("success") && data["success"].is_object())){
                    throw new std::invalid_argument("Passed json object does not contain the required field \"success\"");
                } else if (!(data.contains("error") && data["error"].is_object())){
                    throw new std::invalid_argument("Passed json object does not contain the required field \"error\"");
                } else if (!(data.contains("warning") && data["warning"].is_object())){
                    throw new std::invalid_argument("Passed json object does not contain the required field \"warning\"");
                } else if (!(data.contains("info") && data["info"].is_object())){
                    throw new std::invalid_argument("Passed json object does not contain the required field \"info\"");
                } else if (!(data.contains("muted") && data["muted"].is_object())){
                    throw new std::invalid_argument("Passed json object does not contain the required field \"muted\"");
                }
                return Theme{
                    .success = Style::BuildFromJson(data["success"]),
                    .error = Style::BuildFromJson(data["error"]),
                    .warning = Style::BuildFromJson(data["warning"]),
                    .info = Style::BuildFromJson(data["info"]),
                    .muted = Style::BuildFromJson(data["muted"])
                };
            }
        };

        inline const Theme Light{
            .success = {.foreground = color::green, .background = color::white,
                        .textStyle = TextStyle::Bold},
            .error = {.foreground = color::red, .background = color::white,
                      .textStyle = TextStyle::Bold},
            .warning = {.foreground = color::orange, .background = color::white,
                        .textStyle = TextStyle::Bold},
            .info = {.foreground = color::blue, .background = color::white,
                     .textStyle = TextStyle::None},
            .muted = {.foreground = color::dark_gray, .background = color::white,
                      .textStyle = TextStyle::Dim}};

        inline const Theme Dark{
            .success = {.foreground = color::bright_green, .background = color::black,
                        .textStyle = TextStyle::Bold},
            .error = {.foreground = color::bright_red, .background = color::black,
                      .textStyle = TextStyle::Bold},
            .warning = {.foreground = color::bright_yellow, .background = color::black,
                        .textStyle = TextStyle::Bold},
            .info = {.foreground = color::bright_cyan, .background = color::black,
                     .textStyle = TextStyle::Italic},
            .muted = {.foreground = color::gray, .background = color::black,
                      .textStyle = TextStyle::Italic}};

        std::unordered_map<std::string, Theme> Themes{
            {"Light", Light},
            {"Dark", Dark}
        };

        inline Style success = Dark.success;
        inline Style error = Dark.error;
        inline Style warning = Dark.warning;
        inline Style info = Dark.info;
        inline Style muted = Dark.muted;
        inline std::string current_theme = "Dark";

        /** Load a built-in theme by its exact name. Returns false if unknown. */
        inline bool load(std::string_view name)
        {
            const Theme* selected = nullptr;
            if (name == "Light") selected = &Light;
            else if (name == "Dark") selected = &Dark;
            else if (name == current_theme) return true;
            else{
                if(!Themes.contains(std::string(name))) return false;
                selected = &Themes[std::string(name)];
            }

            success = selected->success;
            error = selected->error;
            warning = selected->warning;
            info = selected->info;
            muted = selected->muted;
            current_theme = std::string(name);
            return true;
        }
        inline bool loadThemesFromJson(std::string_view path){
            std::ifstream file{std::string(path)};
            std::string file_contents;
            while(file >> file_contents){}
            json themes = json::parse(file_contents);
            for(const auto& [key, value] : themes.items()){
                Theme t;
                try{
                    t = Theme::BuildFromJson(value);
                } catch (std::invalid_argument &e){
                    std::cerr << "An error occured while parsing json data...\n";
                    std::cerr << e.what() << "\n";
                    continue;
                }
                if(Themes.contains(key)){
                    // a theme with this name already exists
                    // for now, we will log this to terminal and not overwrite the already existing one
                    std::cerr << "A theme with the name \"" << key << "\" already exists..."; 
                }
                Themes[key] = t;
            }
            return true;
        }

    }

    inline void print(std::string_view message, style::Style output_style = style::Style())
    {
        std::cout << output_style.serialize() << message << "\x1b[0m";
    }

    inline void println(std::string_view message, style::Style output_style = style::Style())
    {
        std::cout << output_style.serialize() << message << "\x1b[0m\n";
    }
}
