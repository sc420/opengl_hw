#pragma once

#include "as/common.hpp"

namespace as {
/**
 * Index manager.
 *
 * Mapping Relationship:
 * 1. The target only binds a name (target-to-name)
 * 2. The name only binds an index (name-to-index)
 * 3. The index can be bound by many names (index-to-names)
 *
 * Procedure:
 * 1. Save index-to-names map
 * 2. When a target unbinds, remove the corresponding item in target-to-name and
 * name-to-index
 * 3. When the value of index-to-names becomes empty, remove the item in
 * index-to-names and mark the index unused
 */
template <class TTarget1, class TTarget2, class TIndex>
class IndexManager {
 public:
  IndexManager();

  void SetMaxIdx(const TIndex max_idx);

  void CheckMaxIdx(const TIndex idx) const;

  TIndex BindTarget1(const TTarget1 &target, const std::string &name);

  TIndex BindTarget2(const TTarget2 &target, const std::string &name);

  TIndex UnbindTarget1(const TTarget1 &target);

  TIndex UnbindTarget2(const TTarget2 &target);

 private:
  TIndex max_idx_;

  // Save used indexes so that we can get unused indexes efficiently
  std::set<TIndex> used_idxs_;

  std::map<TTarget1, std::string> target1_to_name_;

  std::map<TTarget2, std::string> target2_to_name_;

  std::map<std::string, TIndex> name_to_idx_;

  std::map<TIndex, std::set<std::string>> idx_to_names_;

  void InitUsedIdxs();

  TIndex GetUnusedIdx();

  TIndex GetNameToIdx(const std::string &name);

  template <class TTarget, class TTargetToName>
  void UnbindTarget(const TTarget &target, TTargetToName &target_to_name);

  void MarkIdxUnused(const TIndex idx);
};

template <class TTarget1, class TTarget2, class TIndex>
inline IndexManager<TTarget1, TTarget2, TIndex>::IndexManager() {
  InitUsedIdxs();
}

template <class TTarget1, class TTarget2, class TIndex>
inline void IndexManager<TTarget1, TTarget2, TIndex>::SetMaxIdx(
    const TIndex max_idx) {
  max_idx_ = max_idx;
}

template <class TTarget1, class TTarget2, class TIndex>
inline void IndexManager<TTarget1, TTarget2, TIndex>::CheckMaxIdx(
    const TIndex idx) const {
  if (idx >= max_idx_) {
    throw std::runtime_error("Index '" + std::to_string(idx) +
                             "' exceeds the limit '" +
                             std::to_string(max_idx_) + "'");
  }
}

template <class TTarget1, class TTarget2, class TIndex>
inline TIndex IndexManager<TTarget1, TTarget2, TIndex>::BindTarget1(
    const TTarget1 &target, const std::string &name) {
  target1_to_name_[target] = name;
  return GetNameToIdx(name);
}

template <class TTarget1, class TTarget2, class TIndex>
inline TIndex IndexManager<TTarget1, TTarget2, TIndex>::BindTarget2(
    const TTarget2 &target, const std::string &name) {
  target2_to_name_[target] = name;
  return GetNameToIdx(name);
}

template <class TTarget1, class TTarget2, class TIndex>
inline TIndex IndexManager<TTarget1, TTarget2, TIndex>::UnbindTarget1(
    const TTarget1 &target) {
  UnbindTarget(target, target1_to_name_);
  // Return default index
  return 0;
}

template <class TTarget1, class TTarget2, class TIndex>
inline TIndex IndexManager<TTarget1, TTarget2, TIndex>::UnbindTarget2(
    const TTarget2 &target) {
  UnbindTarget(target, target2_to_name_);
  // Return default index
  return 0;
}

template <class TTarget1, class TTarget2, class TIndex>
inline void IndexManager<TTarget1, TTarget2, TIndex>::InitUsedIdxs() {
  // Use 0 as default index to avoid programming errors
  used_idxs_.insert(0);
}

template <class TTarget1, class TTarget2, class TIndex>
inline TIndex IndexManager<TTarget1, TTarget2, TIndex>::GetUnusedIdx() {
  const auto last_it = used_idxs_.rbegin();
  const TIndex new_idx = *last_it + 1;
  CheckMaxIdx(new_idx);
  used_idxs_.insert(new_idx);
  return new_idx;
}

template <class TTarget1, class TTarget2, class TIndex>
inline TIndex IndexManager<TTarget1, TTarget2, TIndex>::GetNameToIdx(
    const std::string &name) {
  if (name_to_idx_.count(name) > 0) {
    return name_to_idx_.at(name);
  } else {
    // Get an unused index
    const TIndex idx = GetUnusedIdx();
    // Save name-to-index mapping
    name_to_idx_[name] = idx;
    // Save index-to-names mapping
    idx_to_names_[idx].insert(name);
    return idx;
  }
}

template <class TTarget1, class TTarget2, class TIndex>
inline void IndexManager<TTarget1, TTarget2, TIndex>::MarkIdxUnused(
    const TIndex idx) {
  used_idxs_.erase(idx);
}

template <class TTarget1, class TTarget2, class TIndex>
template <class TTarget, class TTargetToName>
inline void IndexManager<TTarget1, TTarget2, TIndex>::UnbindTarget(
    const TTarget &target, TTargetToName &target_to_name) {
  // Check whether the target-to-name item exists
  if (target_to_name.count(target) == 0) {
    return;
  }
  const std::string &name = target_to_name.at(target);
  const TIndex idx = name_to_idx_.at(name);
  // Remove the target from target-to-name map
  target_to_name.erase(target);
  // Remove the name from name-to-index map
  name_to_idx_.erase(name);
  // Remove the name from index-to-names map
  idx_to_names_.at(idx).erase(name);
  // Check whether the value of index-to-names is empty
  if (idx_to_names_.at(idx).empty()) {
    // Remove the index-to-names item
    idx_to_names_.erase(idx);
    // Mark the index unused
    MarkIdxUnused(idx);
  }
}

}  // namespace as
