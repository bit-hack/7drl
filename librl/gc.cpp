#include "gc.h"

namespace librl {

gc_t::gc_t() {
  _bounce = [&](gc_base_t *obj) {
    if (obj) {
      _follow(obj, _mark);
    }
  };
}

void gc_t::_follow(gc_base_t *obj, std::set<gc_base_t *> &out) {
  if (obj && out.find(obj) == out.end()) {
    out.insert(obj);
    obj->_enumerate(_bounce);
  }
}

void gc_t::collect() {
  _mark.clear();
  // check in persistent objects
  for (auto &obj : persist) {
    _valid.insert(obj);
  }
  for (auto &obj : _valid) {
    if (obj) {
      _follow(obj, _mark);
    }
  }
  for (auto itt = _allocs.begin(); itt != _allocs.end();) {
    if (_mark.find(*itt) == _mark.end()) {
      assert(*itt);
      delete *itt;
      itt = _allocs.erase(itt);
    } else {
      ++itt;
    }
  }
  _valid.clear();
}

void gc_t::check_in(gc_base_t *obj) {
  _valid.insert(obj);
}

gc_t::~gc_t() {
  for (auto &o : _allocs) {
    delete o;
  }
  _allocs.clear();
}

}  // namespace librl
