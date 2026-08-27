#include "zie/api/ResilientEventBus.hpp"

#include <algorithm>

namespace zie::api {
namespace {

bool known(const EventCategory value) {
  switch (value) {
    case EventCategory::command_accepted:
    case EventCategory::command_rejected:
    case EventCategory::capability_changed:
    case EventCategory::sensor_observation:
    case EventCategory::lifecycle_changed:
      return true;
  }
  return false;
}

bool known(const BackpressurePolicy value) {
  switch (value) {
    case BackpressurePolicy::drop_newest:
      return true;
  }
  return false;
}

}  // namespace

bool ResilientEventBus::eligible(const Subscription& subscription) const {
  const auto* record =
      registry_.find(subscription.request.identity.package_id);
  return record != nullptr &&
         record->device_identity.logical.instance_id ==
             subscription.request.identity.logical_device_instance_id &&
         record->lifecycle == extensions::LifecycleState::active &&
         std::find(record->active_capabilities.begin(),
                   record->active_capabilities.end(),
                   subscription.request.required_capability) !=
             record->active_capabilities.end();
}

EventBusResult ResilientEventBus::subscribe(
    const EventSubscriptionRequest& request) {
  if (queue_depth_ == 0) return EventBusResult::rejected_invalid_bus;
  if (!known(policy_)) return EventBusResult::rejected_unknown_policy;
  if (request.identity.subscriber_id.empty() ||
      request.identity.package_id.empty() ||
      request.identity.logical_device_instance_id.empty() ||
      request.required_capability.empty() || request.categories.empty()) {
    return EventBusResult::rejected_invalid_subscription;
  }
  if (std::any_of(request.categories.begin(), request.categories.end(),
                  [](const EventCategory category) {
                    return !known(category);
                  })) {
    return EventBusResult::rejected_unknown_category;
  }
  if (std::find_if(subscriptions_.begin(), subscriptions_.end(),
                   [&request](const Subscription& subscription) {
                     return subscription.request.identity.subscriber_id ==
                            request.identity.subscriber_id;
                   }) != subscriptions_.end()) {
    return EventBusResult::rejected_duplicate_subscriber;
  }
  const auto* record = registry_.find(request.identity.package_id);
  if (record == nullptr || record->device_identity.logical.instance_id !=
                               request.identity.logical_device_instance_id) {
    return EventBusResult::rejected_registry_identity;
  }
  if (record->lifecycle != extensions::LifecycleState::active) {
    return EventBusResult::rejected_inactive_subscriber;
  }
  if (std::find(record->active_capabilities.begin(),
                record->active_capabilities.end(),
                request.required_capability) ==
      record->active_capabilities.end()) {
    return EventBusResult::rejected_missing_capability;
  }
  subscriptions_.push_back({request, {}});
  return EventBusResult::subscribed;
}

EventBusResult ResilientEventBus::unsubscribe(
    const std::string& subscriber_id) {
  const auto found = std::find_if(
      subscriptions_.begin(), subscriptions_.end(),
      [&subscriber_id](const Subscription& subscription) {
        return subscription.request.identity.subscriber_id == subscriber_id;
      });
  if (found == subscriptions_.end()) {
    return EventBusResult::rejected_not_found;
  }
  subscriptions_.erase(found);
  return EventBusResult::unsubscribed;
}

EventBusResult ResilientEventBus::publish(const RobotEvent& event) {
  EventJournal validator;
  const auto validation = validator.publish(event);
  if (validation == EventResult::rejected_unknown_category) {
    return EventBusResult::rejected_unknown_category;
  }
  if (validation == EventResult::rejected_unknown_source) {
    return EventBusResult::rejected_unknown_source;
  }
  if (validation != EventResult::published) {
    return EventBusResult::rejected_invalid_subscription;
  }
  bool overflow = false;
  bool ineligible = false;
  for (auto& subscription : subscriptions_) {
    if (std::find(subscription.request.categories.begin(),
                  subscription.request.categories.end(), event.category) ==
        subscription.request.categories.end()) {
      continue;
    }
    if (!eligible(subscription)) {
      subscription.queue.clear();
      ineligible = true;
      continue;
    }
    if (subscription.queue.size() >= queue_depth_) {
      overflow = true;
      continue;
    }
    subscription.queue.push_back(event);
  }
  if (overflow) return EventBusResult::published_with_overflow;
  if (ineligible) {
    return EventBusResult::published_with_ineligible_subscriber;
  }
  return EventBusResult::published;
}

EventBusResult ResilientEventBus::deliver_next(
    const std::string& subscriber_id,
    const std::function<void(const RobotEvent&)>& subscriber) {
  const auto found = std::find_if(
      subscriptions_.begin(), subscriptions_.end(),
      [&subscriber_id](const Subscription& subscription) {
        return subscription.request.identity.subscriber_id == subscriber_id;
      });
  if (found == subscriptions_.end()) {
    return EventBusResult::rejected_not_found;
  }
  if (!eligible(*found)) {
    found->queue.clear();
    return EventBusResult::rejected_inactive_subscriber;
  }
  if (found->queue.empty()) return EventBusResult::rejected_no_event;
  const RobotEvent event = found->queue.front();
  found->queue.pop_front();
  try {
    subscriber(event);
  } catch (...) {
    return EventBusResult::subscriber_failed;
  }
  return EventBusResult::delivered;
}

std::size_t ResilientEventBus::queued(
    const std::string& subscriber_id) const {
  const auto found = std::find_if(
      subscriptions_.begin(), subscriptions_.end(),
      [&subscriber_id](const Subscription& subscription) {
        return subscription.request.identity.subscriber_id == subscriber_id;
      });
  return found == subscriptions_.end() ? 0 : found->queue.size();
}

}  // namespace zie::api
