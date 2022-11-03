#include "manager.h"

#include <memory>

#include "t9/hash.h"

namespace sai::asset {

Manager::Manager() : executor_("AssetManager") {}

bool Manager::start() { return executor_.start(1); }
void Manager::stop() { executor_.stop(); }

AssetHandle Manager::load(std::string_view path) {
  auto hash = t9::fxhash(path.data(), path.length());
  auto r = path_map_.equal_range(hash);
  for (auto it = r.first; it != r.second; ++it) {
    auto id = it->second;
    auto a = assets_.get(id);
    if (a->path == path) return assets_.make_handle(id);
  }
  auto a = std::make_shared<Asset>(path);
  {
    auto job = std::make_shared<LoadAssetJob>(a);
    executor_.submit(std::move(job));
    executor_.kick();
  }
  auto handle = assets_.add(std::move(a));
  path_map_.emplace(hash, handle.id);
  return handle;
}

AssetHandle Manager::find(std::string_view path) {
  auto hash = t9::fxhash(path.data(), path.length());
  auto r = path_map_.equal_range(hash);
  for (auto it = r.first; it != r.second; ++it) {
    auto id = it->second;
    auto a = assets_.get(id);
    if (a->path == path) return assets_.make_handle(id);
  }
  return AssetHandle{};
}

void Manager::update() { assets_.update(this); }

/*virtual*/ void Manager::on_remove(HandleId id,
                                    std::shared_ptr<Asset> asset) /*override*/ {
  auto hash = t9::fxhash(asset->path.c_str(), asset->path.length());
  auto r = path_map_.equal_range(hash);
  for (auto it = r.first; it != r.second; ++it) {
    if (id == it->second) {
      path_map_.erase(it);
      break;
    }
  }
}

bool init_manager(Manager* manager) { return manager->start(); }
void update_manager(Manager* manager) { manager->update(); }

}  // namespace sai::asset
