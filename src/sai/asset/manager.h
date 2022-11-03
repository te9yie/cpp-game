#pragma once
#include <string>
#include <unordered_map>

#include "../job/executor.h"
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
  job::Executor executor_;

 public:
  Manager();

  bool start();
  void stop();

  AssetHandle load(std::string_view path);
  AssetHandle find(std::string_view path);

  void update();

 private:
  // AssetRemoveObserver.
  virtual void on_remove(HandleId id, std::shared_ptr<Asset> asset) override;
};

bool init_manager(Manager* manager);
void update_manager(Manager* manager);

}  // namespace sai::asset
