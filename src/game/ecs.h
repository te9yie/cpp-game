#pragma once
#include <string>

#include "frame.h"
#include "sai/task_arg.h"
#include "t9/ecs/entity.h"
#include "t9/ecs/query.h"
#include "t9/ecs/registry.h"
#include "t9/type_list.h"

namespace game {

namespace ecs = t9::ecs;

// Status Component.
struct Status : t9::NonCopyable {
  std::string name;
  int hp = 0;
};

void create_entities(ecs::Registry* reg);
void update_status(const Frame* frame,
                   ecs::Query<ecs::EntityId, Status&> query);
void show_status(ecs::Query<ecs::EntityId, const Status&> query);

}  // namespace game

namespace sai {

namespace ecs_ {

// ecs_arg_traits.
template <typename T>
struct ecs_arg_traits {
  static void set_permission(TaskArgsPermission* p) {
    p->reads.emplace(task_arg_type_index<T>::index());
  }
};

template <typename T>
struct ecs_arg_traits<const T&> {
  static void set_permission(TaskArgsPermission* p) {
    p->reads.emplace(task_arg_type_index<T>::index());
  }
};

template <typename T>
struct ecs_arg_traits<T&> {
  static void set_permission(TaskArgsPermission* p) {
    p->writes.emplace(task_arg_type_index<T>::index());
  }
};

}  // namespace ecs_

// task_arg_traits.
template <typename... Ts>
struct task_arg_traits<t9::ecs::Query<Ts...>> {
  static void set_permission(TaskArgsPermission*, t9::type_list<>) {}
  template <typename U, typename... Us>
  static void set_permission(TaskArgsPermission* p, t9::type_list<U, Us...>) {
    ecs_::ecs_arg_traits<U>::set_permission(p);
    set_permission(p, t9::type_list<Us...>{});
  }

  static void set_permission(TaskArgsPermission* p) {
    set_permission(p, t9::type_list<Ts...>{});
  }
  static t9::ecs::Query<Ts...> from_context(const TaskContext& ctx) {
    auto reg = ctx.get<t9::ecs::Registry>();
    return reg->query<Ts...>();
  }
};

}  // namespace sai