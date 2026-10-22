#include "../../src/dfi_interfaces.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_main.h>

class sdl3_ext: public dfi::extension_interface {
    dfi::valbox color_val(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255) {
        return dfi::valbox{SDL_Color{r, g, b, a}, "gfx_color"};
    }
    struct SDL_Surface_Deleter { void operator()(SDL_Surface* s) { SDL_DestroySurface(s); }};
    template <typename ...Args>
    static std::shared_ptr<SDL_Surface> make_sdl_surface(Args &&...args) { return std::shared_ptr<SDL_Surface>(SDL_CreateSurface(std::forward<Args>(args)...), SDL_Surface_Deleter{}); }
    struct SDL_Window_Deleter { void operator()(SDL_Window* w) { SDL_DestroyWindow(w); }};
    template <typename ...Args>
    static std::shared_ptr<SDL_Window> make_sdl_window(Args &&...args) { return std::shared_ptr<SDL_Window>(SDL_CreateWindow(std::forward<Args>(args)...), SDL_Window_Deleter{}); }

    class window_render final {
    public:
        window_render(const char *title, int width, int height, SDL_WindowFlags window_flags) {
            SDL_Window *windo{nullptr};
            SDL_Renderer *rendere{nullptr};

            if (!SDL_CreateWindowAndRenderer(title, width, height, window_flags, &windo, &rendere)) {
                throw std::runtime_error{"error creating gfx window"};
            }
            window_ = std::shared_ptr<SDL_Window>{windo, SDL_DestroyWindow};
            renderer_ = std::shared_ptr<SDL_Renderer>{rendere, SDL_DestroyRenderer};
            if(!SDL_SetRenderLogicalPresentation(renderer_.get(), width, height, SDL_LOGICAL_PRESENTATION_STRETCH/*SDL_LOGICAL_PRESENTATION_LETTERBOX*/)) {
                window_.reset();
                renderer_.reset();
                throw std::runtime_error{"error creating gfx window"};
            }
        }

        window_render(window_render const &) = delete;
        window_render(window_render &&that):
            window_{that.window_},
            renderer_{that.renderer_}
        {
            that.window_ = nullptr;
            that.renderer_ = nullptr;
        }
        window_render &operator=(window_render const &) = delete;
        window_render &operator=(window_render &&that)
        {
            if(this != &that) {
                std::swap(window_, that.window_);
                std::swap(renderer_, that.renderer_);
            }
            return *this;
        }
        ~window_render() {
            // if(renderer_) { SDL_DestroyRenderer(renderer_); }
            // if(window_) { SDL_DestroyWindow(window_); }
        }

        SDL_Window *window_ptr() const noexcept { return window_.get(); }
        SDL_Window *checked_window_ptr() const { if(window_ == nullptr) { throw std::runtime_error{"invalid window object"}; } return window_.get(); }
        SDL_Window &window() {
            if(window_ == nullptr) {
                throw std::runtime_error{"invalid window object"};
            }
            return *window_;
        }

        SDL_Renderer *renderer_ptr() const noexcept { return renderer_.get(); }
        SDL_Renderer *checked_renderer_ptr() const { if(renderer_ == nullptr) { throw std::runtime_error{"invalid window object"}; } return renderer_.get(); }
        SDL_Renderer &renderer() {
            if(renderer_ == nullptr) {
                throw std::runtime_error{"invalid window object"};
            }
            return *renderer_;
        }

        SDL_WindowID id() const { return SDL_GetWindowID((SDL_Window *)checked_window_ptr()); }
        bool show() { return SDL_ShowWindow(checked_window_ptr()); }
        std::pair<int, int> size() const {
            int w, h;
            if(SDL_GetCurrentRenderOutputSize(checked_renderer_ptr(), &w, &h)) { return {w, h}; }
            throw std::runtime_error{std::string{"error "} + SDL_GetError()};
        }
        auto width() const { return size().first; }
        auto height() const { return size().second; }
        bool display() {
            if(!SDL_RenderPresent(checked_renderer_ptr())) { return false; }
            return true;
        }
        bool clear_background(SDL_Color const &clr) {
            if(!SDL_SetRenderDrawColor(checked_renderer_ptr(), clr.r, clr.g, clr.b, clr.a)) { return false; }
            if(!SDL_RenderClear(checked_renderer_ptr())) { return false; }
            return true;
        }
        bool line(float x1, float y1, float x2, float y2, SDL_Color const &clr) {
            if(!SDL_SetRenderDrawColor(checked_renderer_ptr(), clr.r, clr.g, clr.b, clr.a)) { return false; }
            if(!SDL_RenderLine(checked_renderer_ptr(),x1, y1, x2, y2)) { return false; }
            return true;
        }

    private:
        mutable std::shared_mutex mtp_{};
        std::shared_ptr<SDL_Window> window_{};
        std::shared_ptr<SDL_Renderer> renderer_{};
    };

public:
    sdl3_ext() = default;
    ~sdl3_ext() {
        unregister_runtime();
    }
    sdl3_ext(sdl3_ext const &) = delete;
    sdl3_ext &operator=(sdl3_ext const &) = delete;
    sdl3_ext(sdl3_ext &&) = delete;
    sdl3_ext &operator=(sdl3_ext &&) = delete;

    void register_runtime(dfi::runtime_interface *rt) override {
        std::unique_lock l{rt_mtp_};
        if(rt_ != nullptr) {
            return;
        }
        rt_ = rt;
        if(rt_ == nullptr) {
            return;
        }

        add_events(rt);
        add_keys_scancodes(rt);
        add_misc_constants(rt);

        // rt->add_function("Vector2", DFIFUN(args) {
        //     DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2)
        //     return dfi::valbox{
        //         Vector2{
        //             args.size() > 0 ? DFINUMARG(args, 0, float) : 0,
        //             args.size() > 0 ? DFINUMARG(args, 1, float) : 0
        //         },
        //         "Vector2"
        //     };
        // });
        // rt->add_method("Vector2", "x", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return DFITHIS(args, Vector2).x; });
        // rt->add_method("Vector2", "y", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return DFITHIS(args, Vector2).y; });
        // rt->add_method("Vector2", "set_x", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2) DFITHIS(args, Vector2 &).x = DFINUMARG(args, 1, float); return {}; });
        // rt->add_method("Vector2", "set_y", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2) DFITHIS(args, Vector2 &).y = DFINUMARG(args, 1, float); return {}; });

        // rt->add_function("Vector3", DFIFUN(args) {
        //     DFI_CHCK_FUN_PARMS_NUM_EQ(args, 3)
        //     return dfi::valbox{
        //         Vector3{
        //             args.size() > 0 ? DFINUMARG(args, 0, float) : 0,
        //             args.size() > 0 ? DFINUMARG(args, 1, float) : 0,
        //             args.size() > 0 ? DFINUMARG(args, 2, float) : 0
        //         },
        //         "Vector3"
        //     };
        // });
        // rt->add_method("Vector3", "x", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return DFITHIS(args, Vector2).x; });
        // rt->add_method("Vector3", "y", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return DFITHIS(args, Vector2).y; });
        // rt->add_method("Vector3", "z", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return DFITHIS(args, Vector2).y; });
        // rt->add_method("Vector3", "set_x", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2) DFITHIS(args, Vector2 &).x = DFINUMARG(args, 1, float); return {}; });
        // rt->add_method("Vector3", "set_y", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2) DFITHIS(args, Vector2 &).y = DFINUMARG(args, 1, float); return {}; });
        // rt->add_method("Vector3", "set_z", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2) DFITHIS(args, Vector2 &).y = DFINUMARG(args, 1, float); return {}; });

        // rt->add_function("Vector4", DFIFUN(args) {
        //     DFI_CHCK_FUN_PARMS_NUM_EQ(args, 4)
        //     return dfi::valbox{
        //         Vector4{
        //             args.size() > 0 ? DFINUMARG(args, 0, float) : 0,
        //             args.size() > 0 ? DFINUMARG(args, 1, float) : 0,
        //             args.size() > 0 ? DFINUMARG(args, 2, float) : 0,
        //             args.size() > 0 ? DFINUMARG(args, 3, float) : 0
        //         },
        //         "Vector4"
        //     };
        // });
        // rt->add_function("Quaternion", DFIFUN(args) {
        //     DFI_CHCK_FUN_PARMS_NUM_EQ(args, 4)
        //     return dfi::valbox{
        //         Vector4{
        //             args.size() > 0 ? DFINUMARG(args, 0, float) : 0,
        //             args.size() > 0 ? DFINUMARG(args, 1, float) : 0,
        //             args.size() > 0 ? DFINUMARG(args, 2, float) : 0,
        //             args.size() > 0 ? DFINUMARG(args, 3, float) : 0
        //         },
        //         "Vector4"
        //     };
        // });
        // rt->add_method("Vector4", "x", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return DFITHIS(args, Vector2).x; });
        // rt->add_method("Vector4", "y", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return DFITHIS(args, Vector2).y; });
        // rt->add_method("Vector4", "z", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return DFITHIS(args, Vector2).y; });
        // rt->add_method("Vector4", "w", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return DFITHIS(args, Vector2).y; });
        // rt->add_method("Vector4", "set_x", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2) DFITHIS(args, Vector2 &).x = DFINUMARG(args, 1, float); return {}; });
        // rt->add_method("Vector4", "set_y", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2) DFITHIS(args, Vector2 &).y = DFINUMARG(args, 1, float); return {}; });
        // rt->add_method("Vector4", "set_z", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2) DFITHIS(args, Vector2 &).y = DFINUMARG(args, 1, float); return {}; });
        // rt->add_method("Vector4", "set_w", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2) DFITHIS(args, Vector2 &).y = DFINUMARG(args, 1, float); return {}; });

        rt->add_function("gfx_color", DFIFUN(args) {
            DFI_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 3, 4)
            if(args.size() == 4) {
                return dfi::valbox{
                    SDL_Color{
                        DFINUMARG(args, 0, uint8_t),
                        DFINUMARG(args, 1, uint8_t),
                        DFINUMARG(args, 2, uint8_t),
                        DFINUMARG(args, 3, uint8_t)
                    },
                    "gfx_color"
                };
            } else {
                return dfi::valbox{
                    SDL_Color{
                        DFINUMARG(args, 0, uint8_t),
                        DFINUMARG(args, 1, uint8_t),
                        DFINUMARG(args, 2, uint8_t),
                        255
                    },
                    "gfx_color"
                };
            }
        });
        rt->add_method("gfx_color", "r", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return DFITHIS(args, SDL_Color).r; });
        rt->add_method("gfx_color", "g", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return DFITHIS(args, SDL_Color).g; });
        rt->add_method("gfx_color", "b", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return DFITHIS(args, SDL_Color).b; });
        rt->add_method("gfx_color", "a", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) return DFITHIS(args, SDL_Color).a; });
        rt->add_method("gfx_color", "set_r", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) DFITHIS(args, SDL_Color &).r = DFINUMARG(args, 1, uint8_t); return {}; });
        rt->add_method("gfx_color", "set_g", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) DFITHIS(args, SDL_Color &).g = DFINUMARG(args, 1, uint8_t); return {}; });
        rt->add_method("gfx_color", "set_b", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) DFITHIS(args, SDL_Color &).b = DFINUMARG(args, 1, uint8_t); return {}; });
        rt->add_method("gfx_color", "set_a", DFIFUN(args) { DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1) DFITHIS(args, SDL_Color &).a = DFINUMARG(args, 1, uint8_t); return {}; });

        rt->add_function("gfx_init", DFIFUN(args) {
            DFI_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 0, 1)
            return SDL_Init(static_cast<SDL_InitFlags>(args.size() > 0 ? DFINUMARG(args, 0, uint32_t) : 0));
        });

        rt->add_function("gfx_done", DFIFUN(args) {
            DFI_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 0, 1)
            SDL_Quit();
            return true;
        });

        rt->add_function("gfx_last_error", DFIFUN(args) {
            DFI_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 0, 1)
            return std::string{SDL_GetError()};
        });



        rt->add_function("gfx_poll", DFIFUN(args) {
            DFI_CHCK_FUN_PARMS_NUM_EQ(args, 0)
            SDL_Event evt{};
            dfi::valbox res{};
            if(SDL_PollEvent(&evt)) {
                res.become_object();
                dfi::valbox::object_t &eo{res.as_object()};
                eo["type"] = evt.type;
                switch(evt.type) {
                    case SDL_EVENT_QUIT: break;
                    case SDL_EVENT_TERMINATING: break;
                    case SDL_EVENT_LOW_MEMORY: break;
                    case SDL_EVENT_WILL_ENTER_BACKGROUND: break;
                    case SDL_EVENT_DID_ENTER_BACKGROUND: break;
                    case SDL_EVENT_WILL_ENTER_FOREGROUND: break;
                    case SDL_EVENT_DID_ENTER_FOREGROUND: break;
                    case SDL_EVENT_LOCALE_CHANGED: break;
                    case SDL_EVENT_SYSTEM_THEME_CHANGED: break;
                    case SDL_EVENT_DISPLAY_ORIENTATION:
                    case SDL_EVENT_DISPLAY_ADDED:
                    case SDL_EVENT_DISPLAY_REMOVED:
                    case SDL_EVENT_DISPLAY_MOVED:
                    case SDL_EVENT_DISPLAY_DESKTOP_MODE_CHANGED:
                    case SDL_EVENT_DISPLAY_CURRENT_MODE_CHANGED:
                    case SDL_EVENT_DISPLAY_CONTENT_SCALE_CHANGED:
                    case SDL_EVENT_DISPLAY_USABLE_BOUNDS_CHANGED:
                        eo["display"].become_object();
                        eo["display"].as_object()["timestamp"] = evt.display.timestamp;
                        eo["display"].as_object()["displayID"] = evt.display.displayID;
                        eo["display"].as_object()["data1"] = evt.display.data1;
                        eo["display"].as_object()["data2"] = evt.display.data2;
                        break;
                    // case SDL_EVENT_DISPLAY_FIRST: break;
                    // case SDL_EVENT_DISPLAY_LAST: break;
                    case SDL_EVENT_WINDOW_SHOWN:
                    case SDL_EVENT_WINDOW_HIDDEN:
                    case SDL_EVENT_WINDOW_EXPOSED:
                    case SDL_EVENT_WINDOW_MOVED:
                    case SDL_EVENT_WINDOW_RESIZED:
                    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                    case SDL_EVENT_WINDOW_METAL_VIEW_RESIZED:
                    case SDL_EVENT_WINDOW_MINIMIZED:
                    case SDL_EVENT_WINDOW_MAXIMIZED:
                    case SDL_EVENT_WINDOW_RESTORED:
                    case SDL_EVENT_WINDOW_MOUSE_ENTER:
                    case SDL_EVENT_WINDOW_MOUSE_LEAVE:
                    case SDL_EVENT_WINDOW_FOCUS_GAINED:
                    case SDL_EVENT_WINDOW_FOCUS_LOST:
                    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                    case SDL_EVENT_WINDOW_HIT_TEST:
                    case SDL_EVENT_WINDOW_ICCPROF_CHANGED:
                    case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
                    case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
                    case SDL_EVENT_WINDOW_SAFE_AREA_CHANGED:
                    case SDL_EVENT_WINDOW_OCCLUDED:
                    case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
                    case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
                    case SDL_EVENT_WINDOW_DESTROYED:
                    case SDL_EVENT_WINDOW_HDR_STATE_CHANGED:
                        eo["window"].become_object();
                        eo["window"].as_object()["timestamp"] = evt.window.timestamp;
                        eo["window"].as_object()["windowID"] = evt.window.windowID;
                        eo["window"].as_object()["data1"] = evt.window.data1;
                        eo["window"].as_object()["data2"] = evt.window.data2;
                        break;
                    // case SDL_EVENT_WINDOW_FIRST: break;
                    // case SDL_EVENT_WINDOW_LAST: break;
                    case SDL_EVENT_KEY_DOWN:
                    case SDL_EVENT_KEY_UP:
                        eo["key"].become_object();
                        eo["key"].as_object()["reserved"] = evt.key.reserved;
                        eo["key"].as_object()["timestamp"] = evt.key.timestamp;
                        eo["key"].as_object()["windowID"] = evt.key.windowID;
                        eo["key"].as_object()["which"] = evt.key.which;
                        eo["key"].as_object()["scancode"] = evt.key.scancode;
                        eo["key"].as_object()["key"] = evt.key.key;
                        eo["key"].as_object()["mod"] = evt.key.mod;
                        eo["key"].as_object()["raw"] = evt.key.raw;
                        eo["key"].as_object()["down"] = evt.key.down;
                        eo["key"].as_object()["repeat"] = evt.key.repeat;
                        break;
                    case SDL_EVENT_TEXT_EDITING:
                        eo["edit"].become_object();
                        eo["edit"].as_object()["timestamp"] = evt.edit.timestamp;
                        eo["edit"].as_object()["windowID"] = evt.edit.windowID;
                        eo["edit"].as_object()["text"] = std::string{evt.edit.text};
                        eo["edit"].as_object()["start"] = evt.edit.start;
                        eo["edit"].as_object()["length"] = evt.edit.length;
                        break;
                    case SDL_EVENT_TEXT_INPUT:
                        eo["text"].become_object();
                        eo["text"].as_object()["timestamp"] = evt.text.timestamp;
                        eo["text"].as_object()["windowID"] = evt.text.windowID;
                        eo["text"].as_object()["text"] = std::string{evt.text.text};
                        break;
                    case SDL_EVENT_KEYMAP_CHANGED: break;
                    case SDL_EVENT_KEYBOARD_ADDED:
                    case SDL_EVENT_KEYBOARD_REMOVED:
                        eo["kdevice"].become_object();
                        eo["kdevice"].as_object()["timestamp"] = evt.kdevice.timestamp;
                        eo["kdevice"].as_object()["which"] = evt.kdevice.which;
                        break;
                    case SDL_EVENT_TEXT_EDITING_CANDIDATES:
                        eo["edit_candidates"].become_object();
                        eo["edit_candidates"].as_object()["timestamp"] = evt.edit_candidates.timestamp;
                        eo["edit_candidates"].as_object()["windowID"] = evt.edit_candidates.windowID;
                        eo["edit_candidates"].as_object()["candidates"].become_array();
                        for(int i{0}; i < evt.edit_candidates.num_candidates; ++i) {
                            eo["edit_candidates"].as_object()["candidates"].as_array().push_back(std::string{evt.edit_candidates.candidates[i]});
                        }
                        eo["edit_candidates"].as_object()["num_candidates"] = evt.edit_candidates.num_candidates;
                        eo["edit_candidates"].as_object()["selected_candidate"] = evt.edit_candidates.selected_candidate;
                        eo["edit_candidates"].as_object()["horizontal"] = evt.edit_candidates.horizontal;
                        eo["edit_candidates"].as_object()["padding1"] = (int)evt.edit_candidates.padding1;
                        eo["edit_candidates"].as_object()["padding2"] = (int)evt.edit_candidates.padding2;
                        eo["edit_candidates"].as_object()["padding3"] = (int)evt.edit_candidates.padding3;
                        break;
                    case SDL_EVENT_SCREEN_KEYBOARD_SHOWN: break;
                    case SDL_EVENT_SCREEN_KEYBOARD_HIDDEN: break;
                    case SDL_EVENT_MOUSE_MOTION:
                        eo["motion"].become_object();
                        eo["motion"].as_object()["timestamp"] = evt.motion.timestamp;
                        eo["motion"].as_object()["windowID"] = evt.motion.windowID;
                        eo["motion"].as_object()["which"] = evt.motion.which;
                        eo["motion"].as_object()["state"] = evt.motion.state;
                        eo["motion"].as_object()["x"] = evt.motion.x;
                        eo["motion"].as_object()["y"] = evt.motion.y;
                        eo["motion"].as_object()["xrel"] = evt.motion.xrel;
                        eo["motion"].as_object()["yrel"] = evt.motion.yrel;
                        break;
                    case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    case SDL_EVENT_MOUSE_BUTTON_UP:
                        eo["button"].become_object();
                        eo["button"].as_object()["timestamp"] = evt.button.timestamp;
                        eo["button"].as_object()["windowID"] = evt.button.windowID;
                        eo["button"].as_object()["which"] = evt.button.which;
                        eo["button"].as_object()["button"] = (int)evt.button.button;
                        eo["button"].as_object()["down"] = evt.button.down;
                        eo["button"].as_object()["clicks"] = (int)evt.button.clicks;
                        eo["button"].as_object()["padding"] = (int)evt.button.padding;
                        eo["button"].as_object()["x"] = evt.button.x;
                        eo["button"].as_object()["y"] = evt.button.y;
                        break;
                    case SDL_EVENT_MOUSE_WHEEL:
                        eo["wheel"].become_object();
                        eo["wheel"].as_object()["timestamp"] = evt.wheel.timestamp;
                        eo["wheel"].as_object()["windowID"] = evt.wheel.windowID;
                        eo["wheel"].as_object()["which"] = evt.wheel.which;
                        eo["wheel"].as_object()["x"] = evt.wheel.x;
                        eo["wheel"].as_object()["y"] = evt.wheel.y;
                        eo["wheel"].as_object()["direction"] = evt.wheel.direction;
                        eo["wheel"].as_object()["mouse_x"] = evt.wheel.mouse_x;
                        eo["wheel"].as_object()["mouse_y"] = evt.wheel.mouse_y;
                        eo["wheel"].as_object()["integer_x"] = evt.wheel.integer_x;
                        eo["wheel"].as_object()["integer_y"] = evt.wheel.integer_y;
                        break;
                    case SDL_EVENT_MOUSE_ADDED:
                    case SDL_EVENT_MOUSE_REMOVED:
                        eo["mdevice"].become_object();
                        eo["mdevice"].as_object()["timestamp"] = evt.mdevice.timestamp;
                        eo["mdevice"].as_object()["which"] = evt.mdevice.which;
                        break;
                    case SDL_EVENT_JOYSTICK_AXIS_MOTION:
                        eo["jaxis"].become_object();
                        eo["jaxis"].as_object()["timestamp"] = evt.jaxis.timestamp;
                        eo["jaxis"].as_object()["which"] = evt.jaxis.which;
                        eo["jaxis"].as_object()["axis"] = (int)evt.jaxis.axis;
                        eo["jaxis"].as_object()["padding1"] = (int)evt.jaxis.padding1;
                        eo["jaxis"].as_object()["padding2"] = (int)evt.jaxis.padding2;
                        eo["jaxis"].as_object()["padding3"] = (int)evt.jaxis.padding3;
                        eo["jaxis"].as_object()["value"] = evt.jaxis.value;
                        eo["jaxis"].as_object()["padding4"] = evt.jaxis.padding4;
                        break;
                    case SDL_EVENT_JOYSTICK_BALL_MOTION:
                        eo["jball"].become_object();
                        eo["jball"].as_object()["timestamp"] = evt.jball.timestamp;
                        eo["jball"].as_object()["which"] = evt.jball.which;
                        eo["jball"].as_object()["ball"] = (int)evt.jball.ball;
                        eo["jball"].as_object()["padding1"] = (int)evt.jball.padding1;
                        eo["jball"].as_object()["padding2"] = (int)evt.jball.padding2;
                        eo["jball"].as_object()["padding3"] = (int)evt.jball.padding3;
                        eo["jball"].as_object()["xrel"] = evt.jball.xrel;
                        eo["jball"].as_object()["yrel"] = evt.jball.yrel;
                        break;
                    case SDL_EVENT_JOYSTICK_HAT_MOTION:
                        eo["jhat"].become_object();
                        eo["jhat"].as_object()["timestamp"] = evt.jhat.timestamp;
                        eo["jhat"].as_object()["which"] = evt.jhat.which;
                        eo["jhat"].as_object()["hat"] = (int)evt.jhat.hat;
                        eo["jhat"].as_object()["value"] = (int)evt.jhat.value;
                        eo["jhat"].as_object()["padding1"] = (int)evt.jhat.padding1;
                        eo["jhat"].as_object()["padding2"] = (int)evt.jhat.padding2;
                        break;
                    case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
                    case SDL_EVENT_JOYSTICK_BUTTON_UP:
                        eo["jbutton"].become_object();
                        eo["jbutton"].as_object()["timestamp"] = evt.jbutton.timestamp;
                        eo["jbutton"].as_object()["which"] = evt.jbutton.which;
                        eo["jbutton"].as_object()["button"] = (int)evt.jbutton.button;
                        eo["jbutton"].as_object()["down"] = evt.jbutton.down;
                        eo["jbutton"].as_object()["padding1"] = evt.jbutton.padding1;
                        eo["jbutton"].as_object()["padding2"] = evt.jbutton.padding2;
                        break;
                    case SDL_EVENT_JOYSTICK_BATTERY_UPDATED:
                        eo["jbattery"].become_object();
                        eo["jbattery"].as_object()["timestamp"] = evt.jbattery.timestamp;
                        eo["jbattery"].as_object()["which"] = evt.jbattery.which;
                        eo["jbattery"].as_object()["state"] = evt.jbattery.state;
                        eo["jbattery"].as_object()["percent"] = evt.jbattery.percent;
                        break;
                    case SDL_EVENT_JOYSTICK_ADDED:
                    case SDL_EVENT_JOYSTICK_REMOVED:
                    case SDL_EVENT_JOYSTICK_UPDATE_COMPLETE:
                        eo["jdevice"].become_object();
                        eo["jdevice"].as_object()["timestamp"] = evt.jdevice.timestamp;
                        eo["jdevice"].as_object()["which"] = evt.jdevice.which;
                        break;
                    case SDL_EVENT_GAMEPAD_AXIS_MOTION: break;
                    case SDL_EVENT_GAMEPAD_BUTTON_DOWN: break;
                    case SDL_EVENT_GAMEPAD_BUTTON_UP: break;
                    case SDL_EVENT_GAMEPAD_ADDED: break;
                    case SDL_EVENT_GAMEPAD_REMOVED: break;
                    case SDL_EVENT_GAMEPAD_REMAPPED: break;
                    case SDL_EVENT_GAMEPAD_TOUCHPAD_DOWN: break;
                    case SDL_EVENT_GAMEPAD_TOUCHPAD_MOTION: break;
                    case SDL_EVENT_GAMEPAD_TOUCHPAD_UP: break;
                    case SDL_EVENT_GAMEPAD_SENSOR_UPDATE: break;
                    case SDL_EVENT_GAMEPAD_UPDATE_COMPLETE: break;
                    case SDL_EVENT_GAMEPAD_STEAM_HANDLE_UPDATED: break;
                    case SDL_EVENT_FINGER_DOWN: break;
                    case SDL_EVENT_FINGER_UP: break;
                    case SDL_EVENT_FINGER_MOTION: break;
                    case SDL_EVENT_FINGER_CANCELED: break;
                    case SDL_EVENT_PINCH_BEGIN: break;
                    case SDL_EVENT_PINCH_UPDATE: break;
                    case SDL_EVENT_PINCH_END: break;
                    case SDL_EVENT_CLIPBOARD_UPDATE:
                        eo["clipboard"].become_object();
                        eo["clipboard"].as_object()["timestamp"] = evt.clipboard.timestamp;
                        eo["clipboard"].as_object()["owner"] = evt.clipboard.owner;
                        eo["clipboard"].as_object()["num_mime_types"] = evt.clipboard.num_mime_types;
                        eo["clipboard"].as_object()["mime_types"].become_array();
                        for(int i{0}; i < evt.clipboard.num_mime_types; ++i) {
                            eo["clipboard"].as_object()["mime_types"].as_array().push_back(std::string{evt.clipboard.mime_types[i]});
                        }
                        break;
                    case SDL_EVENT_DROP_FILE: break;
                    case SDL_EVENT_DROP_TEXT: break;
                    case SDL_EVENT_DROP_BEGIN: break;
                    case SDL_EVENT_DROP_COMPLETE: break;
                    case SDL_EVENT_DROP_POSITION: break;
                    case SDL_EVENT_AUDIO_DEVICE_ADDED: break;
                    case SDL_EVENT_AUDIO_DEVICE_REMOVED: break;
                    case SDL_EVENT_AUDIO_DEVICE_FORMAT_CHANGED: break;
                    case SDL_EVENT_SENSOR_UPDATE: break;
                    case SDL_EVENT_PEN_PROXIMITY_IN: break;
                    case SDL_EVENT_PEN_PROXIMITY_OUT: break;
                    case SDL_EVENT_PEN_DOWN: break;
                    case SDL_EVENT_PEN_UP: break;
                    case SDL_EVENT_PEN_BUTTON_DOWN: break;
                    case SDL_EVENT_PEN_BUTTON_UP: break;
                    case SDL_EVENT_PEN_MOTION: break;
                    case SDL_EVENT_PEN_AXIS: break;
                    case SDL_EVENT_CAMERA_DEVICE_ADDED: break;
                    case SDL_EVENT_CAMERA_DEVICE_REMOVED: break;
                    case SDL_EVENT_CAMERA_DEVICE_APPROVED: break;
                    case SDL_EVENT_CAMERA_DEVICE_DENIED: break;
                    case SDL_EVENT_RENDER_TARGETS_RESET: break;
                    case SDL_EVENT_RENDER_DEVICE_RESET: break;
                    case SDL_EVENT_RENDER_DEVICE_LOST: break;
                    case SDL_EVENT_PRIVATE0: break;
                    case SDL_EVENT_PRIVATE1: break;
                    case SDL_EVENT_PRIVATE2: break;
                    case SDL_EVENT_PRIVATE3: break;
                    case SDL_EVENT_POLL_SENTINEL: break;
                    case SDL_EVENT_USER: break;
                    case SDL_EVENT_LAST: break;
                    case SDL_EVENT_ENUM_PADDING: break;
                    default: break;
                }
            }
            return res;
        });

        rt->add_function("gfx_create_window", DFIFUN(args) {
            DFI_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 0, 4)
            return dfi::valbox{
                std::make_shared<window_render>(
                    (args.size() > 0 ? args[0].cast_to_string().c_str() : ""),
                    (args.size() > 1 ? (DFINUMARG(args, 1, int) > 0 ? DFINUMARG(args, 1, int) : 800) : 800),
                    (args.size() > 2 ? (DFINUMARG(args, 2, int) > 0 ? DFINUMARG(args, 2, int) : 600) : 600),
                    (args.size() > 3 ? DFINUMARG(args, 3, uint64_t) : 0)
                ),
                "gfx_window"
            };
        });

        rt->add_method("gfx_window", "id", DFIFUN(args) {
            DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1)
            return DFITHIS(args, std::shared_ptr<window_render>)->id();
        });
        rt->add_method("gfx_window", "show", DFIFUN(args) {
            DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1)
            return DFITHIS(args, std::shared_ptr<window_render>)->show();
        });
        rt->add_method("gfx_window", "width", DFIFUN(args) {
            DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1)
            return DFITHIS(args, std::shared_ptr<window_render>)->width();
        });
        rt->add_method("gfx_window", "height", DFIFUN(args) {
            DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1)
            return DFITHIS(args, std::shared_ptr<window_render>)->height();
        });
        rt->add_method("gfx_window", "display", DFIFUN(args) {
            DFI_CHCK_FUN_PARMS_NUM_EQ(args, 1)
            return DFITHIS(args, std::shared_ptr<window_render>)->display();
        });
        rt->add_method("gfx_window", "clear_background", DFIFUN(args) {
            DFI_CHCK_FUN_PARMS_NUM_EQ(args, 2)
            SDL_Color clr{DFICLASSARG(args, 1, SDL_Color)};
            return DFITHIS(args, std::shared_ptr<window_render>)->clear_background(clr);
        });
        rt->add_method("gfx_window", "line", DFIFUN(args) {
            DFI_CHCK_FUN_PARMS_NUM_EQ(args, 6)
            SDL_Color clr{DFICLASSARG(args, 5, SDL_Color)};
            return DFITHIS(args, std::shared_ptr<window_render>)->line(
                DFINUMARG(args, 1, float), DFINUMARG(args, 2, float), DFINUMARG(args, 3, float), DFINUMARG(args, 4, float), clr
            );
        });

/*
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
            ClearBackground(DFICLASSARG(args, 0, Color));
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
            DrawPixelV(DFICLASSARG(args, 0, Vector2), DFICLASSARG(args, 1, Color)); return dfi::valbox{};
        });

        rt->add_function("ray_draw_line", DFIFUN(args) {
            DFI_CHCK_FUN_PARMS_NUM_EQ(args, 5)
            DrawLine(DFINUMARG(args, 0, int), DFINUMARG(args, 1, int), DFINUMARG(args, 2, int), DFINUMARG(args, 3, int), DFICLASSARG(args, 4, Color)); return dfi::valbox{};
        });
        rt->add_function("ray_draw_line_v", DFIFUN(args) {
            DFI_CHCK_FUN_PARMS_NUM_EQ(args, 3)
            DrawLineV(DFICLASSARG(args, 0, Vector2), DFICLASSARG(args, 1, Vector2), DFICLASSARG(args, 2, Color)); return dfi::valbox{};
        });
        rt->add_function("ray_draw_line_ex", DFIFUN(args) {
            DFI_CHCK_FUN_PARMS_NUM_EQ(args, 4)
            DrawLineEx(DFICLASSARG(args, 0, Vector2), DFICLASSARG(args, 1, Vector2), DFINUMARG(args, 2, float), DFICLASSARG(args, 3, Color)); return dfi::valbox{};
        });
        rt->add_function("ray_draw_line_dashed", DFIFUN(args) {
            DFI_CHCK_FUN_PARMS_NUM_EQ(args, 5)
            DrawLineDashed( DFICLASSARG(args, 0, Vector2), DFICLASSARG(args, 1, Vector2), DFINUMARG(args, 2, int), DFINUMARG(args, 3, int), DFICLASSARG(args, 4, Color)); return dfi::valbox{};
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
            DrawLineBezier(DFICLASSARG(args, 0, Vector2), DFICLASSARG(args, 1, Vector2), DFINUMARG(args, 2, int), DFICLASSARG(args, 3, Color)); return dfi::valbox{};
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
*/
    }

    void add_misc_constants(dfi::runtime_interface *rt) {
        rt->add_var("GFX_LIGHTGRAY",    color_val(200, 200, 200));
        rt->add_var("GFX_GRAY",         color_val(130, 130, 130));
        rt->add_var("GFX_DARKGRAY",     color_val(80, 80, 80));
        rt->add_var("GFX_YELLOW",       color_val(253, 249, 0));
        rt->add_var("GFX_GOLD",         color_val(255, 203, 0));
        rt->add_var("GFX_ORANGE",       color_val(255, 161, 0));
        rt->add_var("GFX_PINK",         color_val(255, 109, 194));
        rt->add_var("GFX_RED",          color_val(230, 41, 55));
        rt->add_var("GFX_MAROON",       color_val(190, 33, 55));
        rt->add_var("GFX_GREEN",        color_val(0, 228, 48));
        rt->add_var("GFX_LIME",         color_val(0, 158, 47));
        rt->add_var("GFX_DARKGREEN",    color_val(0, 117, 44));
        rt->add_var("GFX_SKYBLUE",      color_val(102, 191, 255));
        rt->add_var("GFX_BLUE",         color_val(0, 121, 241));
        rt->add_var("GFX_DARKBLUE",     color_val(0, 82, 172));
        rt->add_var("GFX_PURPLE",       color_val(200, 122, 255));
        rt->add_var("GFX_VIOLET",       color_val(135, 60, 190));
        rt->add_var("GFX_DARKPURPLE",   color_val(112, 31, 126));
        rt->add_var("GFX_BEIGE",        color_val(211, 176, 131));
        rt->add_var("GFX_BROWN",        color_val(127, 106, 79));
        rt->add_var("GFX_DARKBROWN",    color_val(76, 63, 47));
        rt->add_var("GFX_WHITE",        color_val(255, 255, 255));
        rt->add_var("GFX_BLACK",        color_val(0, 0, 0));
        rt->add_var("GFX_BLANK",        color_val(0, 0, 0));
        rt->add_var("GFX_MAGENTA",      color_val(255, 0, 255));
        rt->add_var("GFX_SUBDUEDWHITE", color_val(245, 245, 245));

        rt->add_var("GFX_INIT_AUDIO", SDL_INIT_AUDIO);
        rt->add_var("GFX_INIT_VIDEO", SDL_INIT_VIDEO);
        rt->add_var("GFX_INIT_JOYSTICK", SDL_INIT_JOYSTICK);
        rt->add_var("GFX_INIT_HAPTIC", SDL_INIT_HAPTIC);
        rt->add_var("GFX_INIT_GAMEPAD", SDL_INIT_GAMEPAD);
        rt->add_var("GFX_INIT_EVENTS", SDL_INIT_EVENTS);
        rt->add_var("GFX_INIT_SENSOR", SDL_INIT_SENSOR);
        rt->add_var("GFX_INIT_CAMERA", SDL_INIT_CAMERA);

        rt->add_var("GFX_WINDOW_FULLSCREEN", SDL_WINDOW_FULLSCREEN);
        rt->add_var("GFX_WINDOW_OPENGL", SDL_WINDOW_OPENGL);
        rt->add_var("GFX_WINDOW_HIDDEN", SDL_WINDOW_HIDDEN);
        rt->add_var("GFX_WINDOW_BORDERLESS", SDL_WINDOW_BORDERLESS);
        rt->add_var("GFX_WINDOW_RESIZABLE", SDL_WINDOW_RESIZABLE);
        rt->add_var("GFX_WINDOW_MINIMIZED", SDL_WINDOW_MINIMIZED);
        rt->add_var("GFX_WINDOW_MAXIMIZED", SDL_WINDOW_MAXIMIZED);
        rt->add_var("GFX_WINDOW_MOUSE_GRABBED", SDL_WINDOW_MOUSE_GRABBED);
        rt->add_var("GFX_WINDOW_INPUT_FOCUS", SDL_WINDOW_INPUT_FOCUS);
        rt->add_var("GFX_WINDOW_MOUSE_FOCUS", SDL_WINDOW_MOUSE_FOCUS);
        rt->add_var("GFX_WINDOW_EXTERNAL", SDL_WINDOW_EXTERNAL);
        rt->add_var("GFX_WINDOW_MODAL", SDL_WINDOW_MODAL);
        rt->add_var("GFX_WINDOW_HIGH_PIXEL_DENSITY", SDL_WINDOW_HIGH_PIXEL_DENSITY);
        rt->add_var("GFX_WINDOW_MOUSE_CAPTURE", SDL_WINDOW_MOUSE_CAPTURE);
        rt->add_var("GFX_WINDOW_ALWAYS_ON_TOP", SDL_WINDOW_ALWAYS_ON_TOP);
        rt->add_var("GFX_WINDOW_UTILITY", SDL_WINDOW_UTILITY);
        rt->add_var("GFX_WINDOW_TOOLTIP", SDL_WINDOW_TOOLTIP);
        rt->add_var("GFX_WINDOW_POPUP_MENU", SDL_WINDOW_POPUP_MENU);
        rt->add_var("GFX_WINDOW_KEYBOARD_GRABBED", SDL_WINDOW_KEYBOARD_GRABBED);
        rt->add_var("GFX_WINDOW_VULKAN", SDL_WINDOW_VULKAN);
        rt->add_var("GFX_WINDOW_METAL", SDL_WINDOW_METAL);
        rt->add_var("GFX_WINDOW_TRANSPARENT", SDL_WINDOW_TRANSPARENT);
        rt->add_var("GFX_WINDOW_NOT_FOCUSABLE", SDL_WINDOW_NOT_FOCUSABLE);
    }

    void add_events(dfi::runtime_interface *rt) {
        rt->add_var("GFX_EVENT_FIRST", SDL_EVENT_FIRST);
        rt->add_var("GFX_EVENT_QUIT", SDL_EVENT_QUIT);
        rt->add_var("GFX_EVENT_TERMINATING", SDL_EVENT_TERMINATING);
        rt->add_var("GFX_EVENT_LOW_MEMORY", SDL_EVENT_LOW_MEMORY);
        rt->add_var("GFX_EVENT_WILL_ENTER_BACKGROUND", SDL_EVENT_WILL_ENTER_BACKGROUND);
        rt->add_var("GFX_EVENT_DID_ENTER_BACKGROUND", SDL_EVENT_DID_ENTER_BACKGROUND);
        rt->add_var("GFX_EVENT_WILL_ENTER_FOREGROUND", SDL_EVENT_WILL_ENTER_FOREGROUND);
        rt->add_var("GFX_EVENT_DID_ENTER_FOREGROUND", SDL_EVENT_DID_ENTER_FOREGROUND);
        rt->add_var("GFX_EVENT_LOCALE_CHANGED", SDL_EVENT_LOCALE_CHANGED);
        rt->add_var("GFX_EVENT_SYSTEM_THEME_CHANGED", SDL_EVENT_SYSTEM_THEME_CHANGED);
        rt->add_var("GFX_EVENT_DISPLAY_ORIENTATION", SDL_EVENT_DISPLAY_ORIENTATION);
        rt->add_var("GFX_EVENT_DISPLAY_ADDED", SDL_EVENT_DISPLAY_ADDED);
        rt->add_var("GFX_EVENT_DISPLAY_REMOVED", SDL_EVENT_DISPLAY_REMOVED);
        rt->add_var("GFX_EVENT_DISPLAY_MOVED", SDL_EVENT_DISPLAY_MOVED);
        rt->add_var("GFX_EVENT_DISPLAY_DESKTOP_MODE_CHANGED", SDL_EVENT_DISPLAY_DESKTOP_MODE_CHANGED);
        rt->add_var("GFX_EVENT_DISPLAY_CURRENT_MODE_CHANGED", SDL_EVENT_DISPLAY_CURRENT_MODE_CHANGED);
        rt->add_var("GFX_EVENT_DISPLAY_CONTENT_SCALE_CHANGED", SDL_EVENT_DISPLAY_CONTENT_SCALE_CHANGED);
        rt->add_var("GFX_EVENT_DISPLAY_USABLE_BOUNDS_CHANGED", SDL_EVENT_DISPLAY_USABLE_BOUNDS_CHANGED);
        rt->add_var("GFX_EVENT_DISPLAY_FIRST", SDL_EVENT_DISPLAY_FIRST);
        rt->add_var("GFX_EVENT_DISPLAY_LAST", SDL_EVENT_DISPLAY_LAST);
        rt->add_var("GFX_EVENT_WINDOW_SHOWN", SDL_EVENT_WINDOW_SHOWN);
        rt->add_var("GFX_EVENT_WINDOW_HIDDEN", SDL_EVENT_WINDOW_HIDDEN);
        rt->add_var("GFX_EVENT_WINDOW_EXPOSED", SDL_EVENT_WINDOW_EXPOSED);
        rt->add_var("GFX_EVENT_WINDOW_MOVED", SDL_EVENT_WINDOW_MOVED);
        rt->add_var("GFX_EVENT_WINDOW_RESIZED", SDL_EVENT_WINDOW_RESIZED);
        rt->add_var("GFX_EVENT_WINDOW_PIXEL_SIZE_CHANGED", SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED);
        rt->add_var("GFX_EVENT_WINDOW_METAL_VIEW_RESIZED", SDL_EVENT_WINDOW_METAL_VIEW_RESIZED);
        rt->add_var("GFX_EVENT_WINDOW_MINIMIZED", SDL_EVENT_WINDOW_MINIMIZED);
        rt->add_var("GFX_EVENT_WINDOW_MAXIMIZED", SDL_EVENT_WINDOW_MAXIMIZED);
        rt->add_var("GFX_EVENT_WINDOW_RESTORED", SDL_EVENT_WINDOW_RESTORED);
        rt->add_var("GFX_EVENT_WINDOW_MOUSE_ENTER", SDL_EVENT_WINDOW_MOUSE_ENTER);
        rt->add_var("GFX_EVENT_WINDOW_MOUSE_LEAVE", SDL_EVENT_WINDOW_MOUSE_LEAVE);
        rt->add_var("GFX_EVENT_WINDOW_FOCUS_GAINED", SDL_EVENT_WINDOW_FOCUS_GAINED);
        rt->add_var("GFX_EVENT_WINDOW_FOCUS_LOST", SDL_EVENT_WINDOW_FOCUS_LOST);
        rt->add_var("GFX_EVENT_WINDOW_CLOSE_REQUESTED", SDL_EVENT_WINDOW_CLOSE_REQUESTED);
        rt->add_var("GFX_EVENT_WINDOW_HIT_TEST", SDL_EVENT_WINDOW_HIT_TEST);
        rt->add_var("GFX_EVENT_WINDOW_ICCPROF_CHANGED", SDL_EVENT_WINDOW_ICCPROF_CHANGED);
        rt->add_var("GFX_EVENT_WINDOW_DISPLAY_CHANGED", SDL_EVENT_WINDOW_DISPLAY_CHANGED);
        rt->add_var("GFX_EVENT_WINDOW_DISPLAY_SCALE_CHANGED", SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED);
        rt->add_var("GFX_EVENT_WINDOW_SAFE_AREA_CHANGED", SDL_EVENT_WINDOW_SAFE_AREA_CHANGED);
        rt->add_var("GFX_EVENT_WINDOW_OCCLUDED", SDL_EVENT_WINDOW_OCCLUDED);
        rt->add_var("GFX_EVENT_WINDOW_ENTER_FULLSCREEN", SDL_EVENT_WINDOW_ENTER_FULLSCREEN);
        rt->add_var("GFX_EVENT_WINDOW_LEAVE_FULLSCREEN", SDL_EVENT_WINDOW_LEAVE_FULLSCREEN);
        rt->add_var("GFX_EVENT_WINDOW_DESTROYED", SDL_EVENT_WINDOW_DESTROYED);
        rt->add_var("GFX_EVENT_WINDOW_HDR_STATE_CHANGED", SDL_EVENT_WINDOW_HDR_STATE_CHANGED);
        rt->add_var("GFX_EVENT_WINDOW_FIRST", SDL_EVENT_WINDOW_FIRST);
        rt->add_var("GFX_EVENT_WINDOW_LAST", SDL_EVENT_WINDOW_LAST);
        rt->add_var("GFX_EVENT_KEY_DOWN", SDL_EVENT_KEY_DOWN);
        rt->add_var("GFX_EVENT_KEY_UP", SDL_EVENT_KEY_UP);
        rt->add_var("GFX_EVENT_TEXT_EDITING", SDL_EVENT_TEXT_EDITING);
        rt->add_var("GFX_EVENT_TEXT_INPUT", SDL_EVENT_TEXT_INPUT);
        rt->add_var("GFX_EVENT_KEYMAP_CHANGED", SDL_EVENT_KEYMAP_CHANGED);
        rt->add_var("GFX_EVENT_KEYBOARD_ADDED", SDL_EVENT_KEYBOARD_ADDED);
        rt->add_var("GFX_EVENT_KEYBOARD_REMOVED", SDL_EVENT_KEYBOARD_REMOVED);
        rt->add_var("GFX_EVENT_TEXT_EDITING_CANDIDATES", SDL_EVENT_TEXT_EDITING_CANDIDATES);
        rt->add_var("GFX_EVENT_SCREEN_KEYBOARD_SHOWN", SDL_EVENT_SCREEN_KEYBOARD_SHOWN);
        rt->add_var("GFX_EVENT_SCREEN_KEYBOARD_HIDDEN", SDL_EVENT_SCREEN_KEYBOARD_HIDDEN);
        rt->add_var("GFX_EVENT_MOUSE_MOTION", SDL_EVENT_MOUSE_MOTION);
        rt->add_var("GFX_EVENT_MOUSE_BUTTON_DOWN", SDL_EVENT_MOUSE_BUTTON_DOWN);
        rt->add_var("GFX_EVENT_MOUSE_BUTTON_UP", SDL_EVENT_MOUSE_BUTTON_UP);
        rt->add_var("GFX_EVENT_MOUSE_WHEEL", SDL_EVENT_MOUSE_WHEEL);
        rt->add_var("GFX_EVENT_MOUSE_ADDED", SDL_EVENT_MOUSE_ADDED);
        rt->add_var("GFX_EVENT_MOUSE_REMOVED", SDL_EVENT_MOUSE_REMOVED);
        rt->add_var("GFX_EVENT_JOYSTICK_AXIS_MOTION", SDL_EVENT_JOYSTICK_AXIS_MOTION);
        rt->add_var("GFX_EVENT_JOYSTICK_BALL_MOTION", SDL_EVENT_JOYSTICK_BALL_MOTION);
        rt->add_var("GFX_EVENT_JOYSTICK_HAT_MOTION", SDL_EVENT_JOYSTICK_HAT_MOTION);
        rt->add_var("GFX_EVENT_JOYSTICK_BUTTON_DOWN", SDL_EVENT_JOYSTICK_BUTTON_DOWN);
        rt->add_var("GFX_EVENT_JOYSTICK_BUTTON_UP", SDL_EVENT_JOYSTICK_BUTTON_UP);
        rt->add_var("GFX_EVENT_JOYSTICK_ADDED", SDL_EVENT_JOYSTICK_ADDED);
        rt->add_var("GFX_EVENT_JOYSTICK_REMOVED", SDL_EVENT_JOYSTICK_REMOVED);
        rt->add_var("GFX_EVENT_JOYSTICK_BATTERY_UPDATED", SDL_EVENT_JOYSTICK_BATTERY_UPDATED);
        rt->add_var("GFX_EVENT_JOYSTICK_UPDATE_COMPLETE", SDL_EVENT_JOYSTICK_UPDATE_COMPLETE);
        rt->add_var("GFX_EVENT_GAMEPAD_AXIS_MOTION", SDL_EVENT_GAMEPAD_AXIS_MOTION);
        rt->add_var("GFX_EVENT_GAMEPAD_BUTTON_DOWN", SDL_EVENT_GAMEPAD_BUTTON_DOWN);
        rt->add_var("GFX_EVENT_GAMEPAD_BUTTON_UP", SDL_EVENT_GAMEPAD_BUTTON_UP);
        rt->add_var("GFX_EVENT_GAMEPAD_ADDED", SDL_EVENT_GAMEPAD_ADDED);
        rt->add_var("GFX_EVENT_GAMEPAD_REMOVED", SDL_EVENT_GAMEPAD_REMOVED);
        rt->add_var("GFX_EVENT_GAMEPAD_REMAPPED", SDL_EVENT_GAMEPAD_REMAPPED);
        rt->add_var("GFX_EVENT_GAMEPAD_TOUCHPAD_DOWN", SDL_EVENT_GAMEPAD_TOUCHPAD_DOWN);
        rt->add_var("GFX_EVENT_GAMEPAD_TOUCHPAD_MOTION", SDL_EVENT_GAMEPAD_TOUCHPAD_MOTION);
        rt->add_var("GFX_EVENT_GAMEPAD_TOUCHPAD_UP", SDL_EVENT_GAMEPAD_TOUCHPAD_UP);
        rt->add_var("GFX_EVENT_GAMEPAD_SENSOR_UPDATE", SDL_EVENT_GAMEPAD_SENSOR_UPDATE);
        rt->add_var("GFX_EVENT_GAMEPAD_UPDATE_COMPLETE", SDL_EVENT_GAMEPAD_UPDATE_COMPLETE);
        rt->add_var("GFX_EVENT_GAMEPAD_STEAM_HANDLE_UPDATED", SDL_EVENT_GAMEPAD_STEAM_HANDLE_UPDATED);
        rt->add_var("GFX_EVENT_FINGER_DOWN", SDL_EVENT_FINGER_DOWN);
        rt->add_var("GFX_EVENT_FINGER_UP", SDL_EVENT_FINGER_UP);
        rt->add_var("GFX_EVENT_FINGER_MOTION", SDL_EVENT_FINGER_MOTION);
        rt->add_var("GFX_EVENT_FINGER_CANCELED", SDL_EVENT_FINGER_CANCELED);
        rt->add_var("GFX_EVENT_PINCH_BEGIN", SDL_EVENT_PINCH_BEGIN);
        rt->add_var("GFX_EVENT_PINCH_UPDATE", SDL_EVENT_PINCH_UPDATE);
        rt->add_var("GFX_EVENT_PINCH_END", SDL_EVENT_PINCH_END);
        rt->add_var("GFX_EVENT_CLIPBOARD_UPDATE", SDL_EVENT_CLIPBOARD_UPDATE);
        rt->add_var("GFX_EVENT_DROP_FILE", SDL_EVENT_DROP_FILE);
        rt->add_var("GFX_EVENT_DROP_TEXT", SDL_EVENT_DROP_TEXT);
        rt->add_var("GFX_EVENT_DROP_BEGIN", SDL_EVENT_DROP_BEGIN);
        rt->add_var("GFX_EVENT_DROP_COMPLETE", SDL_EVENT_DROP_COMPLETE);
        rt->add_var("GFX_EVENT_DROP_POSITION", SDL_EVENT_DROP_POSITION);
        rt->add_var("GFX_EVENT_AUDIO_DEVICE_ADDED", SDL_EVENT_AUDIO_DEVICE_ADDED);
        rt->add_var("GFX_EVENT_AUDIO_DEVICE_REMOVED", SDL_EVENT_AUDIO_DEVICE_REMOVED);
        rt->add_var("GFX_EVENT_AUDIO_DEVICE_FORMAT_CHANGED", SDL_EVENT_AUDIO_DEVICE_FORMAT_CHANGED);
        rt->add_var("GFX_EVENT_SENSOR_UPDATE", SDL_EVENT_SENSOR_UPDATE);
        rt->add_var("GFX_EVENT_PEN_PROXIMITY_IN", SDL_EVENT_PEN_PROXIMITY_IN);
        rt->add_var("GFX_EVENT_PEN_PROXIMITY_OUT", SDL_EVENT_PEN_PROXIMITY_OUT);
        rt->add_var("GFX_EVENT_PEN_DOWN", SDL_EVENT_PEN_DOWN);
        rt->add_var("GFX_EVENT_PEN_UP", SDL_EVENT_PEN_UP);
        rt->add_var("GFX_EVENT_PEN_BUTTON_DOWN", SDL_EVENT_PEN_BUTTON_DOWN);
        rt->add_var("GFX_EVENT_PEN_BUTTON_UP", SDL_EVENT_PEN_BUTTON_UP);
        rt->add_var("GFX_EVENT_PEN_MOTION", SDL_EVENT_PEN_MOTION);
        rt->add_var("GFX_EVENT_PEN_AXIS", SDL_EVENT_PEN_AXIS);
        rt->add_var("GFX_EVENT_CAMERA_DEVICE_ADDED", SDL_EVENT_CAMERA_DEVICE_ADDED);
        rt->add_var("GFX_EVENT_CAMERA_DEVICE_REMOVED", SDL_EVENT_CAMERA_DEVICE_REMOVED);
        rt->add_var("GFX_EVENT_CAMERA_DEVICE_APPROVED", SDL_EVENT_CAMERA_DEVICE_APPROVED);
        rt->add_var("GFX_EVENT_CAMERA_DEVICE_DENIED", SDL_EVENT_CAMERA_DEVICE_DENIED);
        rt->add_var("GFX_EVENT_RENDER_TARGETS_RESET", SDL_EVENT_RENDER_TARGETS_RESET);
        rt->add_var("GFX_EVENT_RENDER_DEVICE_RESET", SDL_EVENT_RENDER_DEVICE_RESET);
        rt->add_var("GFX_EVENT_RENDER_DEVICE_LOST", SDL_EVENT_RENDER_DEVICE_LOST);
        rt->add_var("GFX_EVENT_PRIVATE0", SDL_EVENT_PRIVATE0);
        rt->add_var("GFX_EVENT_PRIVATE1", SDL_EVENT_PRIVATE1);
        rt->add_var("GFX_EVENT_PRIVATE2", SDL_EVENT_PRIVATE2);
        rt->add_var("GFX_EVENT_PRIVATE3", SDL_EVENT_PRIVATE3);
        rt->add_var("GFX_EVENT_POLL_SENTINEL", SDL_EVENT_POLL_SENTINEL);
        rt->add_var("GFX_EVENT_USER", SDL_EVENT_USER);
        rt->add_var("GFX_EVENT_LAST", SDL_EVENT_LAST);
        rt->add_var("GFX_EVENT_ENUM_PADDING", SDL_EVENT_ENUM_PADDING);
    }

    void add_keys_scancodes(dfi::runtime_interface *rt) {
        rt->add_var("GFX_SCANCODE_UNKNOWN", SDL_SCANCODE_UNKNOWN);
        rt->add_var("GFX_SCANCODE_A", SDL_SCANCODE_A);
        rt->add_var("GFX_SCANCODE_B", SDL_SCANCODE_B);
        rt->add_var("GFX_SCANCODE_C", SDL_SCANCODE_C);
        rt->add_var("GFX_SCANCODE_D", SDL_SCANCODE_D);
        rt->add_var("GFX_SCANCODE_E", SDL_SCANCODE_E);
        rt->add_var("GFX_SCANCODE_F", SDL_SCANCODE_F);
        rt->add_var("GFX_SCANCODE_G", SDL_SCANCODE_G);
        rt->add_var("GFX_SCANCODE_H", SDL_SCANCODE_H);
        rt->add_var("GFX_SCANCODE_I", SDL_SCANCODE_I);
        rt->add_var("GFX_SCANCODE_J", SDL_SCANCODE_J);
        rt->add_var("GFX_SCANCODE_K", SDL_SCANCODE_K);
        rt->add_var("GFX_SCANCODE_L", SDL_SCANCODE_L);
        rt->add_var("GFX_SCANCODE_M", SDL_SCANCODE_M);
        rt->add_var("GFX_SCANCODE_N", SDL_SCANCODE_N);
        rt->add_var("GFX_SCANCODE_O", SDL_SCANCODE_O);
        rt->add_var("GFX_SCANCODE_P", SDL_SCANCODE_P);
        rt->add_var("GFX_SCANCODE_Q", SDL_SCANCODE_Q);
        rt->add_var("GFX_SCANCODE_R", SDL_SCANCODE_R);
        rt->add_var("GFX_SCANCODE_S", SDL_SCANCODE_S);
        rt->add_var("GFX_SCANCODE_T", SDL_SCANCODE_T);
        rt->add_var("GFX_SCANCODE_U", SDL_SCANCODE_U);
        rt->add_var("GFX_SCANCODE_V", SDL_SCANCODE_V);
        rt->add_var("GFX_SCANCODE_W", SDL_SCANCODE_W);
        rt->add_var("GFX_SCANCODE_X", SDL_SCANCODE_X);
        rt->add_var("GFX_SCANCODE_Y", SDL_SCANCODE_Y);
        rt->add_var("GFX_SCANCODE_Z", SDL_SCANCODE_Z);
        rt->add_var("GFX_SCANCODE_1", SDL_SCANCODE_1);
        rt->add_var("GFX_SCANCODE_2", SDL_SCANCODE_2);
        rt->add_var("GFX_SCANCODE_3", SDL_SCANCODE_3);
        rt->add_var("GFX_SCANCODE_4", SDL_SCANCODE_4);
        rt->add_var("GFX_SCANCODE_5", SDL_SCANCODE_5);
        rt->add_var("GFX_SCANCODE_6", SDL_SCANCODE_6);
        rt->add_var("GFX_SCANCODE_7", SDL_SCANCODE_7);
        rt->add_var("GFX_SCANCODE_8", SDL_SCANCODE_8);
        rt->add_var("GFX_SCANCODE_9", SDL_SCANCODE_9);
        rt->add_var("GFX_SCANCODE_0", SDL_SCANCODE_0);
        rt->add_var("GFX_SCANCODE_RETURN", SDL_SCANCODE_RETURN);
        rt->add_var("GFX_SCANCODE_ESCAPE", SDL_SCANCODE_ESCAPE);
        rt->add_var("GFX_SCANCODE_BACKSPACE", SDL_SCANCODE_BACKSPACE);
        rt->add_var("GFX_SCANCODE_TAB", SDL_SCANCODE_TAB);
        rt->add_var("GFX_SCANCODE_SPACE", SDL_SCANCODE_SPACE);
        rt->add_var("GFX_SCANCODE_MINUS", SDL_SCANCODE_MINUS);
        rt->add_var("GFX_SCANCODE_EQUALS", SDL_SCANCODE_EQUALS);
        rt->add_var("GFX_SCANCODE_LEFTBRACKET", SDL_SCANCODE_LEFTBRACKET);
        rt->add_var("GFX_SCANCODE_RIGHTBRACKET", SDL_SCANCODE_RIGHTBRACKET);
        rt->add_var("GFX_SCANCODE_BACKSLASH", SDL_SCANCODE_BACKSLASH);
        rt->add_var("GFX_SCANCODE_NONUSHASH", SDL_SCANCODE_NONUSHASH);
        rt->add_var("GFX_SCANCODE_SEMICOLON", SDL_SCANCODE_SEMICOLON);
        rt->add_var("GFX_SCANCODE_APOSTROPHE", SDL_SCANCODE_APOSTROPHE);
        rt->add_var("GFX_SCANCODE_GRAVE", SDL_SCANCODE_GRAVE);
        rt->add_var("GFX_SCANCODE_COMMA", SDL_SCANCODE_COMMA);
        rt->add_var("GFX_SCANCODE_PERIOD", SDL_SCANCODE_PERIOD);
        rt->add_var("GFX_SCANCODE_SLASH", SDL_SCANCODE_SLASH);
        rt->add_var("GFX_SCANCODE_CAPSLOCK", SDL_SCANCODE_CAPSLOCK);
        rt->add_var("GFX_SCANCODE_F1", SDL_SCANCODE_F1);
        rt->add_var("GFX_SCANCODE_F2", SDL_SCANCODE_F2);
        rt->add_var("GFX_SCANCODE_F3", SDL_SCANCODE_F3);
        rt->add_var("GFX_SCANCODE_F4", SDL_SCANCODE_F4);
        rt->add_var("GFX_SCANCODE_F5", SDL_SCANCODE_F5);
        rt->add_var("GFX_SCANCODE_F6", SDL_SCANCODE_F6);
        rt->add_var("GFX_SCANCODE_F7", SDL_SCANCODE_F7);
        rt->add_var("GFX_SCANCODE_F8", SDL_SCANCODE_F8);
        rt->add_var("GFX_SCANCODE_F9", SDL_SCANCODE_F9);
        rt->add_var("GFX_SCANCODE_F10", SDL_SCANCODE_F10);
        rt->add_var("GFX_SCANCODE_F11", SDL_SCANCODE_F11);
        rt->add_var("GFX_SCANCODE_F12", SDL_SCANCODE_F12);
        rt->add_var("GFX_SCANCODE_PRINTSCREEN", SDL_SCANCODE_PRINTSCREEN);
        rt->add_var("GFX_SCANCODE_SCROLLLOCK", SDL_SCANCODE_SCROLLLOCK);
        rt->add_var("GFX_SCANCODE_PAUSE", SDL_SCANCODE_PAUSE);
        rt->add_var("GFX_SCANCODE_INSERT", SDL_SCANCODE_INSERT);
        rt->add_var("GFX_SCANCODE_HOME", SDL_SCANCODE_HOME);
        rt->add_var("GFX_SCANCODE_PAGEUP", SDL_SCANCODE_PAGEUP);
        rt->add_var("GFX_SCANCODE_DELETE", SDL_SCANCODE_DELETE);
        rt->add_var("GFX_SCANCODE_END", SDL_SCANCODE_END);
        rt->add_var("GFX_SCANCODE_PAGEDOWN", SDL_SCANCODE_PAGEDOWN);
        rt->add_var("GFX_SCANCODE_RIGHT", SDL_SCANCODE_RIGHT);
        rt->add_var("GFX_SCANCODE_LEFT", SDL_SCANCODE_LEFT);
        rt->add_var("GFX_SCANCODE_DOWN", SDL_SCANCODE_DOWN);
        rt->add_var("GFX_SCANCODE_UP", SDL_SCANCODE_UP);
        rt->add_var("GFX_SCANCODE_NUMLOCKCLEAR", SDL_SCANCODE_NUMLOCKCLEAR);
        rt->add_var("GFX_SCANCODE_KP_DIVIDE", SDL_SCANCODE_KP_DIVIDE);
        rt->add_var("GFX_SCANCODE_KP_MULTIPLY", SDL_SCANCODE_KP_MULTIPLY);
        rt->add_var("GFX_SCANCODE_KP_MINUS", SDL_SCANCODE_KP_MINUS);
        rt->add_var("GFX_SCANCODE_KP_PLUS", SDL_SCANCODE_KP_PLUS);
        rt->add_var("GFX_SCANCODE_KP_ENTER", SDL_SCANCODE_KP_ENTER);
        rt->add_var("GFX_SCANCODE_KP_1", SDL_SCANCODE_KP_1);
        rt->add_var("GFX_SCANCODE_KP_2", SDL_SCANCODE_KP_2);
        rt->add_var("GFX_SCANCODE_KP_3", SDL_SCANCODE_KP_3);
        rt->add_var("GFX_SCANCODE_KP_4", SDL_SCANCODE_KP_4);
        rt->add_var("GFX_SCANCODE_KP_5", SDL_SCANCODE_KP_5);
        rt->add_var("GFX_SCANCODE_KP_6", SDL_SCANCODE_KP_6);
        rt->add_var("GFX_SCANCODE_KP_7", SDL_SCANCODE_KP_7);
        rt->add_var("GFX_SCANCODE_KP_8", SDL_SCANCODE_KP_8);
        rt->add_var("GFX_SCANCODE_KP_9", SDL_SCANCODE_KP_9);
        rt->add_var("GFX_SCANCODE_KP_0", SDL_SCANCODE_KP_0);
        rt->add_var("GFX_SCANCODE_KP_PERIOD", SDL_SCANCODE_KP_PERIOD);
        rt->add_var("GFX_SCANCODE_NONUSBACKSLASH", SDL_SCANCODE_NONUSBACKSLASH);
        rt->add_var("GFX_SCANCODE_APPLICATION", SDL_SCANCODE_APPLICATION);
        rt->add_var("GFX_SCANCODE_POWER", SDL_SCANCODE_POWER);
        rt->add_var("GFX_SCANCODE_KP_EQUALS", SDL_SCANCODE_KP_EQUALS);
        rt->add_var("GFX_SCANCODE_F13", SDL_SCANCODE_F13);
        rt->add_var("GFX_SCANCODE_F14", SDL_SCANCODE_F14);
        rt->add_var("GFX_SCANCODE_F15", SDL_SCANCODE_F15);
        rt->add_var("GFX_SCANCODE_F16", SDL_SCANCODE_F16);
        rt->add_var("GFX_SCANCODE_F17", SDL_SCANCODE_F17);
        rt->add_var("GFX_SCANCODE_F18", SDL_SCANCODE_F18);
        rt->add_var("GFX_SCANCODE_F19", SDL_SCANCODE_F19);
        rt->add_var("GFX_SCANCODE_F20", SDL_SCANCODE_F20);
        rt->add_var("GFX_SCANCODE_F21", SDL_SCANCODE_F21);
        rt->add_var("GFX_SCANCODE_F22", SDL_SCANCODE_F22);
        rt->add_var("GFX_SCANCODE_F23", SDL_SCANCODE_F23);
        rt->add_var("GFX_SCANCODE_F24", SDL_SCANCODE_F24);
        rt->add_var("GFX_SCANCODE_EXECUTE", SDL_SCANCODE_EXECUTE);
        rt->add_var("GFX_SCANCODE_HELP", SDL_SCANCODE_HELP);
        rt->add_var("GFX_SCANCODE_MENU", SDL_SCANCODE_MENU);
        rt->add_var("GFX_SCANCODE_SELECT", SDL_SCANCODE_SELECT);
        rt->add_var("GFX_SCANCODE_STOP", SDL_SCANCODE_STOP);
        rt->add_var("GFX_SCANCODE_AGAIN", SDL_SCANCODE_AGAIN);
        rt->add_var("GFX_SCANCODE_UNDO", SDL_SCANCODE_UNDO);
        rt->add_var("GFX_SCANCODE_CUT", SDL_SCANCODE_CUT);
        rt->add_var("GFX_SCANCODE_COPY", SDL_SCANCODE_COPY);
        rt->add_var("GFX_SCANCODE_PASTE", SDL_SCANCODE_PASTE);
        rt->add_var("GFX_SCANCODE_FIND", SDL_SCANCODE_FIND);
        rt->add_var("GFX_SCANCODE_MUTE", SDL_SCANCODE_MUTE);
        rt->add_var("GFX_SCANCODE_VOLUMEUP", SDL_SCANCODE_VOLUMEUP);
        rt->add_var("GFX_SCANCODE_VOLUMEDOWN", SDL_SCANCODE_VOLUMEDOWN);
        rt->add_var("GFX_SCANCODE_KP_COMMA", SDL_SCANCODE_KP_COMMA);
        rt->add_var("GFX_SCANCODE_KP_EQUALSAS400", SDL_SCANCODE_KP_EQUALSAS400);
        rt->add_var("GFX_SCANCODE_INTERNATIONAL1", SDL_SCANCODE_INTERNATIONAL1);
        rt->add_var("GFX_SCANCODE_INTERNATIONAL2", SDL_SCANCODE_INTERNATIONAL2);
        rt->add_var("GFX_SCANCODE_INTERNATIONAL3", SDL_SCANCODE_INTERNATIONAL3);
        rt->add_var("GFX_SCANCODE_INTERNATIONAL4", SDL_SCANCODE_INTERNATIONAL4);
        rt->add_var("GFX_SCANCODE_INTERNATIONAL5", SDL_SCANCODE_INTERNATIONAL5);
        rt->add_var("GFX_SCANCODE_INTERNATIONAL6", SDL_SCANCODE_INTERNATIONAL6);
        rt->add_var("GFX_SCANCODE_INTERNATIONAL7", SDL_SCANCODE_INTERNATIONAL7);
        rt->add_var("GFX_SCANCODE_INTERNATIONAL8", SDL_SCANCODE_INTERNATIONAL8);
        rt->add_var("GFX_SCANCODE_INTERNATIONAL9", SDL_SCANCODE_INTERNATIONAL9);
        rt->add_var("GFX_SCANCODE_LANG1", SDL_SCANCODE_LANG1);
        rt->add_var("GFX_SCANCODE_LANG2", SDL_SCANCODE_LANG2);
        rt->add_var("GFX_SCANCODE_LANG3", SDL_SCANCODE_LANG3);
        rt->add_var("GFX_SCANCODE_LANG4", SDL_SCANCODE_LANG4);
        rt->add_var("GFX_SCANCODE_LANG5", SDL_SCANCODE_LANG5);
        rt->add_var("GFX_SCANCODE_LANG6", SDL_SCANCODE_LANG6);
        rt->add_var("GFX_SCANCODE_LANG7", SDL_SCANCODE_LANG7);
        rt->add_var("GFX_SCANCODE_LANG8", SDL_SCANCODE_LANG8);
        rt->add_var("GFX_SCANCODE_LANG9", SDL_SCANCODE_LANG9);
        rt->add_var("GFX_SCANCODE_ALTERASE", SDL_SCANCODE_ALTERASE);
        rt->add_var("GFX_SCANCODE_SYSREQ", SDL_SCANCODE_SYSREQ);
        rt->add_var("GFX_SCANCODE_CANCEL", SDL_SCANCODE_CANCEL);
        rt->add_var("GFX_SCANCODE_CLEAR", SDL_SCANCODE_CLEAR);
        rt->add_var("GFX_SCANCODE_PRIOR", SDL_SCANCODE_PRIOR);
        rt->add_var("GFX_SCANCODE_RETURN2", SDL_SCANCODE_RETURN2);
        rt->add_var("GFX_SCANCODE_SEPARATOR", SDL_SCANCODE_SEPARATOR);
        rt->add_var("GFX_SCANCODE_OUT", SDL_SCANCODE_OUT);
        rt->add_var("GFX_SCANCODE_OPER", SDL_SCANCODE_OPER);
        rt->add_var("GFX_SCANCODE_CLEARAGAIN", SDL_SCANCODE_CLEARAGAIN);
        rt->add_var("GFX_SCANCODE_CRSEL", SDL_SCANCODE_CRSEL);
        rt->add_var("GFX_SCANCODE_EXSEL", SDL_SCANCODE_EXSEL);
        rt->add_var("GFX_SCANCODE_FRONT", SDL_SCANCODE_FRONT);
        rt->add_var("GFX_SCANCODE_KP_00", SDL_SCANCODE_KP_00);
        rt->add_var("GFX_SCANCODE_KP_000", SDL_SCANCODE_KP_000);
        rt->add_var("GFX_SCANCODE_THOUSANDSSEPARATOR", SDL_SCANCODE_THOUSANDSSEPARATOR);
        rt->add_var("GFX_SCANCODE_DECIMALSEPARATOR", SDL_SCANCODE_DECIMALSEPARATOR);
        rt->add_var("GFX_SCANCODE_CURRENCYUNIT", SDL_SCANCODE_CURRENCYUNIT);
        rt->add_var("GFX_SCANCODE_CURRENCYSUBUNIT", SDL_SCANCODE_CURRENCYSUBUNIT);
        rt->add_var("GFX_SCANCODE_KP_LEFTPAREN", SDL_SCANCODE_KP_LEFTPAREN);
        rt->add_var("GFX_SCANCODE_KP_RIGHTPAREN", SDL_SCANCODE_KP_RIGHTPAREN);
        rt->add_var("GFX_SCANCODE_KP_LEFTBRACE", SDL_SCANCODE_KP_LEFTBRACE);
        rt->add_var("GFX_SCANCODE_KP_RIGHTBRACE", SDL_SCANCODE_KP_RIGHTBRACE);
        rt->add_var("GFX_SCANCODE_KP_TAB", SDL_SCANCODE_KP_TAB);
        rt->add_var("GFX_SCANCODE_KP_BACKSPACE", SDL_SCANCODE_KP_BACKSPACE);
        rt->add_var("GFX_SCANCODE_KP_A", SDL_SCANCODE_KP_A);
        rt->add_var("GFX_SCANCODE_KP_B", SDL_SCANCODE_KP_B);
        rt->add_var("GFX_SCANCODE_KP_C", SDL_SCANCODE_KP_C);
        rt->add_var("GFX_SCANCODE_KP_D", SDL_SCANCODE_KP_D);
        rt->add_var("GFX_SCANCODE_KP_E", SDL_SCANCODE_KP_E);
        rt->add_var("GFX_SCANCODE_KP_F", SDL_SCANCODE_KP_F);
        rt->add_var("GFX_SCANCODE_KP_XOR", SDL_SCANCODE_KP_XOR);
        rt->add_var("GFX_SCANCODE_KP_POWER", SDL_SCANCODE_KP_POWER);
        rt->add_var("GFX_SCANCODE_KP_PERCENT", SDL_SCANCODE_KP_PERCENT);
        rt->add_var("GFX_SCANCODE_KP_LESS", SDL_SCANCODE_KP_LESS);
        rt->add_var("GFX_SCANCODE_KP_GREATER", SDL_SCANCODE_KP_GREATER);
        rt->add_var("GFX_SCANCODE_KP_AMPERSAND", SDL_SCANCODE_KP_AMPERSAND);
        rt->add_var("GFX_SCANCODE_KP_DBLAMPERSAND", SDL_SCANCODE_KP_DBLAMPERSAND);
        rt->add_var("GFX_SCANCODE_KP_VERTICALBAR", SDL_SCANCODE_KP_VERTICALBAR);
        rt->add_var("GFX_SCANCODE_KP_DBLVERTICALBAR", SDL_SCANCODE_KP_DBLVERTICALBAR);
        rt->add_var("GFX_SCANCODE_KP_COLON", SDL_SCANCODE_KP_COLON);
        rt->add_var("GFX_SCANCODE_KP_HASH", SDL_SCANCODE_KP_HASH);
        rt->add_var("GFX_SCANCODE_KP_SPACE", SDL_SCANCODE_KP_SPACE);
        rt->add_var("GFX_SCANCODE_KP_AT", SDL_SCANCODE_KP_AT);
        rt->add_var("GFX_SCANCODE_KP_EXCLAM", SDL_SCANCODE_KP_EXCLAM);
        rt->add_var("GFX_SCANCODE_KP_MEMSTORE", SDL_SCANCODE_KP_MEMSTORE);
        rt->add_var("GFX_SCANCODE_KP_MEMRECALL", SDL_SCANCODE_KP_MEMRECALL);
        rt->add_var("GFX_SCANCODE_KP_MEMCLEAR", SDL_SCANCODE_KP_MEMCLEAR);
        rt->add_var("GFX_SCANCODE_KP_MEMADD", SDL_SCANCODE_KP_MEMADD);
        rt->add_var("GFX_SCANCODE_KP_MEMSUBTRACT", SDL_SCANCODE_KP_MEMSUBTRACT);
        rt->add_var("GFX_SCANCODE_KP_MEMMULTIPLY", SDL_SCANCODE_KP_MEMMULTIPLY);
        rt->add_var("GFX_SCANCODE_KP_MEMDIVIDE", SDL_SCANCODE_KP_MEMDIVIDE);
        rt->add_var("GFX_SCANCODE_KP_PLUSMINUS", SDL_SCANCODE_KP_PLUSMINUS);
        rt->add_var("GFX_SCANCODE_KP_CLEAR", SDL_SCANCODE_KP_CLEAR);
        rt->add_var("GFX_SCANCODE_KP_CLEARENTRY", SDL_SCANCODE_KP_CLEARENTRY);
        rt->add_var("GFX_SCANCODE_KP_BINARY", SDL_SCANCODE_KP_BINARY);
        rt->add_var("GFX_SCANCODE_KP_OCTAL", SDL_SCANCODE_KP_OCTAL);
        rt->add_var("GFX_SCANCODE_KP_DECIMAL", SDL_SCANCODE_KP_DECIMAL);
        rt->add_var("GFX_SCANCODE_KP_HEXADECIMAL", SDL_SCANCODE_KP_HEXADECIMAL);
        rt->add_var("GFX_SCANCODE_LCTRL", SDL_SCANCODE_LCTRL);
        rt->add_var("GFX_SCANCODE_LSHIFT", SDL_SCANCODE_LSHIFT);
        rt->add_var("GFX_SCANCODE_LALT", SDL_SCANCODE_LALT);
        rt->add_var("GFX_SCANCODE_LGUI", SDL_SCANCODE_LGUI);
        rt->add_var("GFX_SCANCODE_RCTRL", SDL_SCANCODE_RCTRL);
        rt->add_var("GFX_SCANCODE_RSHIFT", SDL_SCANCODE_RSHIFT);
        rt->add_var("GFX_SCANCODE_RALT", SDL_SCANCODE_RALT);
        rt->add_var("GFX_SCANCODE_RGUI", SDL_SCANCODE_RGUI);
        rt->add_var("GFX_SCANCODE_MODE", SDL_SCANCODE_MODE);
        rt->add_var("GFX_SCANCODE_SLEEP", SDL_SCANCODE_SLEEP);
        rt->add_var("GFX_SCANCODE_WAKE", SDL_SCANCODE_WAKE);
        rt->add_var("GFX_SCANCODE_CHANNEL_INCREMENT", SDL_SCANCODE_CHANNEL_INCREMENT);
        rt->add_var("GFX_SCANCODE_CHANNEL_DECREMENT", SDL_SCANCODE_CHANNEL_DECREMENT);
        rt->add_var("GFX_SCANCODE_MEDIA_PLAY", SDL_SCANCODE_MEDIA_PLAY);
        rt->add_var("GFX_SCANCODE_MEDIA_PAUSE", SDL_SCANCODE_MEDIA_PAUSE);
        rt->add_var("GFX_SCANCODE_MEDIA_RECORD", SDL_SCANCODE_MEDIA_RECORD);
        rt->add_var("GFX_SCANCODE_MEDIA_FAST_FORWARD", SDL_SCANCODE_MEDIA_FAST_FORWARD);
        rt->add_var("GFX_SCANCODE_MEDIA_REWIND", SDL_SCANCODE_MEDIA_REWIND);
        rt->add_var("GFX_SCANCODE_MEDIA_NEXT_TRACK", SDL_SCANCODE_MEDIA_NEXT_TRACK);
        rt->add_var("GFX_SCANCODE_MEDIA_PREVIOUS_TRACK", SDL_SCANCODE_MEDIA_PREVIOUS_TRACK);
        rt->add_var("GFX_SCANCODE_MEDIA_STOP", SDL_SCANCODE_MEDIA_STOP);
        rt->add_var("GFX_SCANCODE_MEDIA_EJECT", SDL_SCANCODE_MEDIA_EJECT);
        rt->add_var("GFX_SCANCODE_MEDIA_PLAY_PAUSE", SDL_SCANCODE_MEDIA_PLAY_PAUSE);
        rt->add_var("GFX_SCANCODE_MEDIA_SELECT", SDL_SCANCODE_MEDIA_SELECT);
        rt->add_var("GFX_SCANCODE_AC_NEW", SDL_SCANCODE_AC_NEW);
        rt->add_var("GFX_SCANCODE_AC_OPEN", SDL_SCANCODE_AC_OPEN);
        rt->add_var("GFX_SCANCODE_AC_CLOSE", SDL_SCANCODE_AC_CLOSE);
        rt->add_var("GFX_SCANCODE_AC_EXIT", SDL_SCANCODE_AC_EXIT);
        rt->add_var("GFX_SCANCODE_AC_SAVE", SDL_SCANCODE_AC_SAVE);
        rt->add_var("GFX_SCANCODE_AC_PRINT", SDL_SCANCODE_AC_PRINT);
        rt->add_var("GFX_SCANCODE_AC_PROPERTIES", SDL_SCANCODE_AC_PROPERTIES);
        rt->add_var("GFX_SCANCODE_AC_SEARCH", SDL_SCANCODE_AC_SEARCH);
        rt->add_var("GFX_SCANCODE_AC_HOME", SDL_SCANCODE_AC_HOME);
        rt->add_var("GFX_SCANCODE_AC_BACK", SDL_SCANCODE_AC_BACK);
        rt->add_var("GFX_SCANCODE_AC_FORWARD", SDL_SCANCODE_AC_FORWARD);
        rt->add_var("GFX_SCANCODE_AC_STOP", SDL_SCANCODE_AC_STOP);
        rt->add_var("GFX_SCANCODE_AC_REFRESH", SDL_SCANCODE_AC_REFRESH);
        rt->add_var("GFX_SCANCODE_AC_BOOKMARKS", SDL_SCANCODE_AC_BOOKMARKS);
        rt->add_var("GFX_SCANCODE_SOFTLEFT", SDL_SCANCODE_SOFTLEFT);
        rt->add_var("GFX_SCANCODE_SOFTRIGHT", SDL_SCANCODE_SOFTRIGHT);
        rt->add_var("GFX_SCANCODE_CALL", SDL_SCANCODE_CALL);
        rt->add_var("GFX_SCANCODE_ENDCALL", SDL_SCANCODE_ENDCALL);
        rt->add_var("GFX_SCANCODE_RESERVED", SDL_SCANCODE_RESERVED);
        rt->add_var("GFX_SCANCODE_COUNT", SDL_SCANCODE_COUNT);
    }

    void unregister_runtime() override {
        std::unique_lock l{rt_mtp_};
        if(rt_ == nullptr) {
            return;
        }

        ////////////////////////////

        ////////////////////////////

        rt_ = nullptr;
    }

private:
    dfi::shared_mutex rt_mtp_{};
    dfi::runtime_interface *rt_{nullptr};
};

dfi::extension_interface *create_dfi_extension() {
    static sdl3_ext extinst{};
    return &extinst;
}

void remove_dfi_extension(dfi::extension_interface *) {
}
