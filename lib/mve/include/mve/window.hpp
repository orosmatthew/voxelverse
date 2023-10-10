#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <set>
#include <string>

#include "GLFW/glfw3.h"

#include "math/math.hpp"
#include "monitor.hpp"

namespace mve {

/**
 * @brief Keyboard input key
 */
enum class Key {
    unknown = GLFW_KEY_UNKNOWN,
    space = GLFW_KEY_SPACE,
    apostrophe = GLFW_KEY_APOSTROPHE,
    comma = GLFW_KEY_COMMA,
    minus = GLFW_KEY_MINUS,
    period = GLFW_KEY_PERIOD,
    slash = GLFW_KEY_SLASH,
    zero = GLFW_KEY_0,
    one = GLFW_KEY_1,
    two = GLFW_KEY_2,
    three = GLFW_KEY_3,
    four = GLFW_KEY_4,
    five = GLFW_KEY_5,
    six = GLFW_KEY_6,
    seven = GLFW_KEY_7,
    eight = GLFW_KEY_8,
    nine = GLFW_KEY_9,
    semicolon = GLFW_KEY_SEMICOLON,
    equal = GLFW_KEY_EQUAL,
    a = GLFW_KEY_A,
    b = GLFW_KEY_B,
    c = GLFW_KEY_C,
    d = GLFW_KEY_D,
    e = GLFW_KEY_E,
    f = GLFW_KEY_F,
    g = GLFW_KEY_G,
    h = GLFW_KEY_H,
    i = GLFW_KEY_I,
    j = GLFW_KEY_J,
    k = GLFW_KEY_K,
    l = GLFW_KEY_L,
    m = GLFW_KEY_M,
    n = GLFW_KEY_N,
    o = GLFW_KEY_O,
    p = GLFW_KEY_P,
    q = GLFW_KEY_Q,
    r = GLFW_KEY_R,
    s = GLFW_KEY_S,
    t = GLFW_KEY_T,
    u = GLFW_KEY_U,
    v = GLFW_KEY_V,
    w = GLFW_KEY_W,
    x = GLFW_KEY_X,
    y = GLFW_KEY_Y,
    z = GLFW_KEY_Z,
    left_bracket = GLFW_KEY_LEFT_BRACKET,
    backslash = GLFW_KEY_BACKSLASH,
    right_bracket = GLFW_KEY_RIGHT_BRACKET,
    grave_accent = GLFW_KEY_GRAVE_ACCENT,
    world_1 = GLFW_KEY_WORLD_1,
    world_2 = GLFW_KEY_WORLD_2,
    escape = GLFW_KEY_ESCAPE,
    enter = GLFW_KEY_ENTER,
    tab = GLFW_KEY_TAB,
    backspace = GLFW_KEY_BACKSPACE,
    insert = GLFW_KEY_INSERT,
    del = GLFW_KEY_DELETE,
    right = GLFW_KEY_RIGHT,
    left = GLFW_KEY_LEFT,
    down = GLFW_KEY_DOWN,
    up = GLFW_KEY_UP,
    page_up = GLFW_KEY_PAGE_UP,
    page_down = GLFW_KEY_PAGE_DOWN,
    home = GLFW_KEY_HOME,
    end = GLFW_KEY_END,
    caps_lock = GLFW_KEY_CAPS_LOCK,
    scroll_lock = GLFW_KEY_SCROLL_LOCK,
    num_lock = GLFW_KEY_NUM_LOCK,
    print_screen = GLFW_KEY_PRINT_SCREEN,
    pause = GLFW_KEY_PAUSE,
    f1 = GLFW_KEY_F1,
    f2 = GLFW_KEY_F2,
    f3 = GLFW_KEY_F3,
    f4 = GLFW_KEY_F4,
    f5 = GLFW_KEY_F5,
    f6 = GLFW_KEY_F6,
    f7 = GLFW_KEY_F7,
    f8 = GLFW_KEY_F8,
    f9 = GLFW_KEY_F9,
    f10 = GLFW_KEY_F10,
    f11 = GLFW_KEY_F11,
    f12 = GLFW_KEY_F12,
    f13 = GLFW_KEY_F13,
    f14 = GLFW_KEY_F14,
    f15 = GLFW_KEY_F15,
    f16 = GLFW_KEY_F16,
    f17 = GLFW_KEY_F17,
    f18 = GLFW_KEY_F18,
    f19 = GLFW_KEY_F19,
    f20 = GLFW_KEY_F20,
    f21 = GLFW_KEY_F21,
    f22 = GLFW_KEY_F22,
    f23 = GLFW_KEY_F23,
    f24 = GLFW_KEY_F24,
    f25 = GLFW_KEY_F25,
    numpad_0 = GLFW_KEY_KP_0,
    numpad_1 = GLFW_KEY_KP_1,
    numpad_2 = GLFW_KEY_KP_2,
    numpad_3 = GLFW_KEY_KP_3,
    numpad_4 = GLFW_KEY_KP_4,
    numpad_5 = GLFW_KEY_KP_5,
    numpad_6 = GLFW_KEY_KP_6,
    numpad_7 = GLFW_KEY_KP_7,
    numpad_8 = GLFW_KEY_KP_8,
    numpad_9 = GLFW_KEY_KP_9,
    numpad_decimal = GLFW_KEY_KP_DECIMAL,
    numpad_divide = GLFW_KEY_KP_DIVIDE,
    numpad_multiply = GLFW_KEY_KP_MULTIPLY,
    numpad_subtract = GLFW_KEY_KP_SUBTRACT,
    numpad_add = GLFW_KEY_KP_ADD,
    numpad_enter = GLFW_KEY_KP_ENTER,
    numpad_equal = GLFW_KEY_KP_EQUAL,
    left_shift = GLFW_KEY_LEFT_SHIFT,
    left_control = GLFW_KEY_LEFT_CONTROL,
    left_alt = GLFW_KEY_LEFT_ALT,
    left_super = GLFW_KEY_LEFT_SUPER,
    right_shift = GLFW_KEY_RIGHT_SHIFT,
    right_control = GLFW_KEY_RIGHT_CONTROL,
    right_alt = GLFW_KEY_RIGHT_ALT,
    right_super = GLFW_KEY_RIGHT_SUPER,
    menu = GLFW_KEY_MENU,
    last = GLFW_KEY_LAST,
};

/**
 * @brief Mouse button input
 */
enum class MouseButton {
    one = GLFW_MOUSE_BUTTON_1,
    two = GLFW_MOUSE_BUTTON_2,
    three = GLFW_MOUSE_BUTTON_3,
    four = GLFW_MOUSE_BUTTON_4,
    five = GLFW_MOUSE_BUTTON_5,
    six = GLFW_MOUSE_BUTTON_6,
    seven = GLFW_MOUSE_BUTTON_7,
    eight = GLFW_MOUSE_BUTTON_8,
    last = GLFW_MOUSE_BUTTON_LAST,
    left = GLFW_MOUSE_BUTTON_LEFT,
    right = GLFW_MOUSE_BUTTON_RIGHT,
    middle = GLFW_MOUSE_BUTTON_MIDDLE,
};

/**
 * @brief Gamepad axis input
 */
enum class GamepadAxis {
    left_x = GLFW_GAMEPAD_AXIS_LEFT_X,
    left_y = GLFW_GAMEPAD_AXIS_LEFT_Y,
    right_x = GLFW_GAMEPAD_AXIS_RIGHT_X,
    right_y = GLFW_GAMEPAD_AXIS_RIGHT_Y,
    left_trigger = GLFW_GAMEPAD_AXIS_LEFT_TRIGGER,
    right_trigger = GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER,
    last = GLFW_GAMEPAD_AXIS_LAST,
};

/**
 * @brief Gamepad button input
 */
enum class GamepadButton {
    a = GLFW_GAMEPAD_BUTTON_A,
    b = GLFW_GAMEPAD_BUTTON_B,
    x = GLFW_GAMEPAD_BUTTON_X,
    y = GLFW_GAMEPAD_BUTTON_Y,
    left_bumper = GLFW_GAMEPAD_BUTTON_LEFT_BUMPER,
    right_bumper = GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER,
    back = GLFW_GAMEPAD_BUTTON_BACK,
    start = GLFW_GAMEPAD_BUTTON_START,
    guide = GLFW_GAMEPAD_BUTTON_GUIDE,
    left_thumb = GLFW_GAMEPAD_BUTTON_LEFT_THUMB,
    right_thumb = GLFW_GAMEPAD_BUTTON_RIGHT_THUMB,
    dpad_up = GLFW_GAMEPAD_BUTTON_DPAD_UP,
    dpad_right = GLFW_GAMEPAD_BUTTON_DPAD_RIGHT,
    dpad_down = GLFW_GAMEPAD_BUTTON_DPAD_DOWN,
    dpad_left = GLFW_GAMEPAD_BUTTON_DPAD_LEFT,
    last = GLFW_GAMEPAD_BUTTON_LAST,
    cross = GLFW_GAMEPAD_BUTTON_CROSS,
    circle = GLFW_GAMEPAD_BUTTON_CIRCLE,
    square = GLFW_GAMEPAD_BUTTON_SQUARE,
    triangle = GLFW_GAMEPAD_BUTTON_TRIANGLE,
};

/**
 * @brief Joystick hat input
 */
enum class JoystickHat {
    centered = GLFW_HAT_CENTERED,
    up = GLFW_HAT_UP,
    right = GLFW_HAT_RIGHT,
    down = GLFW_HAT_DOWN,
    left = GLFW_HAT_LEFT,
    right_up = GLFW_HAT_RIGHT_UP,
    right_down = GLFW_HAT_RIGHT_DOWN,
    left_up = GLFW_HAT_LEFT_UP,
    left_down = GLFW_HAT_LEFT_DOWN,
};

/**
 * @brief Joystick input
 */
enum class Joystick {
    one = GLFW_JOYSTICK_1,
    two = GLFW_JOYSTICK_2,
    three = GLFW_JOYSTICK_3,
    four = GLFW_JOYSTICK_4,
    five = GLFW_JOYSTICK_5,
    six = GLFW_JOYSTICK_6,
    seven = GLFW_JOYSTICK_7,
    eight = GLFW_JOYSTICK_8,
    nine = GLFW_JOYSTICK_9,
    ten = GLFW_JOYSTICK_10,
    eleven = GLFW_JOYSTICK_11,
    twelve = GLFW_JOYSTICK_12,
    thirteen = GLFW_JOYSTICK_13,
    fourteen = GLFW_JOYSTICK_14,
    fifteen = GLFW_JOYSTICK_15,
    sixteen = GLFW_JOYSTICK_16,
};

/**
 * @brief Keyboard input modifiers
 */
enum KeyModifierBits {
    shift = GLFW_MOD_SHIFT,
    control = GLFW_MOD_CONTROL,
    alt = GLFW_MOD_ALT,
    super = GLFW_MOD_SUPER,
    caps_lock = GLFW_MOD_CAPS_LOCK,
    num_lock = GLFW_MOD_NUM_LOCK,
};

/**
 * @brief Window class
 */
class Window {
public:
    /**
     * @brief Construct window class
     * @param title - Title of window
     * @param size - Initial size of window
     */
    Window(const std::string& title, mve::Vector2i size, bool resizable = true);

    /**
     * @brief Get GLFW window handle
     * @return - Returns GLFW window pointer
     */
    [[nodiscard]] GLFWwindow* glfw_handle() const;

    /**
     * @brief Get current size of window
     * @return - Returns current window size
     */
    [[nodiscard]] mve::Vector2i size() const;

    /**
     * @brief Determine if window should close such as if close button is pressed
     * @return - Returns true if window should close
     */
    [[nodiscard]] bool should_close() const;

    /**
     * @brief Set callback function when window is resized
     * @param resize_callback - Resize callback. Takes ivec2 new size as parameter
     */
    void set_resize_callback(const std::function<void(mve::Vector2i)>& resize_callback);

    /**
     * @brief Remove window resize callback if set
     */
    void remove_resize_callback();

    /**
     * @brief Update window. Updates inputs and checks if resized
     */
    void poll_events();

    /**
     * @brief Blocks until event occurs
     */
    void wait_for_events() const;

    void enable_event_waiting();

    void disable_event_waiting();

    /**
     * @brief Determine if keyboard key pressed
     * @param key - Keyboard key
     * @return - Returns true if pressed
     */
    bool is_key_pressed(Key key) const;

    [[nodiscard]] bool is_key_down(Key key) const;

    [[nodiscard]] bool is_key_released(Key key) const;

    [[nodiscard]] bool is_key_repeated(Key key) const;

    [[nodiscard]] bool is_mouse_button_down(MouseButton button) const;

    [[nodiscard]] bool is_mouse_button_pressed(MouseButton button) const;

    [[nodiscard]] bool is_mouse_button_released(MouseButton button) const;

    [[nodiscard]] mve::Vector2 mouse_pos() const;

    [[nodiscard]] mve::Vector2 mouse_delta() const;

    void fullscreen(bool use_native = false);

    void windowed();

    void hide();

    void show();

    void maximize();

    void minimize();

    void restore();

    void set_title(const std::string& title);

    void move_to(mve::Vector2i pos);

    void fullscreen_to(Monitor monitor, bool use_native = false);

    void set_min_size(mve::Vector2i size);

    void resize(mve::Vector2i size);

    [[nodiscard]] bool is_minimized() const;

    [[nodiscard]] bool is_maximized() const;

    [[nodiscard]] bool is_focused() const;

    [[nodiscard]] bool is_hidden() const;

    [[nodiscard]] bool is_fullscreen() const;

    [[nodiscard]] Monitor current_monitor() const;

    [[nodiscard]] mve::Vector2i position() const;

    void set_clipboard_text(const std::string& text);

    [[nodiscard]] std::string clipboard_text() const;

    void show_cursor();

    void hide_cursor();

    void enable_cursor();

    void disable_cursor();

    void set_cursor_pos(mve::Vector2 pos);

    mve::Vector2 mouse_scroll() const;

    [[nodiscard]] bool is_cursor_in_window() const;

    [[nodiscard]] bool is_cursor_hidden() const;

    [[nodiscard]] bool is_resizable() const;

    /**
     * @brief Get cursor translation
     * @param clamped_to_window - If true then returns value clamp inside of window
     * @return - Returns vec2 of cursor translation
     */
    mve::Vector2 get_cursor_pos(bool clamped_to_window = true);

    inline const std::vector<std::string> input_stream() const
    {
        return m_input_stream;
    }

private:
    using UniqueGlfwWindow = std::unique_ptr<GLFWwindow, std::function<void(GLFWwindow*)>>;

    const bool m_resizable;
    UniqueGlfwWindow m_glfw_window;
    std::optional<std::function<void(mve::Vector2i)>> m_resize_callback;
    bool m_hidden;
    bool m_minimized;
    bool m_maximized;
    bool m_focused;
    bool m_cursor_hidden;
    bool m_cursor_in_window;
    bool m_event_waiting;
    mve::Vector2 m_current_scroll_offset {};
    mve::Vector2 m_scroll_offset {};
    mve::Vector2i m_pos;
    mve::Vector2i m_size;
    mve::Vector2i m_windowed_size;
    bool m_fullscreen;
    mve::Vector2i m_min_size;
    std::set<Key> m_current_keys_down {};
    std::set<Key> m_keys_down {};
    std::set<Key> m_keys_pressed {};
    std::set<Key> m_current_keys_released {};
    std::set<Key> m_keys_released {};
    std::set<Key> m_current_keys_repeated {};
    std::set<Key> m_keys_repeated {};

    std::set<MouseButton> m_current_mouse_buttons_down {};
    std::set<MouseButton> m_mouse_buttons_down {};
    std::set<MouseButton> m_current_mouse_buttons_released {};
    std::set<MouseButton> m_mouse_buttons_pressed {};
    std::set<MouseButton> m_mouse_buttons_released {};

    mve::Vector2 m_current_mouse_pos;
    mve::Vector2 m_mouse_pos_prev;
    mve::Vector2 m_mouse_delta;
    mve::Vector2 m_mouse_pos;

    std::vector<std::string> m_current_input_stream {};
    std::vector<std::string> m_input_stream {};

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