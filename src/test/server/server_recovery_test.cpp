#include <pqxx/pqxx>

#include <thread>
#include <exception>
#include <boost/filesystem.hpp>

#include "base_test.hpp"
#include "scheduler/node_queue_scheduler.hpp"
#include "scheduler/topology.hpp"

#include "server/server.hpp"
#include "concurrency/logging/logger.hpp"

namespace opossum {

class ServerRecoveryTest : public BaseTestWithParam<Logger::Implementation> {
 protected:
  
  static constexpr char _folder[6] = "data/";

  void start_server(Logger::Implementation implementation) {
    StorageManager::get().reset();
    SQLQueryCache<SQLQueryPlan>::get().clear();

    // just to be sure, since it is essential for these tests
    EXPECT_FALSE(StorageManager::get().has_table("a_table"));

    // Set scheduler so that the server can execute the tasks on separate threads.
    CurrentScheduler::set(std::make_shared<NodeQueueScheduler>(Topology::create_numa_topology()));

    uint16_t server_port = 0;
    std::mutex mutex{};
    std::condition_variable cv{};

    // run on port 0 so the server can pick a free one
    auto server_runner = [&](boost::asio::io_service& io_service) {
      Server server{io_service, /* port = */ 0, test_data_path + _folder, implementation};

      {
        std::unique_lock<std::mutex> lock{mutex};
        server_port = server.get_port_number();
      }

      cv.notify_one();

      io_service.run();
    };

    _io_service = std::make_unique<boost::asio::io_service>();
    _server_thread = std::make_unique<std::thread>(server_runner, std::ref(*_io_service));

    // We need to wait here for the server to have started so we can get its port, which must be set != 0
    {
      std::unique_lock<std::mutex> lock{mutex};
      cv.wait(lock, [&] { return server_port != 0; });
    }

    // Get randomly assigned port number for client connection
    _connection_string = "hostaddr=127.0.0.1 port=" + std::to_string(server_port);

    // Logger needs to be reconstructed since it gets deconstructed on every server shutdown
    Logger::_reconstruct();
  }

  void shutdown_server() {
    // Give the server time to shut down gracefully before force-closing the socket it's working on
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    _io_service->stop();
    _server_thread->join();
  }

  void TearDown() override {
    shutdown_server();
    Logger::_set_implementation(Logger::Implementation::No);
    Logger::delete_log_files();
  }

  void restart_server(Logger::Implementation implementation) {
    shutdown_server();
    // set NoLogger since the server expects NoLogger on startup
    Logger::_set_implementation(Logger::Implementation::No);
    start_server(implementation);
  }

  std::unique_ptr<boost::asio::io_service> _io_service;
  std::unique_ptr<std::thread> _server_thread;
  std::string _connection_string;
};

TEST_P(ServerRecoveryTest, TestSimpleInsert) {
  start_server(GetParam());

  pqxx::connection connection{_connection_string};
  pqxx::nontransaction transaction{connection};

  EXPECT_THROW(transaction.exec("SELECT * FROM a_table;"), std::exception);

  auto a_table = load_table("src/test/tables/int_float.tbl", 2);

  transaction.exec("load src/test/tables/int_float.tbl a_table;");
  const auto result = transaction.exec("SELECT * FROM a_table;");
  EXPECT_EQ(result.size(), a_table->row_count());

  transaction.exec("INSERT INTO a_table VALUES (1, 1.0);");
  transaction.exec("INSERT INTO a_table VALUES (994, 994.0);");
  transaction.exec("DELETE FROM a_table WHERE a = 123");
  transaction.exec("UPDATE a_table SET a = 7, b = 7.2 WHERE a = 1234");

  restart_server(GetParam());

  pqxx::connection connection2{_connection_string};
  pqxx::nontransaction transaction2{connection2};
  const auto result2 = transaction2.exec("SELECT * FROM a_table;");

  EXPECT_EQ(result2.size(), a_table->row_count() + 1);
}

Logger::Implementation logging_implementations[] = {
  Logger::Implementation::Simple, 
  Logger::Implementation::GroupCommit
};

auto formatter = [](const testing::TestParamInfo<Logger::Implementation> info) {
  switch (info.param) {
    case Logger::Implementation::Simple: return "Simple";
    case Logger::Implementation::GroupCommit: return "GroupCommit";
    default: return "unknown";
  }
};

INSTANTIATE_TEST_CASE_P(logging_implementations, ServerRecoveryTest, ::testing::ValuesIn(logging_implementations), formatter);

}  // namespace opossum
