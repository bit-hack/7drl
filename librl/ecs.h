#pragma once
#include <cassert>
#include <unordered_map>
#include <unordered_set>
#include <string>

#include "common.h"


namespace librl {

struct ecs_manager_t;

typedef uint32_t ecs_id_t;

struct ecs_pos_t {
  int2 pos;
};

struct ecs_name_t {
  std::string name;
};

struct ecs_think_t {
  ecs_think_t(ecs_id_t id) : id(id) {}
  const ecs_id_t id;
  virtual void think(ecs_manager_t &man) = 0;
};

struct ecs_render_t {
  ecs_render_t(ecs_id_t id) : id(id) {}
  const ecs_id_t id;
  virtual void render(ecs_manager_t &man) = 0;
};

template <typename type_t>
struct ecs_store_t {

  ~ecs_store_t() {
    clear();
  }

  void clear() {
    auto itt = map.begin();
    while (itt != map.end()) {
      delete itt->second;
      itt = std::next(itt);
    }
    map.clear();
  }

  type_t *get(ecs_id_t id) {
    auto itt = map.find(id);
    assert(itt != map.end());
    return itt->second;
  }

  type_t *try_get(ecs_id_t id) {
    auto itt = map.find(id);
    return (itt == map.end()) ? nullptr : itt->second;
  }

  type_t *insert(ecs_id_t id, type_t *inst) {
    assert(map.find(id) == map.end());
    return map[id] = inst;
  }

  void collect(const std::unordered_map<ecs_id_t, int32_t> &refs) {
    auto itt = map.begin();
    while (itt != map.end()) {
      if (refs.count(itt->first)) {
        itt = std::next(itt);
      }
      else {
        delete itt->second;
        itt = map.erase(itt);
      }
    }
  }

  const auto &get_all() const {
    return map;
  }

protected:
  std::unordered_map<ecs_id_t, type_t*> map;
};

struct ecs_manager_t {

  ecs_manager_t()
    : id_next(0) {
  }

  ecs_id_t id_new() {
    // create a new refcount for this entity
    assert(ref_count.find(id_next) == ref_count.end());
    ref_count[id_next] = 1;
    return id_next++;
  }

  void id_aquire(ecs_id_t id) {
    auto itt = ref_count.find(id);
    assert(itt != ref_count.end());
    ++itt->second;
  }

  void id_release(ecs_id_t id) {
    auto itt = ref_count.find(id);
    assert(itt != ref_count.end());
    assert(itt->second > 0);
    if (--itt->second == 0) {
      // remove from the ref_count list
      ref_count.erase(itt);
    }
  }

  ecs_store_t<ecs_pos_t> pos;
  ecs_store_t<ecs_name_t> name;
  ecs_store_t<ecs_think_t> think;
  ecs_store_t<ecs_render_t> render;

  virtual void gc_collect() {
    pos.collect(ref_count);
    name.collect(ref_count);
    think.collect(ref_count);
    render.collect(ref_count);
  }

protected:
  std::unordered_map<ecs_id_t, int32_t> ref_count;

  ecs_id_t id_next;
};

}  // namespace librl
