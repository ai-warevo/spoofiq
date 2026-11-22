#pragma once

#include <aw/spoofiq/core/types.hpp>
#include <aw/spoofiq/core/metadata.hpp>
#include <aw/spoofiq/proc/image_processor.hpp>
#include <filesystem>
#include <string>
#include <vector>

namespace aw::spoofiq::ui {

class BatchSpooferPanel : public IPanel {
public:
    explicit BatchSpooferPanel();
    
    PanelId id() const override { return PanelId::BatchSpoofer; }
    std::string name() const override { return "Batch Spoofer"; }
    void render() override;

private:
    // UI state
    std::string input_paths_buffer_;
    std::string output_dir_;
    std::string status_message_;
    bool processing_ = false;
    
    // Image processing
    core::ExifData exif_data_;
    int next_index_ = 1;
    
    void process_batch();
};

} // namespace aw::spoofiq::ui
