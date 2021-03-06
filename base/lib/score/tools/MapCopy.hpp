#pragma once
#include <score/model/EntityMap.hpp>
#include <vector>

template <typename T>
std::vector<T*> shallow_copy(const IdContainer<T>& m)
{
  return {m.get().begin(), m.get().end()};
}

template <typename T>
std::vector<T*> shallow_copy(const score::EntityMap<T>& m)
{
  return shallow_copy(m.map());
}
