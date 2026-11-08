#pragma once

#include <atomic>
#include <cstdint>
#include <string>

namespace switchu::manager {

struct ReleaseInfo {
    std::string version;
    std::string name;
    std::string downloadUrl;
    std::string sha256;
    std::uint64_t downloadSize = 0;
    bool updateAvailable = false;
};

struct UpdateInstallResult {
    bool success = false;
    std::string version;
    std::string error;
    int failedStage = 0;
};

enum class UpdateWorkerStage : int {
    Idle,
    Downloading,
    Verifying,
    Extracting,
    Installing,
};

class ReleaseUpdater final {
public:
    static constexpr const char* kCurrentVersion = HOMENX_BUILD;
    // The older of the installed preview/HOME version markers; unknown when
    // either installation is missing so the first run restores both targets.
    static std::string installedVersion();

    static ReleaseInfo checkLatest();
    static UpdateInstallResult install(const ReleaseInfo& release,
                                       bool preserveDisabledOverride,
                                       std::atomic<float>& downloadProgress,
                                       std::atomic<float>& installProgress,
                                       std::atomic<int>& stage);
    static void shutdownNetwork();
    static void requestCancel();
    static void clearCancel();
    // Swaps in files an update couldn't replace while they were open. Call at
    // start-up before fonts or other assets are opened.
    static void applyPending();
};

} // namespace switchu::manager
