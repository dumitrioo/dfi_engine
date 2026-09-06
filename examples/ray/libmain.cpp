#include "../../src/dfi_interfaces.hpp"

#include <raylib.h>
#include <raymath.h>

class ray_ext: public dfi::extension_interface {
    dfi::valbox color_val(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255) {
        return dfi::valbox{Color{r, g, b, a}, "Color"};
    }

public:
    ray_ext() = default;
    ~ray_ext() {
        unregister_runtime();
    }
    ray_ext(ray_ext const &) = delete;
    ray_ext &operator=(ray_ext const &) = delete;
    ray_ext(ray_ext &&) = delete;
    ray_ext &operator=(ray_ext &&) = delete;

    void register_runtime(dfi::runtime_interface *rt) override {
        std::unique_lock l{rt_mtp_};
        if(rt_ != nullptr) {
            return;
        }
        rt_ = rt;
        if(rt_ == nullptr) {
            return;
        }
        rt->add_function("Vector2", DFIFUN(args) {
            DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2)
            return dfi::valbox{
                Vector2{
                    args.size() > 0 ? DFINUMARG(args, 0, float) : 0,
                    args.size() > 0 ? DFINUMARG(args, 1, float) : 0
                },
                "Vector2"
            };
        });
        rt->add_method("Vector2", "x", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return DFITHIS(args, Vector2).x; });
        rt->add_method("Vector2", "y", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return DFITHIS(args, Vector2).y; });
        rt->add_method("Vector2", "set_x", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2) DFITHIS(args, Vector2 &).x = DFINUMARG(args, 1, float); return {}; });
        rt->add_method("Vector2", "set_y", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2) DFITHIS(args, Vector2 &).y = DFINUMARG(args, 1, float); return {}; });

        rt->add_function("Vector3", DFIFUN(args) {
            DFI_CHCK_FUN_PARMS_NUM_EQ(args, 3)
            return dfi::valbox{
                Vector3{
                    args.size() > 0 ? DFINUMARG(args, 0, float) : 0,
                    args.size() > 0 ? DFINUMARG(args, 1, float) : 0,
                    args.size() > 0 ? DFINUMARG(args, 2, float) : 0
                },
                "Vector3"
            };
        });
        rt->add_method("Vector3", "x", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return DFITHIS(args, Vector2).x; });
        rt->add_method("Vector3", "y", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return DFITHIS(args, Vector2).y; });
        rt->add_method("Vector3", "z", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return DFITHIS(args, Vector2).y; });
        rt->add_method("Vector3", "set_x", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2) DFITHIS(args, Vector2 &).x = DFINUMARG(args, 1, float); return {}; });
        rt->add_method("Vector3", "set_y", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2) DFITHIS(args, Vector2 &).y = DFINUMARG(args, 1, float); return {}; });
        rt->add_method("Vector3", "set_z", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2) DFITHIS(args, Vector2 &).y = DFINUMARG(args, 1, float); return {}; });

        rt->add_function("Vector4", DFIFUN(args) {
            DFI_CHCK_FUN_PARMS_NUM_EQ(args, 4)
            return dfi::valbox{
                Vector4{
                    args.size() > 0 ? DFINUMARG(args, 0, float) : 0,
                    args.size() > 0 ? DFINUMARG(args, 1, float) : 0,
                    args.size() > 0 ? DFINUMARG(args, 2, float) : 0,
                    args.size() > 0 ? DFINUMARG(args, 3, float) : 0
                },
                "Vector4"
            };
        });
        rt->add_function("Quaternion", DFIFUN(args) {
            DFI_CHCK_FUN_PARMS_NUM_EQ(args, 4)
            return dfi::valbox{
                Vector4{
                    args.size() > 0 ? DFINUMARG(args, 0, float) : 0,
                    args.size() > 0 ? DFINUMARG(args, 1, float) : 0,
                    args.size() > 0 ? DFINUMARG(args, 2, float) : 0,
                    args.size() > 0 ? DFINUMARG(args, 3, float) : 0
                },
                "Vector4"
            };
        });
        rt->add_method("Vector4", "x", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return DFITHIS(args, Vector2).x; });
        rt->add_method("Vector4", "y", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return DFITHIS(args, Vector2).y; });
        rt->add_method("Vector4", "z", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return DFITHIS(args, Vector2).y; });
        rt->add_method("Vector4", "w", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return DFITHIS(args, Vector2).y; });
        rt->add_method("Vector4", "set_x", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2) DFITHIS(args, Vector2 &).x = DFINUMARG(args, 1, float); return {}; });
        rt->add_method("Vector4", "set_y", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2) DFITHIS(args, Vector2 &).y = DFINUMARG(args, 1, float); return {}; });
        rt->add_method("Vector4", "set_z", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2) DFITHIS(args, Vector2 &).y = DFINUMARG(args, 1, float); return {}; });
        rt->add_method("Vector4", "set_w", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2) DFITHIS(args, Vector2 &).y = DFINUMARG(args, 1, float); return {}; });

        rt->add_function("Color", DFIFUN(args) {
            DFI_CHCK_FUN_PARMS_NUM_EQ(args, 4)
            return dfi::valbox{
                Color{
                    args.size() > 0 ? DFINUMARG(args, 0, uint8_t) : (uint8_t)0,
                    args.size() > 1 ? DFINUMARG(args, 1, uint8_t) : (uint8_t)0,
                    args.size() > 2 ? DFINUMARG(args, 2, uint8_t) : (uint8_t)0,
                    args.size() > 3 ? DFINUMARG(args, 3, uint8_t) : (uint8_t)255
                },
                "Color"
            };
        });
        rt->add_method("Color", "r", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return DFITHIS(args, Color).r; });
        rt->add_method("Color", "g", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return DFITHIS(args, Color).g; });
        rt->add_method("Color", "b", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return DFITHIS(args, Color).b; });
        rt->add_method("Color", "a", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return DFITHIS(args, Color).a; });
        rt->add_method("Color", "set_r", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) DFITHIS(args, Color &).r = DFINUMARG(args, 1, uint8_t); return {}; });
        rt->add_method("Color", "set_g", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) DFITHIS(args, Color &).g = DFINUMARG(args, 1, uint8_t); return {}; });
        rt->add_method("Color", "set_b", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) DFITHIS(args, Color &).b = DFINUMARG(args, 1, uint8_t); return {}; });
        rt->add_method("Color", "set_a", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) DFITHIS(args, Color &).a = DFINUMARG(args, 1, uint8_t); return {}; });

        rt->add_var("LIGHTGRAY",  color_val(200, 200, 200));
        rt->add_var("GRAY",       color_val(130, 130, 130));
        rt->add_var("DARKGRAY",   color_val(80, 80, 80));
        rt->add_var("YELLOW",     color_val(253, 249, 0));
        rt->add_var("GOLD",       color_val(255, 203, 0));
        rt->add_var("ORANGE",     color_val(255, 161, 0));
        rt->add_var("PINK",       color_val(255, 109, 194));
        rt->add_var("RED",        color_val(230, 41, 55));
        rt->add_var("MAROON",     color_val(190, 33, 55));
        rt->add_var("GREEN",      color_val(0, 228, 48));
        rt->add_var("LIME",       color_val(0, 158, 47));
        rt->add_var("DARKGREEN",  color_val(0, 117, 44));
        rt->add_var("SKYBLUE",    color_val(102, 191, 255));
        rt->add_var("BLUE",       color_val(0, 121, 241));
        rt->add_var("DARKBLUE",   color_val(0, 82, 172));
        rt->add_var("PURPLE",     color_val(200, 122, 255));
        rt->add_var("VIOLET",     color_val(135, 60, 190));
        rt->add_var("DARKPURPLE", color_val(112, 31, 126));
        rt->add_var("BEIGE",      color_val(211, 176, 131));
        rt->add_var("BROWN",      color_val(127, 106, 79));
        rt->add_var("DARKBROWN",  color_val(76, 63, 47));
        rt->add_var("WHITE",      color_val(255, 255, 255));
        rt->add_var("BLACK",      color_val(0, 0, 0));
        rt->add_var("BLANK",      color_val(0, 0, 0));
        rt->add_var("MAGENTA",    color_val(255, 0, 255));
        rt->add_var("RAYWHITE",   color_val(245, 245, 245));


        rt->add_function("ray_get_screen_width", DFIFUN() { return GetScreenWidth(); });
        rt->add_function("ray_get_screen_height", DFIFUN() { return GetScreenHeight(); });
        rt->add_function("ray_get_monitor_count", DFIFUN() { return GetMonitorCount(); });
        rt->add_function("ray_get_current_monitor", DFIFUN() { return GetCurrentMonitor(); });
        rt->add_function("ray_get_monitor_width", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return GetMonitorWidth(DFINUMARG(args, 0, int)); });
        rt->add_function("ray_get_monitor_height", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return GetMonitorHeight(DFINUMARG(args, 0, int)); });
        rt->add_function("ray_get_monitor_physical_width", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return GetMonitorPhysicalWidth(DFINUMARG(args, 0, int)); });
        rt->add_function("ray_get_monitor_physical_height", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return GetMonitorPhysicalHeight(DFINUMARG(args, 0, int)); });


        rt->add_function("ray_init_window", DFIFUN(args) {
            InitWindow(
                args.size() > 0 ? DFINUMARG(args, 0, int) > 0 ? DFINUMARG(args, 0, int) : 800 : 800,
                args.size() > 1 ? DFINUMARG(args, 1, int) > 0 ? DFINUMARG(args, 1, int) : 600 : 600,
                args.size() > 2 ? args[2].cast_to_string().c_str() : "Raylib Window"
            );
            return dfi::valbox{};
        });
        rt->add_function("ray_maximize_window", DFIFUN() { MaximizeWindow(); return true; });
        rt->add_function("ray_minimize_window", DFIFUN() { MinimizeWindow(); return true; });
        rt->add_function("ray_window_should_close", DFIFUN() { return WindowShouldClose(); });
        rt->add_function("ray_begin_drawing", DFIFUN() { BeginDrawing(); return dfi::valbox{}; });
        rt->add_function("ray_end_drawing", DFIFUN() { EndDrawing(); return dfi::valbox{}; });
        rt->add_function("ray_close_window", DFIFUN() { CloseWindow(); return dfi::valbox{}; });
        rt->add_function("ray_clear_background", DFIFUN(args) {
            DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1)
            ClearBackground(DFITHIS(args, Color));
            return dfi::valbox{};
        });

        // SetTargetFPS
        rt->add_function("ray_set_target_fps", DFIFUN(args) {
            DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1)
            SetTargetFPS(args[0].cast_to_s32());
            return dfi::valbox{};
        });

        rt->add_function("ray_toggle_full_screen", DFIFUN() { ToggleFullscreen(); return dfi::valbox{}; });

        rt->add_function("ray_draw_pixel", DFIFUN(args) {
            DFI_CHCK_FUN_PARMS_NUM_EQ(args, 3)
            DrawPixel(DFINUMARG(args, 0, int), DFINUMARG(args, 1, int), DFICLASSARG(args, 2, Color));
            return dfi::valbox{};
        });
        rt->add_function("ray_draw_pixel_v", DFIFUN(args) {
            DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2)
            DrawPixelV(DFITHIS(args, Vector2), DFICLASSARG(args, 1, Color)); return dfi::valbox{};
        });

        rt->add_function("ray_draw_line", DFIFUN(args) {
            DFI_CHCK_FUN_PARMS_NUM_EQ(args, 5)
            DrawLine(DFINUMARG(args, 0, int), DFINUMARG(args, 1, int), DFINUMARG(args, 2, int), DFINUMARG(args, 3, int), DFICLASSARG(args, 4, Color)); return dfi::valbox{};
        });
        rt->add_function("ray_draw_line_v", DFIFUN(args) {
            DFI_CHCK_FUN_PARMS_NUM_EQ(args, 3)
            DrawLineV(DFITHIS(args, Vector2), DFICLASSARG(args, 1, Vector2), DFICLASSARG(args, 2, Color)); return dfi::valbox{};
        });
        rt->add_function("ray_draw_line_ex", DFIFUN(args) {
            DFI_CHCK_FUN_PARMS_NUM_EQ(args, 4)
            DrawLineEx(DFITHIS(args, Vector2), DFICLASSARG(args, 1, Vector2), DFINUMARG(args, 2, float), DFICLASSARG(args, 3, Color)); return dfi::valbox{};
        });

        rt->add_function("ray_draw_line_strip", DFIFUN(args) {
            DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2)
            auto &&vec2arr{args[0].as_array()};
            std::vector<Vector2> pts{};
            pts.reserve(vec2arr.size());
            for(auto &&v: vec2arr) {
                pts.push_back(v.as_class<Vector2>());
            }
            DrawLineStrip(pts.data(), pts.size(), DFICLASSARG(args, 1, Color));
            return {};
        });


        rt->add_function("ray_draw_line_bezier", DFIFUN(args) {
            DFI_CHCK_FUN_PARMS_NUM_EQ(args, 4)
            DrawLineBezier(DFITHIS(args, Vector2), DFICLASSARG(args, 1, Vector2), DFINUMARG(args, 2, int), DFICLASSARG(args, 3, Color)); return dfi::valbox{};
        });
        rt->add_function("ray_draw_circle", DFIFUN(args) {
            DFI_CHCK_FUN_PARMS_NUM_EQ(args, 4)
            DrawCircle(DFINUMARG(args, 0, int), DFINUMARG(args, 1, int), DFINUMARG(args, 2, float), DFICLASSARG(args, 3, Color)); return dfi::valbox{};
        });

        rt->add_function("ray_draw_text", DFIFUN(args) {
            DFI_CHCK_FUN_PARMS_NUM_EQ(args, 5)
            DrawText(
                args[0].cast_to_string().c_str(),
                DFINUMARG(args, 1, int), DFINUMARG(args, 2, int),
                DFINUMARG(args, 3, int),
                DFICLASSARG(args, 4, Color)
            );
            return dfi::valbox{};
        });

        rt->add_function("ray_draw_rectangle", DFIFUN(args) {
            DFI_CHCK_FUN_PARMS_NUM_EQ(args, 5)
            DrawRectangle(DFINUMARG(args, 0, int), DFINUMARG(args, 1, int), DFINUMARG(args, 2, int), DFINUMARG(args, 3, int), DFICLASSARG(args, 4, Color)); return dfi::valbox{};
        });

        rt->add_function("ray_is_key_pressed", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return IsKeyPressed(DFINUMARG(args, 0, int)); });
        rt->add_function("ray_is_key_pressed_repeat", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return IsKeyPressedRepeat(DFINUMARG(args, 0, int)); });
        rt->add_function("ray_is_key_down", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return IsKeyDown(DFINUMARG(args, 0, int)); });
        rt->add_function("ray_is_key_released", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return IsKeyReleased(DFINUMARG(args, 0, int)); });
        rt->add_function("ray_is_key_up", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return IsKeyUp(DFINUMARG(args, 0, int)); });
        rt->add_function("ray_get_key_pressed", DFIFUN() { return GetKeyPressed(); });
        rt->add_function("ray_get_char_pressed", DFIFUN() { return GetCharPressed(); });
        rt->add_function("ray_set_exit_key", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) SetExitKey(DFINUMARG(args, 0, int)); return {}; });
        rt->add_function("ray_is_gamepad_available", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return IsGamepadAvailable(DFINUMARG(args, 0, int)); });
        rt->add_function("ray_get_gamepad_name", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return std::string{GetGamepadName(DFINUMARG(args, 0, int))}; });
        rt->add_function("ray_is_gamepad_button_pressed", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2) return IsGamepadButtonPressed(DFINUMARG(args, 0, int), DFINUMARG(args, 1, int)); });
        rt->add_function("ray_is_gamepad_button_down", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2) return IsGamepadButtonDown(DFINUMARG(args, 0, int), DFINUMARG(args, 1, int)); });
        rt->add_function("ray_is_gamepad_button_released", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2) return IsGamepadButtonReleased(DFINUMARG(args, 0, int), DFINUMARG(args, 1, int)); });
        rt->add_function("ray_is_gamepad_button_up", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2) return IsGamepadButtonUp(DFINUMARG(args, 0, int), DFINUMARG(args, 1, int)); });
        rt->add_function("ray_get_gamepad_button_pressed", DFIFUN() { return GetGamepadButtonPressed(); });
        rt->add_function("ray_get_gamepad_axis_count", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return GetGamepadAxisCount(DFINUMARG(args, 0, int)); });
        rt->add_function("ray_get_gamepad_axis_movement", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2) return GetGamepadAxisMovement(DFINUMARG(args, 0, int), DFINUMARG(args, 1, int)); });
        rt->add_function("ray_set_gamepad_mappings", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return SetGamepadMappings(args[0].cast_to_string().c_str()); });
        rt->add_function("ray_set_gamepad_vibration", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 4) SetGamepadVibration(DFINUMARG(args, 0, int), DFINUMARG(args, 1, float), DFINUMARG(args, 2, float), DFINUMARG(args, 3, float)); return {}; });

        rt->add_function("ray_is_mouse_button_pressed", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return IsMouseButtonPressed(DFINUMARG(args, 0, int)); });
        rt->add_function("ray_is_mouse_button_down", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return IsMouseButtonDown(DFINUMARG(args, 0, int)); });
        rt->add_function("ray_is_mouse_button_released", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return IsMouseButtonReleased(DFINUMARG(args, 0, int)); });
        rt->add_function("ray_is_mouse_button_up", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return IsMouseButtonUp(DFINUMARG(args, 0, int)); });
        rt->add_function("ray_get_mouse_x", DFIFUN() { return GetMouseX(); });
        rt->add_function("ray_get_mouse_y", DFIFUN() { return GetMouseY(); });
        rt->add_function("ray_get_mouse_position", DFIFUN() { return dfi::valbox{GetMousePosition(), "Vector2"}; });
        rt->add_function("ray_get_mouse_delta", DFIFUN() { return dfi::valbox{GetMouseDelta(), "Vector2"}; });
        rt->add_function("ray_set_mouse_position", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2) SetMousePosition(DFINUMARG(args, 0, int), DFINUMARG(args, 1, int)); return {}; });
        rt->add_function("ray_set_mouse_offset", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2) SetMouseOffset(DFINUMARG(args, 0, int), DFINUMARG(args, 1, int)); return {}; });
        rt->add_function("ray_set_mouse_scale", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2) SetMouseScale(DFINUMARG(args, 0, float), DFINUMARG(args, 1, float)); return {}; });
        rt->add_function("ray_get_mouse_wheel_move", DFIFUN() { return GetMouseWheelMove(); });
        rt->add_function("ray_get_mouse_wheel_move_v", DFIFUN() { return dfi::valbox{GetMouseWheelMoveV(), "Vector2"}; });
        rt->add_function("ray_set_mouse_cursor", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) SetMouseCursor(DFINUMARG(args, 0, int)); return {}; });
        rt->add_function("ray_get_touch_x", DFIFUN() { return GetTouchX(); });
        rt->add_function("ray_get_touch_y", DFIFUN() { return GetTouchY(); });
        rt->add_function("ray_get_touch_position", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return dfi::valbox{GetTouchPosition(DFINUMARG(args, 0, int)), "Vector2"}; });
        rt->add_function("ray_get_touch_point_id", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return GetTouchPointId(DFINUMARG(args, 0, int)); });
        rt->add_function("ray_get_touch_point_count", DFIFUN() { return GetTouchPointCount(); });
    }

    void unregister_runtime() override {
        std::unique_lock l{rt_mtp_};
        if(rt_ == nullptr) {
            return;
        }
        rt_->remove_function("Vector2");
        rt_->remove_method("Vector2", "x");
        rt_->remove_method("Vector2", "y");
        rt_->remove_method("Vector2", "set_x");
        rt_->remove_method("Vector2", "set_y");
        rt_->remove_function("Vector3");
        rt_->remove_method("Vector3", "x");
        rt_->remove_method("Vector3", "y");
        rt_->remove_method("Vector3", "z");
        rt_->remove_method("Vector3", "set_x");
        rt_->remove_method("Vector3", "set_y");
        rt_->remove_method("Vector3", "set_z");

        rt_->remove_function("Vector4");
        rt_->remove_function("Quaternion");
        rt_->remove_method("Vector4", "x");
        rt_->remove_method("Vector4", "y");
        rt_->remove_method("Vector4", "z");
        rt_->remove_method("Vector4", "w");
        rt_->remove_method("Vector4", "set_x");
        rt_->remove_method("Vector4", "set_y");
        rt_->remove_method("Vector4", "set_z");
        rt_->remove_method("Vector4", "set_w");

        rt_->remove_function("Color");
        rt_->remove_method("Color", "r");
        rt_->remove_method("Color", "g");
        rt_->remove_method("Color", "b");
        rt_->remove_method("Color", "a");
        rt_->remove_method("Color", "set_r");
        rt_->remove_method("Color", "set_g");
        rt_->remove_method("Color", "set_b");
        rt_->remove_method("Color", "set_a");

        rt_->remove_var("LIGHTGRAY");
        rt_->remove_var("GRAY");
        rt_->remove_var("DARKGRAY");
        rt_->remove_var("YELLOW");
        rt_->remove_var("GOLD");
        rt_->remove_var("ORANGE");
        rt_->remove_var("PINK");
        rt_->remove_var("RED");
        rt_->remove_var("MAROON");
        rt_->remove_var("GREEN");
        rt_->remove_var("LIME");
        rt_->remove_var("DARKGREEN");
        rt_->remove_var("SKYBLUE");
        rt_->remove_var("BLUE");
        rt_->remove_var("DARKBLUE");
        rt_->remove_var("PURPLE");
        rt_->remove_var("VIOLET");
        rt_->remove_var("DARKPURPLE");
        rt_->remove_var("BEIGE");
        rt_->remove_var("BROWN");
        rt_->remove_var("DARKBROWN");
        rt_->remove_var("WHITE");
        rt_->remove_var("BLACK");
        rt_->remove_var("BLANK");
        rt_->remove_var("MAGENTA");
        rt_->remove_var("RAYWHITE");

        rt_->remove_function("ray_get_screen_width");
        rt_->remove_function("ray_get_screen_height");
        rt_->remove_function("ray_get_monitor_count");
        rt_->remove_function("ray_get_current_monitor");
        rt_->remove_function("ray_get_monitor_width");
        rt_->remove_function("ray_get_monitor_height");

        rt_->remove_function("ray_init_window");
        rt_->remove_function("ray_window_should_close");
        rt_->remove_function("ray_begin_drawing");
        rt_->remove_function("ray_end_drawing");
        rt_->remove_function("ray_close_window");
        rt_->remove_function("ray_clear_background");

        rt_->remove_function("ray_set_target_fps");
        rt_->remove_function("ray_toggle_full_screen");
        rt_->remove_function("ray_draw_pixel");
        rt_->remove_function("ray_draw_pixel_v");
        rt_->remove_function("ray_draw_line");
        rt_->remove_function("ray_draw_line_v");
        rt_->remove_function("ray_draw_line_ex");
        rt_->remove_function("ray_draw_line_strip");
        rt_->remove_function("ray_draw_line_bezier");
        rt_->remove_function("ray_draw_circle");
        rt_->remove_function("ray_draw_text");
        rt_->remove_function("ray_draw_rectangle");

        rt_->remove_function("ray_is_key_pressed");
        rt_->remove_function("ray_is_key_pressed_repeat");
        rt_->remove_function("ray_is_key_down");
        rt_->remove_function("ray_is_key_released");
        rt_->remove_function("ray_is_key_up");
        rt_->remove_function("ray_get_key_pressed");
        rt_->remove_function("ray_get_char_pressed");
        rt_->remove_function("ray_set_exit_key");
        rt_->remove_function("ray_is_gamepad_available");
        rt_->remove_function("ray_get_gamepad_name");
        rt_->remove_function("ray_is_gamepad_button_pressed");
        rt_->remove_function("ray_is_gamepad_button_down");
        rt_->remove_function("ray_is_gamepad_button_released");
        rt_->remove_function("ray_is_gamepad_button_up");
        rt_->remove_function("ray_get_gamepad_button_pressed");
        rt_->remove_function("ray_get_gamepad_axis_count");
        rt_->remove_function("ray_get_gamepad_axis_movement");
        rt_->remove_function("ray_set_gamepad_mappings");
        rt_->remove_function("ray_set_gamepad_vibration");

        rt_->remove_function("ray_is_mouse_button_pressed");
        rt_->remove_function("ray_is_mouse_button_down");
        rt_->remove_function("ray_is_mouse_button_released");
        rt_->remove_function("ray_is_mouse_button_up");
        rt_->remove_function("ray_get_mouse_x");
        rt_->remove_function("ray_get_mouse_y");
        rt_->remove_function("ray_get_mouse_position");
        rt_->remove_function("ray_get_mouse_delta");
        rt_->remove_function("ray_set_mouse_position");
        rt_->remove_function("ray_set_mouse_offset");
        rt_->remove_function("ray_set_mouse_scale");
        rt_->remove_function("ray_get_mouse_wheel_move");
        rt_->remove_function("ray_get_mouse_wheel_move_v");
        rt_->remove_function("ray_set_mouse_cursor");
        rt_->remove_function("ray_get_touch_x");
        rt_->remove_function("ray_get_touch_y");
        rt_->remove_function("ray_get_touch_position");
        rt_->remove_function("ray_get_touch_point_id");
        rt_->remove_function("ray_get_touch_point_count");

        rt_ = nullptr;
    }

private:
    dfi::shared_mutex rt_mtp_{};
    dfi::runtime_interface *rt_{nullptr};
};

dfi::extension_interface *create_dfi_extension() {
    static ray_ext extinst{};
    return &extinst;
}

void remove_dfi_extension(dfi::extension_interface *) {
}
