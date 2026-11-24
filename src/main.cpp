#include "gl_loader.hpp"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <cstdlib>
#include <iostream>
#include <memory>

/* CLI mode */
#include <aw/spoofiq/cli/cli_parser.hpp>
#include <aw/spoofiq/cli/cli_runner.hpp>

/* Panel implementations */
#include <aw/spoofiq/ui/ui.hpp>
#include <aw/spoofiq/ui/panels/batch_spoofer_panel.hpp>
#include <aw/spoofiq/ui/panels/presets_panel.hpp>
#include <aw/spoofiq/ui/panels/console_panel.hpp>
#include <aw/spoofiq/ui/panels/appearance_panel.hpp>
#include <aw/spoofiq/logging/logger.hpp>
#include <aw/spoofiq/event_bus/event_bus_manager.hpp>
#include <aw/spoofiq/proc/task_executor.hpp>
#include <aw/spoofiq/config/config_manager.hpp>

    namespace {
    void glfw_error_callback(int error, const char* description) {
        std::cerr << "GLFW Error " << error << ": " << description << "\n";
    }

    void setup_imgui_style(bool dark_mode) {
        if (dark_mode) {
            ImGui::StyleColorsDark();
        } else {
            ImGui::StyleColorsLight();
        }

        /* Apply design_sense: dark tonal palette with rounded geometry */
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 8.0f;
        style.FrameRounding = 999.0f; // Pill-shaped buttons
        style.ScrollbarRounding = 999.0f;
        style.GrabRounding = 999.0f;
        style.TabRounding = 4.0f;

        if (dark_mode) {
            ImVec4* colors = style.Colors;
            colors[ImGuiCol_WindowBg] = ImVec4(0.04f, 0.05f, 0.07f, 1.00f);       // #0A0D12
            colors[ImGuiCol_ChildBg] = ImVec4(0.06f, 0.08f, 0.11f, 1.00f);        // #0F131C
            colors[ImGuiCol_FrameBg] = ImVec4(0.09f, 0.11f, 0.17f, 1.00f);        // #161D2B
            colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.16f, 0.21f, 1.00f); // #1E2636
            colors[ImGuiCol_FrameBgActive] = ImVec4(0.22f, 0.74f, 0.97f, 0.30f);  // Accent #38BDF8
            colors[ImGuiCol_TitleBg] = ImVec4(0.02f, 0.03f, 0.05f, 1.00f);        // #05070C
            colors[ImGuiCol_TitleBgActive] = ImVec4(0.04f, 0.05f, 0.07f, 1.00f);  // #0A0D12
            colors[ImGuiCol_Button] = ImVec4(0.22f, 0.74f, 0.97f, 0.40f);         // Accent
            colors[ImGuiCol_ButtonHovered] = ImVec4(0.22f, 0.74f, 0.97f, 0.60f);
            colors[ImGuiCol_ButtonActive] = ImVec4(0.22f, 0.74f, 0.97f, 0.80f);
            colors[ImGuiCol_Header] = ImVec4(0.22f, 0.74f, 0.97f, 0.30f);
            colors[ImGuiCol_HeaderHovered] = ImVec4(0.22f, 0.74f, 0.97f, 0.50f);
            colors[ImGuiCol_HeaderActive] = ImVec4(0.22f, 0.74f, 0.97f, 0.70f);
        }
    }

} // namespace

int main(int argc, char* argv[]) {
    // Check for --help first
    if (aw::spoofiq::cli::is_help_requested(argc, argv)) {
        aw::spoofiq::cli::print_help(argv[0]);
        return 0;
    }

    // Check for --version
    if (aw::spoofiq::cli::is_version_requested(argc, argv)) {
        aw::spoofiq::cli::print_version();
        return 0;
    }

    // Initialize logger early for both CLI and GUI modes
    aw::spoofiq::Logger::init();

    /* Check for CLI mode */
    if (aw::spoofiq::cli::is_cli_mode(argc, argv)) {
        // Parse CLI arguments
        auto cli_result = aw::spoofiq::cli::parse_cli_args(argc, argv);
        if (!cli_result) {
            std::cerr << "CLI Error: " << cli_result.error() << "\n";
            aw::spoofiq::Logger::error("CLI Parse Error: {}", cli_result.error());
            return 1;
        }

        // Load application settings (for presets)
        auto settings_result = aw::spoofiq::ConfigManager::load();
        if (!settings_result) {
            std::cerr << "Config Error: " << settings_result.error() << "\n";
            aw::spoofiq::Logger::error("Config Load Error: {}", settings_result.error());
            return 1;
        }

        // Initialize event bus and task executor
        aw::spoofiq::EventBusManager::init();
        aw::spoofiq::proc::TaskExecutor::init();

        // Run CLI
        int exit_code = aw::spoofiq::cli::run_cli(*cli_result, *settings_result);

        // Shutdown task executor
        aw::spoofiq::proc::TaskExecutor::shutdown(std::chrono::milliseconds(5000));

        return exit_code;
    }

    /* Initialize GLFW */
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(1280, 720, "spoofiq " APP_VERSION, nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    /* Setup Dear ImGui context */
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    /* Setup Platform/Renderer backends */
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    /* Initialize event bus */
    aw::spoofiq::EventBusManager::init();

    /* Initialize task executor */
    aw::spoofiq::proc::TaskExecutor::init();

    /* Application registry */
    aw::spoofiq ::PanelRegistry registry;

    /* Register panels */
    registry.register_panel(std::make_shared<aw::spoofiq::ui::BatchSpooferPanel>());
    registry.register_panel(std::make_shared<aw::spoofiq::ui::PresetsPanel>());
    registry.register_panel(std::make_shared<aw::spoofiq::ui::ConsolePanel>());
    registry.register_panel(std::make_shared<aw::spoofiq::ui::AppearancePanel>());

    /* UI root */
    aw::spoofiq ::UiRoot ui_root(registry);

    /* Setup initial style */
    setup_imgui_style(false);
    
#ifndef NDEBUG
    bool show_style_editor = false;
#endif

    /* Main loop */
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        /* Handle ESC quit */
        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        
#ifndef NDEBUG
        // F12 toggles style editor in debug builds only
        if (ImGui::IsKeyPressed(ImGuiKey_F12)) {
            show_style_editor = !show_style_editor;
        }
#endif

        /* Start ImGui frame */
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        /* Always render UI (no toggle) */
        ui_root.render();
        
#ifndef NDEBUG
        // Developer-only: show ImGui style editor when toggled
        if (show_style_editor) {
            ImGui::Begin("Debug: Style Editor", &show_style_editor);
            ImGui::ShowStyleEditor();
            ImGui::End();
        }
#endif

        /* Rendering */
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.02f, 0.03f, 0.05f, 1.00f); // #05070C
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    /* Graceful shutdown: wait for pending tasks (5s timeout) */
    aw::spoofiq::proc::TaskExecutor::shutdown(std::chrono::milliseconds(5000));

    /* Cleanup */
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
