#pragma once

#include <cstddef>
#include <deque>
#include <functional>
#include <string>
#include <vector>

#include "zie/api/SemanticRobotApi.hpp"

namespace zie::api {

enum class BackpressurePolicy { drop_newest };

struct EventSubscriberIdentity {
  std::string subscriber_id;
  std::string package_id;
  std::string logical_device_instance_id;
};

struct EventSubscriptionRequest {
  EventSubscriberIdentity identity;
  std::string required_capability;
  std::vector<EventCategory> categories;
};

enum class EventBusResult {
  subscribed,
  unsubscribed,
  published,
  published_with_overflow,
  published_with_ineligible_subscriber,
  delivered,
  subscriber_failed,
  rejected_invalid_bus,
  rejected_invalid_subscription,
  rejected_duplicate_subscriber,
  rejected_unknown_category,
  rejected_unknown_source,
  rejected_unknown_policy,
  rejected_not_found,
  rejected_registry_identity,
  rejected_inactive_subscriber,
  rejected_missing_capability,
  rejected_no_event,
};

class ResilientEventBus {
 public:
  ResilientEventBus(const extensions::ExtensionRegistry& registry,
                    std::size_t queue_depth,
                    BackpressurePolicy policy)
      : registry_(registry), queue_depth_(queue_depth), policy_(policy) {}

  EventBusResult subscribe(const EventSubscriptionRequest& request);
  EventBusResult unsubscribe(const std::string& subscriber_id);
  EventBusResult publish(const RobotEvent& event);
  EventBusResult deliver_next(
      const std::string& subscriber_id,
      const std::function<void(const RobotEvent&)>& subscriber);
  std::size_t queued(const std::string& subscriber_id) const;

 private:
  struct Subscription {
    EventSubscriptionRequest request;
    std::deque<RobotEvent> queue;
  };
  bool eligible(const Subscription& subscription) const;
  const extensions::ExtensionRegistry& registry_;
  std::size_t queue_depth_{0};
  BackpressurePolicy policy_{BackpressurePolicy::drop_newest};
  std::vector<Subscription> subscriptions_;
};

}  // namespace zie::api
