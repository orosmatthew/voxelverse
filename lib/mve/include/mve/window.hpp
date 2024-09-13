#pragma once

#include <functional>
#include <optional>
#include <set>
#include <string>

#include <mve/monitor.hpp>

#include <nnm/nnm.hpp>


struct GLFWwindow;

namespace mve {

enum class Key {
    unknown = -1,
    space = 32,
    apostrophe = 39,
    comma = 44,
    minus = 45,
    period = 46,
    slash = 47,
    zero = 48,
    one = 49,
    two = 50,
    three = 51,
    four = 52,
    five = 53,
    six = 54,
    seven = 55,
    eight = 56,
    nine = 57,
    semicolon = 59,
    equal = 61,
    a = 65,
    b = 66,
    c = 67,
    d = 68,
    e = 69,
    f = 70,
    g = 71,
    h = 72,
    i = 73,
    j = 74,
    k = 75,
    l = 76,
    m = 77,
    n = 78,
    o = 79,
    p = 80,
    q = 81,
    r = 82,
    s = 83,
    t = 84,
    u = 85,
    v = 86,
    w = 87,
    x = 88,
    y = 89,
    z = 90,
    left_bracket = 91,
    backslash = 92,
    right_bracket = 93,
    grave_accent = 96,
    world_1 = 161,
    world_2 = 162,
    escape = 256,
    enter = 257,
    tab = 258,
    backspace = 259,
    insert = 260,
    del = 261,
    right = 262,
    left = 263,
    down = 264,
    up = 265,
    page_up = 266,
    page_down = 267,
    home = 268,
    end = 269,
    caps_lock = 280,
    scroll_lock = 281,
    num_lock = 282,
    print_screen = 283,
    pause = 284,
    f1 = 290,
    f2 = 291,
    f3 = 292,
    f4 = 293,
    f5 = 294,
    f6 = 295,
    f7 = 296,
    f8 = 297,
    f9 = 298,
    f10 = 299,
    f11 = 300,
    f12 = 301,
    f13 = 302,
    f14 = 303,
    f15 = 304,
    f16 = 305,
    f17 = 306,
    f18 = 307,
    f19 = 308,
    f20 = 309,
    f21 = 310,
    f22 = 311,
    f23 = 312,
    f24 = 313,
    f25 = 314,
    numpad_0 = 320,
    numpad_1 = 321,
    numpad_2 = 322,
    numpad_3 = 323,
    numpad_4 = 324,
    numpad_5 = 325,
    numpad_6 = 326,
    numpad_7 = 327,
    numpad_8 = 328,
    numpad_9 = 329,
    numpad_decimal = 330,
    numpad_divide = 331,
    numpad_multiply = 332,
    numpad_subtract = 333,
    numpad_add = 334,
    numpad_enter = 335,
    numpad_equal = 336,
    left_shift = 340,
    left_control = 341,
    left_alt = 342,
    left_super = 343,
    right_shift = 344,
    right_control = 345,
    right_alt = 346,
    right_super = 347,
    menu = 348,
    last = 348,
};

enum class MouseButton {
    one = 0,
    two = 1,
    three = 2,
    four = 3,
    five = 4,
    six = 5,
    seven = 6,
    eight = 7,
    last = 7,
    left = 0,
    right = 1,
    middle = 2,
};

enum class GamepadAxis {
    left_x = 0,
    left_y = 1,
    right_x = 2,
    right_y = 3,
    left_trigger = 4,
    right_trigger = 5,
    last = 5,
};

enum class GamepadButton {
    a = 0,
    b = 1,
    x = 2,
    y = 3,
    left_bumper = 4,
    right_bumper = 5,
    back = 6,
    start = 7,
    guide = 8,
    left_thumb = 9,
    right_thumb = 10,
    dpad_up = 11,
    dpad_right = 12,
    dpad_down = 13,
    dpad_left = 14,
    last = 14,
    cross = 0,
    circle = 1,
    square = 2,
    triangle = 3,
};

enum class JoystickHat {
    centered = 0,
    up = 1,
    right = 2,
    down = 4,
    left = 8,
    right_up = 2 | 1,
    right_down = 2 | 4,
    left_up = 8 | 1,
    left_down = 8 | 4,
};

enum class Joystick {
    one = 0,
    two = 1,
    three = 2,
    four = 3,
    five = 4,
    six = 5,
    seven = 6,
    eight = 7,
    nine = 8,
    ten = 9,
    eleven = 10,
    twelve = 11,
    thirteen = 12,
    fourteen = 13,
    fifteen = 14,
    sixteen = 15,
};

enum KeyModifierBits {
    shift = 0x0001,
    control = 0x0002,
    alt = 0x0004,
    super = 0x0008,
    caps_lock = 0x0010,
    num_lock = 0x0020,
};

class Window {
public:
    Window(const std::string& title, nnm::Vector2i size, bool resizable = true);

    ~Window();

    [[nodiscard]] GLFWwindow* glfw_handle() const;

    [[nodiscard]] nnm::Vector2i size() const;

    [[nodiscard]] bool should_close() const;

    void set_resize_callback(const std::function<void(nnm::Vector2i)>& resize_callback);

    void remove_resize_callback();

    void poll_events();

    static void wait_for_events();

    void enable_event_waiting();

    void disable_event_waiting();

    [[nodiscard]] bool is_key_pressed(Key key) const;

    [[nodiscard]] bool is_key_down(Key key) const;

    [[nodiscard]] bool is_key_released(Key key) const;

    [[nodiscard]] bool is_key_repeated(Key key) const;

    [[nodiscard]] bool is_mouse_button_down(MouseButton button) const;

    [[nodiscard]] bool is_mouse_button_pressed(MouseButton button) const;

    [[nodiscard]] bool is_mouse_button_released(MouseButton button) const;

    [[nodiscard]] nnm::Vector2f mouse_pos() const;

    [[nodiscard]] nnm::Vector2f mouse_delta() const;

    void fullscreen(bool use_native = false);

    void windowed();

    void hide();

    void show();

    void maximize();

    void minimize() const;

    void restore() const;

    void set_title(const std::string& title) const;

    void move_to(nnm::Vector2i pos) const;

    void fullscreen_to(Monitor monitor, bool use_native = false) const;

    void set_min_size(nnm::Vector2i size) const;

    void resize(nnm::Vector2i size) const;

    [[nodiscard]] bool is_minimized() const;

    [[nodiscard]] bool is_maximized() const;

    [[nodiscard]] bool is_focused() const;

    [[nodiscard]] bool is_hidden() const;

    [[nodiscard]] bool is_fullscreen() const;

    [[nodiscard]] Monitor current_monitor() const;

    [[nodiscard]] nnm::Vector2i position() const;

    void set_clipboard_text(const std::string& text) const;

    [[nodiscard]] std::string clipboard_text() const;

    void show_cursor();

    void hide_cursor();

    void enable_cursor();

    void disable_cursor();

    void set_cursor_pos(nnm::Vector2f pos) const;

    [[nodiscard]] nnm::Vector2f mouse_scroll() const;

    [[nodiscard]] bool is_cursor_in_window() const;

    [[nodiscard]] bool is_cursor_hidden() const;

    [[nodiscard]] bool is_resizable() const;

    [[nodiscard]] nnm::Vector2f get_cursor_pos(bool clamped_to_window = true) const;

    [[nodiscard]] const std::vector<char32_t>& input_char_stream() const;

private:
    template <typename T>
    struct Buffered {
        T current;
        T buffered;

        void swap()
        {
            std::swap(current, buffered);
        }
    };

    const bool m_resizable;
    GLFWwindow* m_glfw_window;
    std::optional<std::function<void(nnm::Vector2i)>> m_resize_callback;
    bool m_hidden;
    bool m_minimized;
    bool m_maximized;
    bool m_focused;
    bool m_cursor_hidden;
    bool m_cursor_in_window;
    bool m_event_waiting;
    Buffered<nnm::Vector2f> m_scroll_offset;
    nnm::Vector2i m_pos;
    nnm::Vector2i m_size;
    nnm::Vector2i m_windowed_size;
    bool m_fullscreen;
    nnm::Vector2i m_min_size;
    Buffered<std::vector<Key>> m_keys_down {};
    std::vector<Key> m_keys_pressed {};
    Buffered<std::vector<Key>> m_keys_released {};
    Buffered<std::vector<Key>> m_keys_repeated {};

    Buffered<std::vector<MouseButton>> m_mouse_buttons_down {};
    std::vector<MouseButton> m_mouse_buttons_pressed {};
    Buffered<std::vector<MouseButton>> m_mouse_buttons_released {};

    Buffered<nnm::Vector2f> m_mouse_pos;
    nnm::Vector2f m_mouse_pos_prev;
    nnm::Vector2f m_mouse_delta;

    Buffered<std::vector<char32_t>> m_input_char_stream;

    static void glfw_framebuffer_resize_callback(GLFWwindow* window, int width, int height);

    static void glfw_iconify_callback(GLFWwindow* window, int iconified);

    static void glfw_focused_callback(GLFWwindow* window, int focused);

    static void glfw_cursor_enter_callback(GLFWwindow* window, int entered);

    static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

    static void glfw_cursor_pos_callback(GLFWwindow* window, double pos_x, double pos_y);

    static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

    static void glfw_scroll_callback(GLFWwindow* window, double offset_x, double offset_y);

    static void glfw_char_callback(GLFWwindow* window, unsigned int codepoint);
};
}