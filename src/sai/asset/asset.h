#pragma once
#include <string>
#include <vector>

#include "../handle.h"
#include "../job/job.h"
#include "../task/event.h"

namespace sai::asset {

// Asset.
struct Asset {
  std::string path;
  std::vector<std::uint8_t> data;
  bool is_not_found = false;
  bool is_loaded = false;

  Asset(std::string_view p) : path(p) {}
};

using AssetHandle = Handle<Asset>;
using AssetEvent = HandleEvent<Asset>;
using AssetRemoveObserver = HandleRemoveObserver<Asset>;
using AssetStorage = HandleStorage<Asset>;

// LoadAssetJob.
class LoadAssetJob : public job::Job {
 private:
  std::shared_ptr<Asset> asset_;

 public:
  explicit LoadAssetJob(std::shared_ptr<Asset> p);

 protected:
  virtual void on_exec() override;
};

// load_asset.
bool load_asset(Asset* a);

}  // namespace sai::asset
