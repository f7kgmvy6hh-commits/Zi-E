#include "zie/presentation/PresentationEngine.hpp"

#include <algorithm>
#include <cctype>

#include "core/AuthoritativeRobotCore.hpp"

namespace zie::presentation {
namespace {
bool same_version(const extensions::ContractVersion a,
                  const extensions::ContractVersion b) {
  return a.major == b.major && a.minor == b.minor && a.patch == b.patch;
}
bool supported_schema(const extensions::ContractVersion v) {
  return v.major == 1 && v.minor == 0;
}
bool known(const PackDomain value) {
  switch (value) { case PackDomain::face: case PackDomain::sound: return true; }
  return false;
}
bool known(const SoundCueKind value) {
  switch (value) {
    case SoundCueKind::speech: case SoundCueKind::non_speech: return true;
  }
  return false;
}
bool key(const std::string& value) {
  return !value.empty() &&
         std::all_of(value.begin(), value.end(), [](unsigned char c) {
           return std::islower(c) || std::isdigit(c) || c == '.' || c == '-' ||
                  c == '_';
         });
}
template <typename T, typename Name>
bool duplicate(const std::vector<T>& values, Name name) {
  for (std::size_t i = 0; i < values.size(); ++i)
    for (std::size_t j = i + 1; j < values.size(); ++j)
      if (name(values[i]) == name(values[j])) return true;
  return false;
}
bool asset(const std::string& value) { return value.rfind("asset.", 0) == 0 && key(value); }
}  // namespace

PackResult PackCatalog::authorize(const PackIdentity& identity,
                                  const extensions::ExtensionCategory category,
                                  const bool require_active) const {
  if (!known(identity.domain)) return PackResult::rejected_unknown_domain;
  const auto* record = registry_.find(identity.package_id);
  if (record == nullptr || identity.pack_id != identity.package_id ||
      record->device_identity.logical.instance_id !=
          identity.logical_device_instance_id ||
      record->device_identity.hardware_profile.profile_id !=
          identity.hardware_profile_id ||
      !same_version(record->manifest.version, identity.version))
    return PackResult::rejected_registry_identity;
  if (record->manifest.extension_class != extensions::ExtensionClass::asset_pack ||
      record->manifest.category != category || !record->manifest.entrypoint.empty() ||
      !record->manifest.required_permissions.empty())
    return PackResult::rejected_data_boundary;
  if (require_active && record->lifecycle != extensions::LifecycleState::active)
    return PackResult::rejected_inactive_pack;
  if (require_active && std::find(record->active_capabilities.begin(),
                                  record->active_capabilities.end(),
                                  identity.registry_capability) ==
                            record->active_capabilities.end())
    return PackResult::rejected_missing_capability;
  return PackResult::validated;
}

PackResult PackCatalog::declare_pack(const FacePack& pack) {
  if (max_packs_ == 0 || max_contexts_ == 0)
    return PackResult::rejected_invalid_catalog;
  if (faces_.size() + sounds_.size() >= max_packs_)
    return PackResult::rejected_invalid_catalog;
  if (pack.identity.domain != PackDomain::face)
    return known(pack.identity.domain) ? PackResult::rejected_unknown_domain
                                       : PackResult::rejected_unknown_domain;
  const auto auth = authorize(pack.identity, extensions::ExtensionCategory::face_pack,
                              false);
  if (auth != PackResult::validated) return auth;
  if (std::any_of(faces_.begin(), faces_.end(), [&pack](const FaceRecord& r) {
        return r.pack.identity.pack_id == pack.identity.pack_id;
      })) return PackResult::rejected_invalid_identity;
  faces_.push_back({pack, PackState::declared});
  return PackResult::declared;
}

PackResult PackCatalog::declare_pack(const SoundPack& pack) {
  if (max_packs_ == 0 || max_contexts_ == 0 ||
      faces_.size() + sounds_.size() >= max_packs_)
    return PackResult::rejected_invalid_catalog;
  if (pack.identity.domain != PackDomain::sound)
    return PackResult::rejected_unknown_domain;
  const auto auth = authorize(pack.identity, extensions::ExtensionCategory::sound_pack,
                              false);
  if (auth != PackResult::validated) return auth;
  if (std::any_of(sounds_.begin(), sounds_.end(), [&pack](const SoundRecord& r) {
        return r.pack.identity.pack_id == pack.identity.pack_id;
      })) return PackResult::rejected_invalid_identity;
  sounds_.push_back({pack, PackState::declared});
  return PackResult::declared;
}

PackResult PackCatalog::validate(const std::string& id) {
  auto face = std::find_if(faces_.begin(), faces_.end(), [&id](const FaceRecord& r) {
    return r.pack.identity.pack_id == id;
  });
  if (face != faces_.end()) {
    const auto& p = face->pack;
    if (!supported_schema(p.identity.schema_version))
      return PackResult::rejected_unknown_schema;
    if (p.expressions.empty() || duplicate(p.expressions, [](const FaceExpression& e) { return e.name; }))
      return PackResult::rejected_duplicate_name;
    const auto has = [&p](const std::string& name) {
      return std::any_of(p.expressions.begin(), p.expressions.end(), [&name](const FaceExpression& e) { return e.name == name; });
    };
    if (!has(p.default_expression)) return PackResult::rejected_missing_default;
    if (!has(p.fallback_expression)) return PackResult::rejected_missing_fallback;
    for (const auto& e : p.expressions)
      if (!key(e.name) || !asset(e.asset_handle)) return PackResult::rejected_invalid_asset;
    for (const auto& t : p.transitions)
      if (!has(t.from_expression) || !has(t.to_expression) || !key(t.semantic_animation))
        return PackResult::rejected_unknown_reference;
    face->state = PackState::validated;
    return PackResult::validated;
  }
  auto sound = std::find_if(sounds_.begin(), sounds_.end(), [&id](const SoundRecord& r) { return r.pack.identity.pack_id == id; });
  if (sound == sounds_.end()) return PackResult::rejected_not_found;
  if (!supported_schema(sound->pack.identity.schema_version))
    return PackResult::rejected_unknown_schema;
  if (sound->pack.cues.empty() || duplicate(sound->pack.cues, [](const SoundCue& c) { return c.name; }))
    return PackResult::rejected_duplicate_name;
  for (const auto& cue : sound->pack.cues) {
    if (!known(cue.kind)) return PackResult::rejected_unknown_cue_kind;
    if (!key(cue.name) || !asset(cue.asset_handle)) return PackResult::rejected_invalid_asset;
  }
  sound->state = PackState::validated;
  return PackResult::validated;
}

PackCatalog::Context* PackCatalog::ensure_context(const std::string& id) {
  auto found = std::find_if(contexts_.begin(), contexts_.end(), [&id](const Context& c) { return c.id == id; });
  if (found != contexts_.end()) return &*found;
  if (id.empty() || contexts_.size() >= max_contexts_) return nullptr;
  contexts_.push_back({id, {}, {}});
  return &contexts_.back();
}

PackCatalog::Context* PackCatalog::find_context(const std::string& id) {
  const auto found = std::find_if(
      contexts_.begin(), contexts_.end(),
      [&id](const Context& selected) { return selected.id == id; });
  return found == contexts_.end() ? nullptr : &*found;
}

PackResult PackCatalog::activate_face(const std::string& context_id,
                                      const std::string& id) {
  auto found = std::find_if(faces_.begin(), faces_.end(), [&id](const FaceRecord& r) { return r.pack.identity.pack_id == id; });
  if (found == faces_.end()) return PackResult::rejected_not_found;
  if (found->state != PackState::validated) return PackResult::rejected_wrong_state;
  const auto auth = authorize(found->pack.identity, extensions::ExtensionCategory::face_pack, true);
  if (auth != PackResult::validated) return auth;
  auto* selected = ensure_context(context_id);
  if (selected == nullptr) return PackResult::rejected_invalid_catalog;
  const bool replaced = !selected->face_pack.empty() && selected->face_pack != id;
  selected->face_pack = id;
  return replaced ? PackResult::replaced : PackResult::activated;
}

PackResult PackCatalog::activate_sound(const std::string& context_id,
                                       const std::string& id) {
  auto found = std::find_if(sounds_.begin(), sounds_.end(), [&id](const SoundRecord& r) { return r.pack.identity.pack_id == id; });
  if (found == sounds_.end()) return PackResult::rejected_not_found;
  if (found->state != PackState::validated) return PackResult::rejected_wrong_state;
  const auto auth = authorize(found->pack.identity, extensions::ExtensionCategory::sound_pack, true);
  if (auth != PackResult::validated) return auth;
  auto* selected = ensure_context(context_id);
  if (selected == nullptr) return PackResult::rejected_invalid_catalog;
  const bool replaced = !selected->sound_pack.empty() && selected->sound_pack != id;
  selected->sound_pack = id;
  return replaced ? PackResult::replaced : PackResult::activated;
}

const FacePack* PackCatalog::active_face(const std::string& context_id) {
  auto* selected = find_context(context_id);
  if (selected == nullptr || selected->face_pack.empty()) return nullptr;
  auto found = std::find_if(faces_.begin(), faces_.end(), [selected](const FaceRecord& r) { return r.pack.identity.pack_id == selected->face_pack; });
  if (found == faces_.end() || authorize(found->pack.identity, extensions::ExtensionCategory::face_pack, true) != PackResult::validated) {
    const auto revoked_id = selected->face_pack;
    for (auto& context : contexts_)
      if (context.face_pack == revoked_id) context.face_pack.clear();
    return nullptr;
  }
  return &found->pack;
}

const SoundPack* PackCatalog::active_sound(const std::string& context_id) {
  auto* selected = find_context(context_id);
  if (selected == nullptr || selected->sound_pack.empty()) return nullptr;
  auto found = std::find_if(sounds_.begin(), sounds_.end(), [selected](const SoundRecord& r) { return r.pack.identity.pack_id == selected->sound_pack; });
  if (found == sounds_.end() || authorize(found->pack.identity, extensions::ExtensionCategory::sound_pack, true) != PackResult::validated) {
    const auto revoked_id = selected->sound_pack;
    for (auto& context : contexts_)
      if (context.sound_pack == revoked_id) context.sound_pack.clear();
    return nullptr;
  }
  return &found->pack;
}

PresentationResult PresentationEngine::consume(const api::AcceptedSemanticCommand& accepted) {
  const auto& command = accepted.command();
  if (command.type == api::SemanticCommandType::expression_intent) {
    const auto* pack = packs_.active_face(context_);
    if (pack == nullptr) return PresentationResult::rejected_no_active_pack;
    const auto requested = std::get<api::ExpressionIntent>(command.payload).expression;
    const auto expression = std::find_if(pack->expressions.begin(), pack->expressions.end(), [&requested](const FaceExpression& e) { return e.name == requested; });
    const std::string resolved = expression == pack->expressions.end()
                                     ? pack->fallback_expression
                                     : requested;
    const std::string from = state_.expression.empty() ||
                                     state_.face_pack_id != pack->identity.pack_id
                                 ? pack->default_expression
                                 : state_.expression;
    std::string transition = "direct";
    const auto route = std::find_if(pack->transitions.begin(), pack->transitions.end(), [&from, &resolved](const FaceTransition& t) { return t.from_expression == from && t.to_expression == resolved; });
    if (route != pack->transitions.end()) transition = route->semantic_animation;
    const std::uint64_t generation = state_.generation + 1;
    if (core_.update_state(robot_state_, {api::RobotStateCategory::presentation, generation, resolved}) != api::StateResult::accepted)
      return PresentationResult::state_update_failed;
    state_.generation = generation; state_.face_pack_id = pack->identity.pack_id;
    state_.expression = resolved; state_.transition = transition;
    return PresentationResult::expression_rendered;
  }
  if (command.type == api::SemanticCommandType::audio_cue_intent) {
    const auto* pack = packs_.active_sound(context_);
    if (pack == nullptr) return PresentationResult::rejected_no_active_pack;
    const auto requested = std::get<api::AudioCueIntent>(command.payload).cue;
    const auto cue = std::find_if(pack->cues.begin(), pack->cues.end(), [&requested](const SoundCue& c) { return c.name == requested; });
    if (cue == pack->cues.end()) return PresentationResult::rejected_unknown_sound_cue;
    const std::uint64_t generation = state_.generation + 1;
    if (core_.update_state(robot_state_, {api::RobotStateCategory::audio, generation, requested}) != api::StateResult::accepted)
      return PresentationResult::state_update_failed;
    state_.generation = generation; state_.sound_pack_id = pack->identity.pack_id; state_.sound_cue = requested;
    return PresentationResult::sound_rendered;
  }
  if (command.type == api::SemanticCommandType::audio_speech_intent) {
    const auto& utterance = std::get<api::AudioSpeechIntent>(command.payload).utterance;
    const std::uint64_t generation = state_.generation + 1;
    if (core_.update_state(robot_state_, {api::RobotStateCategory::audio, generation, "speech"}) != api::StateResult::accepted)
      return PresentationResult::state_update_failed;
    state_.generation = generation; state_.sound_cue = utterance;
    return PresentationResult::speech_presented;
  }
  return PresentationResult::rejected_unsupported_intent;
}

}  // namespace zie::presentation
