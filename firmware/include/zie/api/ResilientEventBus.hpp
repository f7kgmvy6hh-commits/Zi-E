#pragma once

#include <cstddef>
#include <cstdint>
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
  subscriber_failed_retry_pending,
  subscriber_dead_lettered,
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
                    BackpressurePolicy policy,
                    std::size_t max_delivery_attempts = 3)
      : registry_(registry),
        queue_depth_(queue_depth),
        policy_(policy),
        max_delivery_attempts_(max_delivery_attempts) {}

  EventBusResult subscribe(const EventSubscriptionRequest& request);
  EventBusResult unsubscribe(const std::string& subscriber_id);
  EventBusResult publish(const RobotEvent& event);
  EventBusResult deliver_next(
      const std::string& subscriber_id,
      const std::function<void(const RobotEvent&)>& subscriber);
  std::size_t queued(const std::string& subscriber_id) const;
  std::size_t dead_lettered(const std::string& subscriber_id) const;

 private:
  struct Subscription {
    EventSubscriptionRequest request;
    std::uint64_t authorization_generation{0};
    struct PendingEvent {
      RobotEvent event;
      std::size_t failed_attempts{0};
    };
    std::deque<PendingEvent> queue;
    std::size_t dead_letter_count{0};
  };
  bool eligible(const Subscription& subscription) const;
  const extensions::ExtensionRegistry& registry_;
  std::size_t queue_depth_{0};
  BackpressurePolicy policy_{BackpressurePolicy::drop_newest};
  std::size_t max_delivery_attempts_{0};
  std::vector<Subscription> subscriptions_;
};

}  // namespace zie::api
