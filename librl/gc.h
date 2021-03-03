#pragma once

#include <cassert>
#include <functional>
#include <list>
#include <set>


namespace librl {

struct gc_base_t;

using gc_enum_t = std::function<void(gc_base_t *)>;

struct gc_base_t {
  virtual ~gc_base_t() {}
protected:
  friend struct gc_t;
  virtual void _enumerate(gc_enum_t &func) = 0;
};

struct gc_t {

  // constructor
  gc_t();

  // release all objects
  ~gc_t();

  // allocate an object
  template <class T, class... Types> T *alloc(Types &&... args) {
    // check allocation derives from object_t
    static_assert(std::is_base_of<gc_base_t, T>::value,
                  "type must derive from object_t");
    // perform the allocation and call constructor
    T *obj = new T(std::forward<Types>(args)...);
    // add to 'allocated' list
    _allocs.push_back(obj);
    // newly created objects are valid for the first cycle which allows us to
    // do incremental collection
    _valid.insert(obj);
    return obj;
  }

  // invoke a full collection sweep
  void collect();

  // check in a known live object
  void check_in(gc_base_t *);

  // return the number of allocations
  size_t num_allocs() const {
    return _allocs.size();
  }

  // persistant objects
  std::set<gc_base_t *> persist;

protected:
  // set of all allocations
  std::list<gc_base_t *> _allocs;

  // set of user provided known valid objects
  std::set<gc_base_t *> _valid;

  // set of objects marked as visited
  std::set<gc_base_t *> _mark;
  gc_enum_t _bounce;

  // follow an object and its associates
  void _follow(gc_base_t *obj, std::set<gc_base_t *> &out);
};

}  // namespace librl
