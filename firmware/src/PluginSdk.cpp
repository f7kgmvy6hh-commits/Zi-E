#include "zie/sdk/PluginSdk.hpp"

namespace zie::sdk {
namespace {

bool less(const SemanticVersion left, const SemanticVersion right) {
  if (left.major != right.major) return left.major < right.major;
  if (left.minor != right.minor) return left.minor < right.minor;
  return left.patch < right.patch;
}

bool zero(const SemanticVersion version) {
  return version.major == 0 && version.minor == 0 && version.patch == 0;
}

}  // namespace

ContractCompatibility check_contract_compatibility(
    const ContractRange requested) {
  const auto supported = supported_contract_range();
  if (zero(requested.minimum) || zero(requested.maximum) ||
      less(requested.maximum, requested.minimum)) {
    return ContractCompatibility::invalid_range;
  }
  if (requested.minimum.major != supported.minimum.major ||
      requested.maximum.major != supported.maximum.major) {
    return ContractCompatibility::incompatible_major;
  }
  if (less(supported.maximum, requested.minimum) ||
      less(requested.maximum, supported.minimum)) {
    return ContractCompatibility::unsupported_version;
  }
  return ContractCompatibility::compatible;
}

}  // namespace zie::sdk
