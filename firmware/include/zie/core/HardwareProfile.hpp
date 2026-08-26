#pragma once
#include <string>
#include <vector>
#include "zie/core/HardwareRegistry.hpp"

namespace zie::core {
enum class ProfileId { bench_minimal, stage1, full_prototype };
struct ProfileValidation {
  bool valid{false};
  std::vector<std::string> missing_required;
};
ProfileValidation validate_profile(ProfileId profile, const HardwareRegistry& hardware);
const char* profile_name(ProfileId profile);
}  // namespace zie::core
