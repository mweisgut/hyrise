#include "micro_benchmark_basic_fixture.hpp"

#include "benchmark_config.hpp"
#include "constant_mappings.hpp"
#include "expression/expression_functional.hpp"
#include "logical_query_plan/abstract_lqp_node.hpp"
#include "logical_query_plan/join_node.hpp"
#include "logical_query_plan/lqp_translator.hpp"
#include "logical_query_plan/predicate_node.hpp"
#include "logical_query_plan/projection_node.hpp"
#include "logical_query_plan/stored_table_node.hpp"
#include "operators/index_scan.hpp"
#include "operators/join_hash.hpp"
#include "operators/join_index.hpp"
#include "operators/join_nested_loop.hpp"
#include "operators/join_sort_merge.hpp"
#include "operators/table_scan.hpp"
#include "operators/table_wrapper.hpp"
#include "scheduler/current_scheduler.hpp"
#include "scheduler/operator_task.hpp"
#include "storage/chunk_encoder.hpp"
#include "storage/encoding_type.hpp"
#include "storage/index/b_tree/b_tree_index.hpp"
#include "storage/index/group_key/group_key_index.hpp"
#include "storage/storage_manager.hpp"
#include "storage/table.hpp"
#include "tpch/tpch_table_generator.hpp"

//#define DEBUG

using namespace opossum::expression_functional;  // NOLINT

namespace opossum {

class TableWrapper;

// Defining the base fixture class
class TPCHDataMicroBenchmarkFixture : public MicroBenchmarkBasicFixture {
 public:
  void SetUp(::benchmark::State& state) {
    auto& sm = StorageManager::get();
    const auto scale_factor = 1.0f;
    const auto default_encoding = EncodingType::Dictionary;

    auto benchmark_config = BenchmarkConfig::get_default_config();
    // TODO(anyone): setup benchmark_config with the given default_encoding
    // benchmark_config.encoding_config = EncodingConfig{SegmentEncodingSpec{default_encoding}};

    if (sm.tables().empty()) {
      std::cout << "Generating TPC-H data set with scale factor " << scale_factor << " and "
                << encoding_type_to_string.left.at(default_encoding) << " encoding:" << std::endl;
      TpchTableGenerator(scale_factor, std::make_shared<BenchmarkConfig>(benchmark_config)).generate_and_store();
    }

    _table_wrapper_map = create_table_wrappers(sm);

    auto lineitem_table = sm.get_table("lineitem");

    // TPC-H Q6 predicates. With an optimal predicate order (logical costs), discount (between on float) is first
    // executed, followed by shipdate <, followed by quantity, and eventually shipdate >= (note, order calculated
    // assuming non-inclusive between predicates are not yet supported).
    // This order is not necessarily the order Hyrise uses (estimates can be vastly off) or which will eventually
    // be calculated by more sophisticated cost models.
    _tpchq6_discount_operand = pqp_column_(ColumnID{6}, lineitem_table->column_data_type(ColumnID{6}),
                                           lineitem_table->column_is_nullable(ColumnID{6}), "");
    _tpchq6_discount_predicate =
        std::make_shared<BetweenExpression>(_tpchq6_discount_operand, value_(0.05), value_(0.070001));

    _tpchq6_shipdate_less_operand = pqp_column_(ColumnID{10}, lineitem_table->column_data_type(ColumnID{10}),
                                                lineitem_table->column_is_nullable(ColumnID{10}), "");
    _tpchq6_shipdate_less_predicate = std::make_shared<BinaryPredicateExpression>(
        PredicateCondition::LessThan, _tpchq6_shipdate_less_operand, value_("1995-01-01"));

    _tpchq6_quantity_operand = pqp_column_(ColumnID{4}, lineitem_table->column_data_type(ColumnID{4}),
                                           lineitem_table->column_is_nullable(ColumnID{4}), "");
    _tpchq6_quantity_predicate =
        std::make_shared<BinaryPredicateExpression>(PredicateCondition::LessThan, _tpchq6_quantity_operand, value_(24));

    // The following two "synthetic" predicates have a selectivity of 1.0
    _lorderkey_operand = pqp_column_(ColumnID{0}, lineitem_table->column_data_type(ColumnID{0}),
                                     lineitem_table->column_is_nullable(ColumnID{0}), "");
    _int_predicate = std::make_shared<BinaryPredicateExpression>(PredicateCondition::GreaterThanEquals,
                                                                 _lorderkey_operand, value_(-5));

    _lshipinstruct_operand = pqp_column_(ColumnID{13}, lineitem_table->column_data_type(ColumnID{13}),
                                         lineitem_table->column_is_nullable(ColumnID{13}), "");
    _string_predicate =
        std::make_shared<BinaryPredicateExpression>(PredicateCondition::NotEquals, _lshipinstruct_operand, value_("a"));

    _orders_table_node = StoredTableNode::make("orders");
    _orders_orderpriority = _orders_table_node->get_column("o_orderpriority");
    _orders_orderdate = _orders_table_node->get_column("o_orderdate");
    _orders_orderkey = _orders_table_node->get_column("o_orderkey");

    _lineitem_table_node = StoredTableNode::make("lineitem");
    _lineitem_orderkey = _lineitem_table_node->get_column("l_orderkey");
    _lineitem_commitdate = _lineitem_table_node->get_column("l_commitdate");
    _lineitem_receiptdate = _lineitem_table_node->get_column("l_receiptdate");
  }

  // Required to avoid resetting of StorageManager in MicroBenchmarkBasicFixture::TearDown()
  void TearDown(::benchmark::State&) {}

  std::map<std::string, std::shared_ptr<TableWrapper>> create_table_wrappers(StorageManager& sm) {
    std::map<std::string, std::shared_ptr<TableWrapper>> wrapper_map;
    for (const auto& table_name : sm.table_names()) {
      auto table = sm.get_table(table_name);
      auto table_wrapper = std::make_shared<TableWrapper>(table);
      table_wrapper->execute();

      wrapper_map.emplace(table_name, table_wrapper);
    }

    return wrapper_map;
  }

  void print_table_row_count(const std::string& table_name) {
    const auto& table_wrapper = _table_wrapper_map.at(table_name);
    if (table_wrapper != nullptr && table_wrapper->get_output() != nullptr) {
      std::cout << table_name << ": " << table_wrapper->get_output()->row_count() << " rows" << std::endl;
    }
  }

  void remove_table_index(const std::shared_ptr<Table>& table, const std::vector<ColumnID>& index_column_ids) {
    // remove created chunk indices, since table has no function "remove_index",
    // we have to iterate over the chunks
    for (const auto& chunk : table->chunks()) {
      for (const auto& index : chunk->get_indices(index_column_ids)) {
        chunk->remove_index(index);
      }
    }
  }

  inline static bool _tpch_data_generated = false;

  std::map<std::string, std::shared_ptr<TableWrapper>> _table_wrapper_map;

  std::shared_ptr<PQPColumnExpression> _lorderkey_operand;
  std::shared_ptr<BinaryPredicateExpression> _int_predicate;
  std::shared_ptr<PQPColumnExpression> _lshipinstruct_operand;
  std::shared_ptr<BinaryPredicateExpression> _string_predicate;

  std::shared_ptr<PQPColumnExpression> _tpchq6_discount_operand;
  std::shared_ptr<BetweenExpression> _tpchq6_discount_predicate;
  std::shared_ptr<PQPColumnExpression> _tpchq6_shipdate_less_operand;
  std::shared_ptr<BinaryPredicateExpression> _tpchq6_shipdate_less_predicate;
  std::shared_ptr<PQPColumnExpression> _tpchq6_quantity_operand;
  std::shared_ptr<BinaryPredicateExpression> _tpchq6_quantity_predicate;

  std::shared_ptr<StoredTableNode> _orders_table_node, _lineitem_table_node;
  LQPColumnReference _orders_orderpriority, _orders_orderdate, _orders_orderkey;
  LQPColumnReference _lineitem_orderkey, _lineitem_commitdate, _lineitem_receiptdate;
};

BENCHMARK_F(TPCHDataMicroBenchmarkFixture, BM_TPCH_Q6_lineitem_shipdate_less_predicate_table_scan)
(benchmark::State& state) {
  for (auto _ : state) {
    const auto table_scan =
        std::make_shared<TableScan>(_table_wrapper_map.at("lineitem"), _tpchq6_shipdate_less_predicate);
    table_scan->execute();
  }
}

BENCHMARK_F(TPCHDataMicroBenchmarkFixture, BM_TPCH_Q6_lineitem_shipdate_less_predicate_groupKeyIndex_scan)
(benchmark::State& state) {
  // column "shipdate" of table "lineitem" has the column id 10
  const std::vector<ColumnID>& index_column_ids = {ColumnID{10}};
  const auto& right_values = std::vector<AllTypeVariant>{AllTypeVariant{"1995-01-01"}};

  const auto& lineitem_table = StorageManager::get().get_table("lineitem");
  lineitem_table->create_index<GroupKeyIndex>(index_column_ids);
  auto lineitem_table_wrapper = std::make_shared<TableWrapper>(lineitem_table);
  lineitem_table_wrapper->execute();

  for (auto _ : state) {
    const auto index_scan = std::make_shared<IndexScan>(lineitem_table_wrapper, SegmentIndexType::GroupKey,
                                                        index_column_ids, PredicateCondition::LessThan, right_values);
    index_scan->execute();
  }
}

BENCHMARK_F(TPCHDataMicroBenchmarkFixture, BM_TPCH_Q6_lineitem_shipdate_less_predicate_bTreeIndex_scan)
(benchmark::State& state) {
  // column "shipdate" of table "lineitem" has the column id 10
  const std::vector<ColumnID>& index_column_ids = {ColumnID{10}};
  const auto& right_values = std::vector<AllTypeVariant>{AllTypeVariant{"1995-01-01"}};

  const auto& lineitem_table = StorageManager::get().get_table("lineitem");
  lineitem_table->create_index<BTreeIndex>(index_column_ids);
  auto lineitem_table_wrapper = std::make_shared<TableWrapper>(lineitem_table);
  lineitem_table_wrapper->execute();

  for (auto _ : state) {
    const auto index_scan = std::make_shared<IndexScan>(lineitem_table_wrapper, SegmentIndexType::BTree,
                                                        index_column_ids, PredicateCondition::LessThan, right_values);
    index_scan->execute();
  }
}

BENCHMARK_F(TPCHDataMicroBenchmarkFixture, BM_TPCH_Q3_orders_orderdate_predicate_table_scan)(benchmark::State& state) {
  // TCP-H Q3, orders.o_orderdate
  // SELECT l_orderkey, SUM(l_extendedprice*(1.0-l_discount)) as revenue, o_orderdate, o_shippriority
  // FROM customer, orders, lineitem
  // WHERE c_mktsegment = 'BUILDING'
  //   AND c_custkey = o_custkey
  //   AND l_orderkey = o_orderkey
  //   AND o_orderdate < '1995-03-15'
  //   AND l_shipdate > '1995-03-15'
  // GROUP BY l_orderkey, o_orderdate, o_shippriority
  // ORDER BY revenue DESC, o_orderdate;

  const auto& orders_table = StorageManager::get().get_table("orders");

  const auto& tpchq3_orderdate_operand = pqp_column_(ColumnID{4}, orders_table->column_data_type(ColumnID{4}),
                                                     orders_table->column_is_nullable(ColumnID{4}), "");
  const auto& tpchq3_orderdate_predicate = std::make_shared<BinaryPredicateExpression>(
      PredicateCondition::LessThan, tpchq3_orderdate_operand, value_("1995-03-15"));

  for (auto _ : state) {
    const auto table_scan = std::make_shared<TableScan>(_table_wrapper_map.at("orders"), tpchq3_orderdate_predicate);
    table_scan->execute();
  }
}

BENCHMARK_F(TPCHDataMicroBenchmarkFixture, BM_TPCH_Q3_orders_orderdate_predicate_groupKeyIndex_scan)
(benchmark::State& state) {
  // column "orderdate" of table "orders" has the column id 4
  const std::vector<ColumnID>& index_column_ids = {ColumnID{4}};
  const auto& right_values = std::vector<AllTypeVariant>{AllTypeVariant{"1995-03-15"}};

  const auto& orders_table = StorageManager::get().get_table("orders");
  orders_table->create_index<GroupKeyIndex>(index_column_ids);
  auto orders_table_wrapper = std::make_shared<TableWrapper>(orders_table);
  orders_table_wrapper->execute();

  for (auto _ : state) {
    const auto index_scan = std::make_shared<IndexScan>(orders_table_wrapper, SegmentIndexType::GroupKey,
                                                        index_column_ids, PredicateCondition::LessThan, right_values);
    index_scan->execute();
  }
}

BENCHMARK_F(TPCHDataMicroBenchmarkFixture, BM_TPCH_Q3_orders_orderdate_predicate_bTreeIndex_scan)
(benchmark::State& state) {
  // column "orderdate" of table "orders" has the column id 4
  const std::vector<ColumnID>& index_column_ids = {ColumnID{4}};
  const auto& right_values = std::vector<AllTypeVariant>{AllTypeVariant{"1995-03-15"}};

  const auto& orders_table = StorageManager::get().get_table("orders");
  orders_table->create_index<BTreeIndex>(index_column_ids);
  auto orders_table_wrapper = std::make_shared<TableWrapper>(orders_table);
  orders_table_wrapper->execute();

  for (auto _ : state) {
    const auto index_scan = std::make_shared<IndexScan>(orders_table_wrapper, SegmentIndexType::BTree, index_column_ids,
                                                        PredicateCondition::LessThan, right_values);
    index_scan->execute();
  }
}

// Joins part.p_partkey (z.B.: Query 2, 8, 9, 14, 16, 17)

BENCHMARK_F(TPCHDataMicroBenchmarkFixture, BM_TPCH_pPartkey_psPartkey_hash_join)(benchmark::State& state) {
  const auto& left_table_name = "part";
  const auto& right_table_name = "partsupp";
  const auto& left_column_id = ColumnID{0};
  const auto& right_column_id = ColumnID{0};

  const auto& storage_manager = StorageManager::get();

  const auto& left_table = storage_manager.get_table(left_table_name);
  const auto& left_table_wrapper = std::make_shared<TableWrapper>(left_table);
  left_table_wrapper->execute();
#ifdef DEBUG
  print_table_row_count(left_table_name);
#endif
  const auto& right_table = storage_manager.get_table(right_table_name);
  const auto& right_table_wrapper = std::make_shared<TableWrapper>(right_table);
  right_table_wrapper->execute();
#ifdef DEBUG
  print_table_row_count(right_table_name);
#endif
  for (auto _ : state) {
    const auto hash_join = std::make_shared<JoinHash>(
        left_table_wrapper, right_table_wrapper, JoinMode::Inner,
        OperatorJoinPredicate{{left_column_id, right_column_id}, PredicateCondition::Equals});
    hash_join->execute();
    std::cout << "join result table: " << hash_join->get_output()->row_count() << " rows" << std::endl;
  }
}

BENCHMARK_F(TPCHDataMicroBenchmarkFixture, BM_TPCH_psPartkey_lPartkey_hash_join)(benchmark::State& state) {
  const auto& left_table_name = "partsupp";
  const auto& right_table_name = "lineitem";
  const auto& left_column_id = ColumnID{0};
  const auto& right_column_id = ColumnID{1};

  const auto& storage_manager = StorageManager::get();

  const auto& left_table = storage_manager.get_table(left_table_name);
  const auto& left_table_wrapper = std::make_shared<TableWrapper>(left_table);
  left_table_wrapper->execute();
#ifdef DEBUG
  print_table_row_count(left_table_name);
#endif
  const auto& right_table = storage_manager.get_table(right_table_name);
  const auto& right_table_wrapper = std::make_shared<TableWrapper>(right_table);
  right_table_wrapper->execute();
#ifdef DEBUG
  print_table_row_count(right_table_name);
#endif
  for (auto _ : state) {
    const auto hash_join = std::make_shared<JoinHash>(
        left_table_wrapper, right_table_wrapper, JoinMode::Inner,
        OperatorJoinPredicate{{left_column_id, right_column_id}, PredicateCondition::Equals});
    hash_join->execute();
#ifdef DEBUG
    std::cout << "join result table: " << hash_join->get_output()->row_count() << " rows" << std::endl;
#endif
  }
}

BENCHMARK_F(TPCHDataMicroBenchmarkFixture, BM_TPCH_pPartkey_lPartkey_hash_join)(benchmark::State& state) {
  const auto& left_table_name = "part";
  const auto& right_table_name = "lineitem";
  const auto& left_column_id = ColumnID{0};
  const auto& right_column_id = ColumnID{1};

  const auto& storage_manager = StorageManager::get();

  const auto& left_table = storage_manager.get_table(left_table_name);
  const auto& left_table_wrapper = std::make_shared<TableWrapper>(left_table);
  left_table_wrapper->execute();
#ifdef DEBUG
  print_table_row_count(left_table_name);
#endif
  const auto& right_table = storage_manager.get_table(right_table_name);
  const auto& right_table_wrapper = std::make_shared<TableWrapper>(right_table);
  right_table_wrapper->execute();
#ifdef DEBUG
  print_table_row_count(right_table_name);
#endif
  for (auto _ : state) {
    const auto hash_join = std::make_shared<JoinHash>(
        left_table_wrapper, right_table_wrapper, JoinMode::Inner,
        OperatorJoinPredicate{{left_column_id, right_column_id}, PredicateCondition::Equals});
    hash_join->execute();
#ifdef DEBUG
    std::cout << "join result table: " << hash_join->get_output()->row_count() << " rows" << std::endl;
#endif
  }
}

BENCHMARK_F(TPCHDataMicroBenchmarkFixture, BM_TPCH_pPartkey_lPartkey_groupKeyIndex_join)
(benchmark::State& state) {
  const auto& left_table_name = "part";
  const auto& right_table_name = "lineitem";
  const auto& left_column_id = ColumnID{0};
  const auto& right_column_id = ColumnID{1};

  const auto& storage_manager = StorageManager::get();

  const auto& left_table = storage_manager.get_table(left_table_name);
  left_table->create_index<GroupKeyIndex>({left_column_id});
  const auto& left_table_wrapper = std::make_shared<TableWrapper>(left_table);
  left_table_wrapper->execute();
#ifdef DEBUG
  print_table_row_count(left_table_name);
#endif
  const auto& right_table = storage_manager.get_table(right_table_name);
  right_table->create_index<GroupKeyIndex>({right_column_id});
  const auto& right_table_wrapper = std::make_shared<TableWrapper>(right_table);
  right_table_wrapper->execute();
#ifdef DEBUG
  print_table_row_count(right_table_name);
#endif

  for (auto _ : state) {
    const auto index_join = std::make_shared<JoinIndex>(
        left_table_wrapper, right_table_wrapper, JoinMode::Inner,
        OperatorJoinPredicate{{left_column_id, right_column_id}, PredicateCondition::Equals});
    index_join->execute();
#ifdef DEBUG
    std::cout << "join result table: " << index_join->get_output()->row_count() << " rows" << std::endl;
#endif
  }
}

BENCHMARK_F(TPCHDataMicroBenchmarkFixture, BM_TPCH_pPartkey_lPartkey_bTreeIndex_join)
(benchmark::State& state) {
  const auto& left_table_name = "part";
  const auto& right_table_name = "lineitem";
  const auto& left_column_id = ColumnID{0};
  const auto& right_column_id = ColumnID{1};

  const auto& storage_manager = StorageManager::get();

  const auto& left_table = storage_manager.get_table(left_table_name);
  left_table->create_index<BTreeIndex>({left_column_id});
  const auto& left_table_wrapper = std::make_shared<TableWrapper>(left_table);
  left_table_wrapper->execute();
#ifdef DEBUG
  print_table_row_count(left_table_name);
#endif
  const auto& right_table = storage_manager.get_table(right_table_name);
  right_table->create_index<BTreeIndex>({right_column_id});
  const auto& right_table_wrapper = std::make_shared<TableWrapper>(right_table);
  right_table_wrapper->execute();
  print_table_row_count(right_table_name);

  for (auto _ : state) {
    const auto index_join = std::make_shared<JoinIndex>(
        left_table_wrapper, right_table_wrapper, JoinMode::Inner,
        OperatorJoinPredicate{{left_column_id, right_column_id}, PredicateCondition::Equals});
    index_join->execute();
#ifdef DEBUG
    std::cout << "join result table: " << index_join->get_output()->row_count() << " rows" << std::endl;
#endif
  }
}

BENCHMARK_F(TPCHDataMicroBenchmarkFixture, BM_TPCH_Q14_hash_join)(benchmark::State& state) {
  // SELECT 100.00 * SUM(case when p_type like 'PROMO%' then l_extendedprice*(1.0-l_discount) else 0 end) /
  //   SUM(l_extendedprice * (1.0 - l_discount)) as promo_revenue
  // FROM lineitem, "part"
  // WHERE l_partkey = p_partkey
  // AND l_shipdate >= '1995-09-01'
  // AND l_shipdate < '1995-10-01';
  const auto& p_partkey_column_id = ColumnID{0};
  const auto& l_partkey_column_id = ColumnID{1};
  const auto& l_shipdate_column_id = ColumnID{10};
  const std::vector<ColumnID>& part_index_column_ids = {p_partkey_column_id};

  const auto& storage_manager = StorageManager::get();

  const auto& part_table = storage_manager.get_table("part");
  const auto& part_table_wrapper = std::make_shared<TableWrapper>(part_table);
  part_table_wrapper->execute();
#ifdef DEBUG
  print_table_row_count("part");
#endif
  const auto& lineitem_table = storage_manager.get_table("lineitem");
  const auto& lineitem_table_wrapper = std::make_shared<TableWrapper>(lineitem_table);
  lineitem_table_wrapper->execute();
#ifdef DEBUG
  print_table_row_count("lineitem");
#endif

  // index scan shipdate >= '1995-09-01'
  const auto& shipdate_pqp_column = pqp_column_(l_shipdate_column_id, lineitem_table->column_data_type(ColumnID{10}),
                                                lineitem_table->column_is_nullable(ColumnID{10}), "");
  const auto& shipdate_gte_predicate = std::make_shared<BinaryPredicateExpression>(
      PredicateCondition::GreaterThanEquals, shipdate_pqp_column, value_("1995-09-01"));

  const auto& table_scan_shipdate_gte = std::make_shared<TableScan>(lineitem_table_wrapper, shipdate_gte_predicate);
  table_scan_shipdate_gte->execute();

  const auto& table_scanned_gte = std::const_pointer_cast<Table>(table_scan_shipdate_gte->get_output());
#ifdef DEBUG
  std::cout << "intermediate table 1: " << table_scanned_gte->row_count() << " rows" << std::endl;
#endif
  const auto& table_scanned_gte_wrapper = std::make_shared<TableWrapper>(table_scanned_gte);
  table_scanned_gte_wrapper->execute();

  // table scan shipdate < '1995-10-01'
  const auto& lineitem_scanned_shipdate_gte_pqp_column =
      pqp_column_(l_shipdate_column_id, table_scanned_gte->column_data_type(ColumnID{10}),
                  table_scanned_gte->column_is_nullable(ColumnID{10}), "");
  const auto& shipdate_lt_predicate = std::make_shared<BinaryPredicateExpression>(
      PredicateCondition::LessThan, lineitem_scanned_shipdate_gte_pqp_column, value_("1995-10-01"));

  const auto& table_scan_shipdate_lt = std::make_shared<TableScan>(table_scanned_gte_wrapper, shipdate_lt_predicate);
  table_scan_shipdate_lt->execute();

  const auto& table_scanned_le = std::const_pointer_cast<Table>(table_scan_shipdate_lt->get_output());
#ifdef DEBUG
  std::cout << "intermediate table 2: " << table_scanned_le->row_count() << " rows" << std::endl;
#endif
  const auto& table_scanned_le_wrapper = std::make_shared<TableWrapper>(table_scanned_le);
  table_scanned_le_wrapper->execute();

  // hash join

  for (auto _ : state) {
    const auto hash_join = std::make_shared<JoinHash>(
        table_scanned_le_wrapper, part_table_wrapper, JoinMode::Inner,
        OperatorJoinPredicate{{l_partkey_column_id, p_partkey_column_id}, PredicateCondition::Equals});
    hash_join->execute();
#ifdef DEBUG
    std::cout << "join result table: " << hash_join->get_output()->row_count() << " rows" << std::endl;
#endif
  }
}

BENCHMARK_F(TPCHDataMicroBenchmarkFixture, BM_TPCH_Q14_index_join)(benchmark::State& state) {
  // SELECT 100.00 * SUM(case when p_type like 'PROMO%' then l_extendedprice*(1.0-l_discount) else 0 end) /
  //   SUM(l_extendedprice * (1.0 - l_discount)) as promo_revenue
  // FROM lineitem, "part"
  // WHERE l_partkey = p_partkey
  // AND l_shipdate >= '1995-09-01'
  // AND l_shipdate < '1995-10-01';
  const auto& p_partkey_column_id = ColumnID{0};
  const auto& l_partkey_column_id = ColumnID{1};
  const auto& l_shipdate_column_id = ColumnID{10};
  const std::vector<ColumnID>& part_index_column_ids = {p_partkey_column_id};

  const auto& storage_manager = StorageManager::get();

  const auto& part_table = storage_manager.get_table("part");
  part_table->create_index<GroupKeyIndex>(part_index_column_ids);
  const auto& part_table_wrapper = std::make_shared<TableWrapper>(part_table);
  part_table_wrapper->execute();
#ifdef DEBUG
  print_table_row_count("part");
#endif
  const auto& lineitem_table = storage_manager.get_table("lineitem");
  const auto& lineitem_table_wrapper = std::make_shared<TableWrapper>(lineitem_table);
  lineitem_table_wrapper->execute();
#ifdef DEBUG
  print_table_row_count("lineitem");
#endif

  // index scan shipdate >= '1995-09-01'
  const auto& shipdate_pqp_column = pqp_column_(l_shipdate_column_id, lineitem_table->column_data_type(ColumnID{10}),
                                                lineitem_table->column_is_nullable(ColumnID{10}), "");
  const auto& shipdate_gte_predicate = std::make_shared<BinaryPredicateExpression>(
      PredicateCondition::GreaterThanEquals, shipdate_pqp_column, value_("1995-09-01"));

  const auto& table_scan_shipdate_gte = std::make_shared<TableScan>(lineitem_table_wrapper, shipdate_gte_predicate);
  table_scan_shipdate_gte->execute();

  const auto& table_scanned_gte = std::const_pointer_cast<Table>(table_scan_shipdate_gte->get_output());
#ifdef DEBUG
  std::cout << "intermediate table 1: " << table_scanned_gte->row_count() << " rows" << std::endl;
#endif
  const auto& table_scanned_gte_wrapper = std::make_shared<TableWrapper>(table_scanned_gte);
  table_scanned_gte_wrapper->execute();

  // table scan shipdate < '1995-10-01'
  const auto& lineitem_scanned_shipdate_gte_pqp_column =
      pqp_column_(l_shipdate_column_id, table_scanned_gte->column_data_type(ColumnID{10}),
                  table_scanned_gte->column_is_nullable(ColumnID{10}), "");
  const auto& shipdate_lt_predicate = std::make_shared<BinaryPredicateExpression>(
      PredicateCondition::LessThan, lineitem_scanned_shipdate_gte_pqp_column, value_("1995-10-01"));

  const auto& table_scan_shipdate_lt = std::make_shared<TableScan>(table_scanned_gte_wrapper, shipdate_lt_predicate);
  table_scan_shipdate_lt->execute();

  const auto& table_scanned_le = std::const_pointer_cast<Table>(table_scan_shipdate_lt->get_output());
#ifdef DEBUG
  std::cout << "intermediate table 2: " << table_scanned_le->row_count() << " rows" << std::endl;
#endif
  const auto& table_scanned_le_wrapper = std::make_shared<TableWrapper>(table_scanned_le);
  table_scanned_le_wrapper->execute();

  // hash join

  for (auto _ : state) {
    const auto index_join = std::make_shared<JoinIndex>(
        table_scanned_le_wrapper, part_table_wrapper, JoinMode::Inner,
        OperatorJoinPredicate{{l_partkey_column_id, p_partkey_column_id}, PredicateCondition::Equals});
    index_join->execute();
#ifdef DEBUG
    std::cout << "join result table: " << index_join->get_output()->row_count() << " rows" << std::endl;
#endif
  }
}

BENCHMARK_F(TPCHDataMicroBenchmarkFixture, BM_TPCHQ6FirstScanPredicate)(benchmark::State& state) {
  for (auto _ : state) {
    const auto table_scan = std::make_shared<TableScan>(_table_wrapper_map.at("lineitem"), _tpchq6_discount_predicate);
    table_scan->execute();
  }
}

BENCHMARK_F(TPCHDataMicroBenchmarkFixture, BM_TPCHQ6SecondScanPredicate)(benchmark::State& state) {
  const auto first_scan = std::make_shared<TableScan>(_table_wrapper_map.at("lineitem"), _tpchq6_discount_predicate);
  first_scan->execute();

  for (auto _ : state) {
    const auto table_scan = std::make_shared<TableScan>(first_scan, _tpchq6_shipdate_less_predicate);
    table_scan->execute();
  }
}

BENCHMARK_F(TPCHDataMicroBenchmarkFixture, BM_TPCHQ6ThirdScanPredicate)(benchmark::State& state) {
  const auto first_scan = std::make_shared<TableScan>(_table_wrapper_map.at("lineitem"), _tpchq6_discount_predicate);
  first_scan->execute();
  const auto first_scan_result = first_scan->get_output();
  const auto second_scan = std::make_shared<TableScan>(first_scan, _tpchq6_shipdate_less_predicate);
  second_scan->execute();

  for (auto _ : state) {
    const auto table_scan = std::make_shared<TableScan>(second_scan, _tpchq6_quantity_predicate);
    table_scan->execute();
  }
}

BENCHMARK_F(TPCHDataMicroBenchmarkFixture, BM_TableScanIntegerOnPhysicalTable)(benchmark::State& state) {
  for (auto _ : state) {
    const auto table_scan = std::make_shared<TableScan>(_table_wrapper_map.at("lineitem"), _int_predicate);
    table_scan->execute();
  }
}

BENCHMARK_F(TPCHDataMicroBenchmarkFixture, BM_TableScanIntegerOnReferenceTable)(benchmark::State& state) {
  const auto table_scan = std::make_shared<TableScan>(_table_wrapper_map.at("lineitem"), _int_predicate);
  table_scan->execute();
  const auto scanned_table = table_scan->get_output();

  for (auto _ : state) {
    auto reference_table_scan = std::make_shared<TableScan>(table_scan, _int_predicate);
    reference_table_scan->execute();
  }
}

BENCHMARK_F(TPCHDataMicroBenchmarkFixture, BM_TableScanStringOnPhysicalTable)(benchmark::State& state) {
  for (auto _ : state) {
    const auto table_scan = std::make_shared<TableScan>(_table_wrapper_map.at("lineitem"), _string_predicate);
    table_scan->execute();
  }
}

BENCHMARK_F(TPCHDataMicroBenchmarkFixture, BM_TableScanStringOnReferenceTable)(benchmark::State& state) {
  const auto table_scan = std::make_shared<TableScan>(_table_wrapper_map.at("lineitem"), _string_predicate);
  table_scan->execute();
  const auto scanned_table = table_scan->get_output();

  for (auto _ : state) {
    auto reference_table_scan = std::make_shared<TableScan>(table_scan, _int_predicate);
    reference_table_scan->execute();
  }
}

/** TPC-H Q4 Benchmarks:
  - the following two benchmarks use a static and slightly simplified TPC-H Query 4
  - objective is to compare the performance of unnesting the EXISTS subquery

  - The LQPs translate roughly to this query:
      SELECT
         o_orderpriority
      FROM orders
      WHERE
         o_orderdate >= date '1993-07-01'
         AND o_orderdate < date '1993-10-01'
         AND exists (
             SELECT *
             FROM lineitem
             WHERE
                 l_orderkey = o_orderkey
                 AND l_commitdate < l_receiptdate
             )
 */
BENCHMARK_F(TPCHDataMicroBenchmarkFixture, BM_TPCHQ4WithExistsSubquery)(benchmark::State& state) {
  // clang-format off
        const auto parameter = correlated_parameter_(ParameterID{0}, _orders_orderkey);
        const auto subquery_lqp = PredicateNode::make(equals_(parameter, _lineitem_orderkey),
                                                      PredicateNode::make(
                                                              less_than_(_lineitem_commitdate, _lineitem_receiptdate),
                                                              _lineitem_table_node));
        const auto subquery = lqp_subquery_(subquery_lqp, std::make_pair(ParameterID{0}, _orders_orderkey));

        const auto lqp =
                ProjectionNode::make(expression_vector(_orders_orderpriority),
                                     PredicateNode::make(equals_(exists_(subquery), 1),
                                                         PredicateNode::make(
                                                                 greater_than_equals_(_orders_orderdate, "1993-07-01"),
                                                                 PredicateNode::make(
                                                                         less_than_(_orders_orderdate, "1993-10-01"),
                                                                         _orders_table_node))));
  // clang-format on

  for (auto _ : state) {
    const auto pqp = LQPTranslator{}.translate_node(lqp);
    const auto tasks = OperatorTask::make_tasks_from_operator(pqp, CleanupTemporaries::Yes);
    CurrentScheduler::schedule_and_wait_for_tasks(tasks);
  }
}

BENCHMARK_F(TPCHDataMicroBenchmarkFixture, BM_TPCHQ4WithUnnestedSemiJoin)(benchmark::State& state) {
  // clang-format off
        const auto lqp =
                ProjectionNode::make(expression_vector(_orders_orderpriority),
                                     JoinNode::make(JoinMode::Semi, equals_(_lineitem_orderkey, _orders_orderkey),
                                                    PredicateNode::make(
                                                            greater_than_equals_(_orders_orderdate, "1993-07-01"),
                                                            PredicateNode::make(
                                                                    less_than_(_orders_orderdate, "1993-10-01"),
                                                                    _orders_table_node)),
                                                    PredicateNode::make(
                                                            less_than_(_lineitem_commitdate, _lineitem_receiptdate),
                                                            _lineitem_table_node)));
  // clang-format on

  for (auto _ : state) {
    const auto pqp = LQPTranslator{}.translate_node(lqp);
    const auto tasks = OperatorTask::make_tasks_from_operator(pqp, CleanupTemporaries::Yes);
    CurrentScheduler::schedule_and_wait_for_tasks(tasks);
  }
}

/**
 * For semi joins, the semi relation (which is filtered and returned in a semi join) is passed as the left input and
 * the other relation (which is solely checked for value existence and then discarded) is passed as the right side.
 *
 * For hash-based semi joins, inputs are switched as the left relation can probe the (later discarded) right relation.
 * In case the left relation is significantly smaller, the hash join does not perform optimally due to the switching.
 */
BENCHMARK_F(TPCHDataMicroBenchmarkFixture, BM_HashSemiProbeRelationSmaller)(benchmark::State& state) {
  for (auto _ : state) {
    auto join = std::make_shared<JoinHash>(
        _table_wrapper_map.at("orders"), _table_wrapper_map.at("lineitem"), JoinMode::Semi,
        OperatorJoinPredicate{ColumnIDPair(ColumnID{0}, ColumnID{0}), PredicateCondition::Equals});
    join->execute();
  }
}

BENCHMARK_F(TPCHDataMicroBenchmarkFixture, BM_HashSemiProbeRelationLarger)(benchmark::State& state) {
  for (auto _ : state) {
    auto join = std::make_shared<JoinHash>(
        _table_wrapper_map.at("lineitem"), _table_wrapper_map.at("orders"), JoinMode::Semi,
        OperatorJoinPredicate{ColumnIDPair(ColumnID{0}, ColumnID{0}), PredicateCondition::Equals});
    join->execute();
  }
}

BENCHMARK_F(TPCHDataMicroBenchmarkFixture, BM_SortMergeSemiProbeRelationSmaller)(benchmark::State& state) {
  for (auto _ : state) {
    auto join = std::make_shared<JoinSortMerge>(
        _table_wrapper_map.at("orders"), _table_wrapper_map.at("lineitem"), JoinMode::Semi,
        OperatorJoinPredicate{ColumnIDPair(ColumnID{0}, ColumnID{0}), PredicateCondition::Equals});
    join->execute();
  }
}

BENCHMARK_F(TPCHDataMicroBenchmarkFixture, BM_SortMergeSemiProbeRelationLarger)(benchmark::State& state) {
  for (auto _ : state) {
    auto join = std::make_shared<JoinSortMerge>(
        _table_wrapper_map.at("lineitem"), _table_wrapper_map.at("orders"), JoinMode::Semi,
        OperatorJoinPredicate{ColumnIDPair(ColumnID{0}, ColumnID{0}), PredicateCondition::Equals});
    join->execute();
  }
}

}  // namespace opossum
