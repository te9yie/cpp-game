#pragma once
#include <string>
#include <unordered_map>

#include "asset.h"
#include "t9/noncopyable.h"

namespace sai::asset {

// Manager.
class Manager : private t9::NonCopyable, private AssetRemoveObserver {
 private:
  using PathMap = std::unordered_multimap<std::uint32_t, HandleId>;

 private:
  AssetStorage assets_;
  PathMap path_map_;

 public:
  AssetHandle load(std::string_view path);
  AssetHandle find(std::string_view path);

  void update();

 private:
  // AssetRemoveObserver.
  virtual void on_remove(Asset* asset) override;
};

void update_manager(Manager* manager);

}  // namespace sai::asset