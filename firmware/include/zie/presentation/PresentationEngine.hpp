#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "zie/api/SemanticRobotApi.hpp"

namespace zie::api { class AuthoritativeRobotCore; }

namespace zie::presentation {

enum class PackDomain { face, sound };
enum class SoundCueKind { speech, non_speech };
enum class PackState { declared, validated, active };

struct PackIdentity {
  std::string pack_id;
  extensions::ContractVersion version{};
  extensions::ContractVersion schema_version{};
  PackDomain domain{PackDomain::face};
  std::string package_id;
  std::string logical_device_instance_id;
  std::string hardware_profile_id;
  std::string registry_capability;
};

struct SemanticParameter { std::string name; std::string default_value; };
struct FaceExpression {
  std::string name;
  std::string asset_handle;
  std::vector<SemanticParameter> parameters;
};
struct FaceTransition {
  std::string from_expression;
  std::string to_expression;
  std::string semantic_animation;
};
struct FacePack {
  PackIdentity identity;
  std::string default_expression;
  std::string fallback_expression;
  std::vector<FaceExpression> expressions;
  std::vector<FaceTransition> transitions;
};

struct SoundCue {
  std::string name;
  SoundCueKind kind{SoundCueKind::non_speech};
  std::string asset_handle;
};
struct SoundPack { PackIdentity identity; std::vector<SoundCue> cues; };

enum class PackResult {
  declared,
  validated,
  activated,
  replaced,
  rejected_invalid_catalog,
  rejected_invalid_identity,
  rejected_registry_identity,
  rejected_data_boundary,
  rejected_unknown_schema,
  rejected_unknown_domain,
  rejected_duplicate_name,
  rejected_missing_default,
  rejected_missing_fallback,
  rejected_unknown_reference,
  rejected_invalid_asset,
  rejected_unknown_cue_kind,
  rejected_not_found,
  rejected_wrong_state,
  rejected_inactive_pack,
  rejected_missing_capability,
};

class PackCatalog {
 public:
  PackCatalog(const extensions::ExtensionRegistry& registry,
              std::size_t max_packs, std::size_t max_contexts)
      : registry_(registry), max_packs_(max_packs), max_contexts_(max_contexts) {}
  PackResult declare_pack(const FacePack& pack);
  PackResult declare_pack(const SoundPack& pack);
  PackResult validate(const std::string& pack_id);
  PackResult activate_face(const std::string& context,
                           const std::string& pack_id);
  PackResult activate_sound(const std::string& context,
                            const std::string& pack_id);
  const FacePack* active_face(const std::string& context);
  const SoundPack* active_sound(const std::string& context);

 private:
  struct FaceRecord { FacePack pack; PackState state{PackState::declared}; };
  struct SoundRecord { SoundPack pack; PackState state{PackState::declared}; };
  struct Context { std::string id; std::string face_pack; std::string sound_pack; };
  PackResult authorize(const PackIdentity& identity,
                       extensions::ExtensionCategory category,
                       bool require_active) const;
  Context* context(const std::string& id);
  const extensions::ExtensionRegistry& registry_;
  std::size_t max_packs_{0};
  std::size_t max_contexts_{0};
  std::vector<FaceRecord> faces_;
  std::vector<SoundRecord> sounds_;
  std::vector<Context> contexts_;
};

enum class PresentationResult {
  expression_rendered,
  sound_rendered,
  speech_presented,
  rejected_unsupported_intent,
  rejected_no_active_pack,
  rejected_unknown_expression,
  rejected_unknown_sound_cue,
  state_update_failed,
};

struct PresentationSnapshot {
  std::uint64_t generation{0};
  std::string face_pack_id;
  std::string expression;
  std::string transition;
  std::string sound_pack_id;
  std::string sound_cue;
};

class PresentationEngine {
 public:
  PresentationEngine(const api::AuthoritativeRobotCore& core,
                     api::RobotStateStore& robot_state, PackCatalog& packs,
                     std::string context)
      : core_(core), robot_state_(robot_state), packs_(packs),
        context_(std::move(context)) {}
  PresentationResult consume(const api::AcceptedSemanticCommand& accepted);
  const PresentationSnapshot& current() const { return state_; }

 private:
  const api::AuthoritativeRobotCore& core_;
  api::RobotStateStore& robot_state_;
  PackCatalog& packs_;
  std::string context_;
  PresentationSnapshot state_;
};

}  // namespace zie::presentation
