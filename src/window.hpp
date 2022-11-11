#pragma once

#include <functional>
#include <optional>
#include <string>
#include <memory>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace mve {

    enum class InputKey {
        e_unknown = GLFW_KEY_UNKNOWN,
        e_space = GLFW_KEY_SPACE,
        e_apostrophe = GLFW_KEY_APOSTROPHE,
        e_comma = GLFW_KEY_COMMA,
        e_minus = GLFW_KEY_MINUS,
        e_period = GLFW_KEY_PERIOD,
        e_slash = GLFW_KEY_SLASH,
        e_0 = GLFW_KEY_0,
        e_1 = GLFW_KEY_1,
        e_2 = GLFW_KEY_2,
        e_3 = GLFW_KEY_3,
        e_4 = GLFW_KEY_4,
        e_5 = GLFW_KEY_5,
        e_6 = GLFW_KEY_6,
        e_7 = GLFW_KEY_7,
        e_8 = GLFW_KEY_8,
        e_9 = GLFW_KEY_9,
        e_semicolon = GLFW_KEY_SEMICOLON,
        e_equal = GLFW_KEY_EQUAL,
        e_a = GLFW_KEY_A,
        e_b = GLFW_KEY_B,
        e_c = GLFW_KEY_C,
        e_d = GLFW_KEY_D,
        e_e = GLFW_KEY_E,
        e_f = GLFW_KEY_F,
        e_g = GLFW_KEY_G,
        e_h = GLFW_KEY_H,
        e_i = GLFW_KEY_I,
        e_j = GLFW_KEY_J,
        e_k = GLFW_KEY_K,
        e_l = GLFW_KEY_L,
        e_m = GLFW_KEY_M,
        e_n = GLFW_KEY_N,
        e_o = GLFW_KEY_O,
        e_p = GLFW_KEY_P,
        e_q = GLFW_KEY_Q,
        e_r = GLFW_KEY_R,
        e_s = GLFW_KEY_S,
        e_t = GLFW_KEY_T,
        e_u = GLFW_KEY_U,
        e_v = GLFW_KEY_V,
        e_w = GLFW_KEY_W,
        e_x = GLFW_KEY_X,
        e_y = GLFW_KEY_Y,
        e_z = GLFW_KEY_Z,
        e_left_bracket = GLFW_KEY_LEFT_BRACKET,
        e_backslash = GLFW_KEY_BACKSLASH,
        e_right_bracket = GLFW_KEY_RIGHT_BRACKET,
        e_grave_accent = GLFW_KEY_GRAVE_ACCENT,
        e_world_1 = GLFW_KEY_WORLD_1,
        e_world_2 = GLFW_KEY_WORLD_2,
        e_escape = GLFW_KEY_ESCAPE,
        e_enter = GLFW_KEY_ENTER,
        e_tab = GLFW_KEY_TAB,
        e_backspace = GLFW_KEY_BACKSPACE,
        e_insert = GLFW_KEY_INSERT,
        e_delete = GLFW_KEY_DELETE,
        e_right = GLFW_KEY_RIGHT,
        e_left = GLFW_KEY_LEFT,
        e_down = GLFW_KEY_DOWN,
        e_up = GLFW_KEY_UP,
        e_page_up = GLFW_KEY_PAGE_UP,
        e_page_down = GLFW_KEY_PAGE_DOWN,
        e_home = GLFW_KEY_HOME,
        e_end = GLFW_KEY_END,
        e_caps_lock = GLFW_KEY_CAPS_LOCK,
        e_scroll_lock = GLFW_KEY_SCROLL_LOCK,
        e_num_lock = GLFW_KEY_NUM_LOCK,
        e_print_screen = GLFW_KEY_PRINT_SCREEN,
        e_pause = GLFW_KEY_PAUSE,
        e_f1 = GLFW_KEY_F1,
        e_f2 = GLFW_KEY_F2,
        e_f3 = GLFW_KEY_F3,
        e_f4 = GLFW_KEY_F4,
        e_f5 = GLFW_KEY_F5,
        e_f6 = GLFW_KEY_F6,
        e_f7 = GLFW_KEY_F7,
        e_f8 = GLFW_KEY_F8,
        e_f9 = GLFW_KEY_F9,
        e_f10 = GLFW_KEY_F10,
        e_f11 = GLFW_KEY_F11,
        e_f12 = GLFW_KEY_F12,
        e_f13 = GLFW_KEY_F13,
        e_f14 = GLFW_KEY_F14,
        e_f15 = GLFW_KEY_F15,
        e_f16 = GLFW_KEY_F16,
        e_f17 = GLFW_KEY_F17,
        e_f18 = GLFW_KEY_F18,
        e_f19 = GLFW_KEY_F19,
        e_f20 = GLFW_KEY_F20,
        e_f21 = GLFW_KEY_F21,
        e_f22 = GLFW_KEY_F22,
        e_f23 = GLFW_KEY_F23,
        e_f24 = GLFW_KEY_F24,
        e_f25 = GLFW_KEY_F25,
        e_numpad_0 = GLFW_KEY_KP_0,
        e_numpad_1 = GLFW_KEY_KP_1,
        e_numpad_2 = GLFW_KEY_KP_2,
        e_numpad_3 = GLFW_KEY_KP_3,
        e_numpad_4 = GLFW_KEY_KP_4,
        e_numpad_5 = GLFW_KEY_KP_5,
        e_numpad_6 = GLFW_KEY_KP_6,
        e_numpad_7 = GLFW_KEY_KP_7,
        e_numpad_8 = GLFW_KEY_KP_8,
        e_numpad_9 = GLFW_KEY_KP_9,
        e_numpad_decimal = GLFW_KEY_KP_DECIMAL,
        e_numpad_divide = GLFW_KEY_KP_DIVIDE,
        e_numpad_multiply = GLFW_KEY_KP_MULTIPLY,
        e_numpad_subtract = GLFW_KEY_KP_SUBTRACT,
        e_numpad_add = GLFW_KEY_KP_ADD,
        e_numpad_enter = GLFW_KEY_KP_ENTER,
        e_numpad_equal = GLFW_KEY_KP_EQUAL,
        e_left_shift = GLFW_KEY_LEFT_SHIFT,
        e_left_control = GLFW_KEY_LEFT_CONTROL,
        e_left_alt = GLFW_KEY_LEFT_ALT,
        e_left_super = GLFW_KEY_LEFT_SUPER,
        e_right_shift = GLFW_KEY_RIGHT_SHIFT,
        e_right_control = GLFW_KEY_RIGHT_CONTROL,
        e_right_alt = GLFW_KEY_RIGHT_ALT,
        e_right_super = GLFW_KEY_RIGHT_SUPER,
        e_menu = GLFW_KEY_MENU,
        e_last = GLFW_KEY_LAST,
    };

    enum class InputMouseButton {
        e_1 = GLFW_MOUSE_BUTTON_1,
        e_2 = GLFW_MOUSE_BUTTON_2,
        e_3 = GLFW_MOUSE_BUTTON_3,
        e_4 = GLFW_MOUSE_BUTTON_4,
        e_5 = GLFW_MOUSE_BUTTON_5,
        e_6 = GLFW_MOUSE_BUTTON_6,
        e_7 = GLFW_MOUSE_BUTTON_7,
        e_8 = GLFW_MOUSE_BUTTON_8,
        e_last = GLFW_MOUSE_BUTTON_LAST,
        e_left = GLFW_MOUSE_BUTTON_LEFT,
        e_right = GLFW_MOUSE_BUTTON_RIGHT,
        e_middle = GLFW_MOUSE_BUTTON_MIDDLE,
    };

    enum class InputGamepadAxis {
        e_left_x = GLFW_GAMEPAD_AXIS_LEFT_X,
        e_left_y = GLFW_GAMEPAD_AXIS_LEFT_Y,
        e_right_x = GLFW_GAMEPAD_AXIS_RIGHT_X,
        e_right_y = GLFW_GAMEPAD_AXIS_RIGHT_Y,
        e_left_trigger = GLFW_GAMEPAD_AXIS_LEFT_TRIGGER,
        e_right_trigger = GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER,
        e_last = GLFW_GAMEPAD_AXIS_LAST,
    };

    enum class InputGamepadButton {
        e_a = GLFW_GAMEPAD_BUTTON_A,
        e_b = GLFW_GAMEPAD_BUTTON_B,
        e_x = GLFW_GAMEPAD_BUTTON_X,
        e_y = GLFW_GAMEPAD_BUTTON_Y,
        e_left_bumper = GLFW_GAMEPAD_BUTTON_LEFT_BUMPER,
        e_right_bumper = GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER,
        e_back = GLFW_GAMEPAD_BUTTON_BACK,
        e_start = GLFW_GAMEPAD_BUTTON_START,
        e_guide = GLFW_GAMEPAD_BUTTON_GUIDE,
        e_left_thumb = GLFW_GAMEPAD_BUTTON_LEFT_THUMB,
        e_right_thumb = GLFW_GAMEPAD_BUTTON_RIGHT_THUMB,
        e_dpad_up = GLFW_GAMEPAD_BUTTON_DPAD_UP,
        e_dpad_right = GLFW_GAMEPAD_BUTTON_DPAD_RIGHT,
        e_dpad_down = GLFW_GAMEPAD_BUTTON_DPAD_DOWN,
        e_dpad_left = GLFW_GAMEPAD_BUTTON_DPAD_LEFT,
        e_last = GLFW_GAMEPAD_BUTTON_LAST,
        e_cross = GLFW_GAMEPAD_BUTTON_CROSS,
        e_circle = GLFW_GAMEPAD_BUTTON_CIRCLE,
        e_square = GLFW_GAMEPAD_BUTTON_SQUARE,
        e_triangle = GLFW_GAMEPAD_BUTTON_TRIANGLE,
    };

    enum class InputJoystickHat {
        e_centered = GLFW_HAT_CENTERED,
        e_up = GLFW_HAT_UP,
        e_right = GLFW_HAT_RIGHT,
        e_down = GLFW_HAT_DOWN,
        e_left = GLFW_HAT_LEFT,
        e_right_up = GLFW_HAT_RIGHT_UP,
        e_right_down = GLFW_HAT_RIGHT_DOWN,
        e_left_up = GLFW_HAT_LEFT_UP,
        e_left_down = GLFW_HAT_LEFT_DOWN,
    };

    enum class InputJoystick {
        e_1 = GLFW_JOYSTICK_1,
        e_2 = GLFW_JOYSTICK_2,
        e_3 = GLFW_JOYSTICK_3,
        e_4 = GLFW_JOYSTICK_4,
        e_5 = GLFW_JOYSTICK_5,
        e_6 = GLFW_JOYSTICK_6,
        e_7 = GLFW_JOYSTICK_7,
        e_8 = GLFW_JOYSTICK_8,
        e_9 = GLFW_JOYSTICK_9,
        e_10 = GLFW_JOYSTICK_10,
        e_11 = GLFW_JOYSTICK_11,
        e_12 = GLFW_JOYSTICK_12,
        e_13 = GLFW_JOYSTICK_13,
        e_14 = GLFW_JOYSTICK_14,
        e_15 = GLFW_JOYSTICK_15,
        e_16 = GLFW_JOYSTICK_16,
        e_last = GLFW_JOYSTICK_LAST,
    };

    enum InputKeyModifierBits {
        e_shift = GLFW_MOD_SHIFT,
        e_control = GLFW_MOD_CONTROL,
        e_alt = GLFW_MOD_ALT,
        e_super = GLFW_MOD_SUPER,
        e_caps_lock = GLFW_MOD_CAPS_LOCK,
        e_num_lock = GLFW_MOD_NUM_LOCK,
    };

    enum InputState {
        e_release = 0,
        e_press = 1,
    };

    class Window {
    public:
        Window(const std::string &title, glm::ivec2 size);

        [[nodiscard]] GLFWwindow *get_glfw_handle() const;

        [[nodiscard]] bool was_resized() const;

        [[nodiscard]] glm::ivec2 get_size() const;

        [[nodiscard]] bool should_close() const;

        void set_resize_callback(const std::function<void(glm::ivec2)> &resize_callback);

        void remove_resize_callback();

        void update();

        void wait_for_events() const;

        InputState get_key(InputKey key);

        glm::vec2 get_cursor_pos(bool clamped_to_window = true);

    private:
        typedef std::unique_ptr<GLFWwindow, std::function<void(GLFWwindow *)>> UniqueGlfwWindow;

        UniqueGlfwWindow m_glfw_window;
        bool m_was_resized;
        std::optional<std::function<void(glm::ivec2)>> m_resize_callback;

        static void glfw_framebuffer_resize_callback(GLFWwindow *window, int width, int height);
    };
}