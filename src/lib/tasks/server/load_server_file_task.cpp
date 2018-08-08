#include "load_server_file_task.hpp"

#include "concurrency/logging/logger.hpp"
#include "storage/chunk.hpp"
#include "storage/storage_manager.hpp"
#include "utils/load_table.hpp"

namespace opossum {

void LoadServerFileTask::_on_execute() {
  try {
    const auto table = load_table(_file_name, Chunk::MAX_SIZE);
    StorageManager::get().add_table(_table_name, table);
    Logger::getInstance().load_table(_file_name, _table_name);
    _promise.set_value();
  } catch (const std::exception& exception) {
    _promise.set_exception(boost::current_exception());
  }
}

}  // namespace opossum
