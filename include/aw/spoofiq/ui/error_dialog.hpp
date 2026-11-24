#ifndef AW_SPOOFIQ_UI_ERROR_DIALOG_HPP
#define AW_SPOOFIQ_UI_ERROR_DIALOG_HPP

#include <string>
#include <functional>

namespace aw::spoofiq::ui {

class ErrorDialog {
public:
    static void show(const std::string& title, const std::string& message);
    static void render();  // Call from main render loop
    static bool is_open();
    
private:
    static inline bool open_ = false;
    static inline std::string title_;
    static inline std::string message_;
};

} // namespace aw::spoofiq::ui

#endif // AW_SPOOFIQ_UI_ERROR_DIALOG_HPP
