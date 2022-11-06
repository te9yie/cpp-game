#pragma once
#include <string>
#include <unordered_map>

#include "../job/executor.h"
#include "../task/args.h"
#include "asset.h"
#include "t9/noncopyable.h"

namespace sai::asset {

// Manager.
class Manager : private t9::NonCopyable, public AssetRemoveObserver {
 private:
  using PathMap = std::unordered_multimap<std::uint32_t, HandleId>;

 private:
  AssetStorage* assets_ = nullptr;
  PathMap path_map_;
  job::Executor executor_;

 public:
  Manager(AssetStorage* assets);

  bool start();
  void stop();

  AssetHandle load(std::string_view path);
  AssetHandle find(std::string_view path);

 public:
  // AssetRemoveObserver.
  virtual void on_remove(HandleId id, std::shared_ptr<Asset> asset) override;
};

bool init_manager(Manager* manager);
void update_manager(Manager* manager, AssetStorage* storage);

}  // namespace sai::asset

namespace sai::task {

// arg_traits.
template <>
struct arg_traits<asset::Manager*> {
  static void set_type_bits(ArgsTypeBits* bits) {
    bits->set_write<asset::Manager>();
    bits->set_write<asset::AssetStorage>();
  }
  static asset::Manager* to(const AppContext* ctx, TaskWork*) {
    return ctx->get<asset::Manager>();
  }
};

}  // namespace sai::task