### Performance analysis of scan and join operators with index structures

- `BM_TPCH_Q6_lineitem_shipdate_less_predicate_table_scan`  
uses a `TableScan` operator to scan TPC-H table `lineitem` with predicate `l_shipdate < '1995-01-01'`
- `BM_TPCH_Q6_lineitem_shipdate_less_predicate_groupKeyIndex_scan`  
uses an `IndexScan` operator to scan TPC-H table `lineitem` with predicate `l_shipdate < '1995-01-01'`.  
Only indices of type `GroupKeyIndex` exist for column `l_shipdate` of table `lineitem`.
- `BM_TPCH_Q6_lineitem_shipdate_less_predicate_bTreeIndex_scan`  
uses an `IndexScan` operator to scan TPC-H table `lineitem` with predicate `l_shipdate < '1995-01-01'`.  
Only indices of type `BTreeIndex` exist for column `l_shipdate` of table `lineitem`.
- `BM_TPCH_Q3_orders_orderdate_predicate_table_scan`  
uses a `TableScan` operator to scan TPC-H table `orders` with predicate `o_orderdate < '1995-03-15'`
- `BM_TPCH_Q3_orders_orderdate_predicate_groupKeyIndex_scan`  
uses an `IndexScan` operator to scan TPC-H table `orders` with predicate `o_orderdate < '1995-03-15'`.  
Only indices of type `GroupKeyIndex` exist for column `o_orderdate` of table `orders`.
- `BM_TPCH_Q3_orders_orderdate_predicate_bTreeIndex_scan`  
uses an `IndexScan` operator to scan TPC-H table `orders` with predicate `o_orderdate < '1995-03-15'`.  
Only indices of type `BTreeIndex` exist for column `o_orderdate` of table `orders`.
- `BM_TPCH_Q14_part_partkey_predicate_hash_join`  
uses a `JoinHash` operator to join tables `part` and `lineitem` with join predicate `l_partkey = p_partkey`.
- `BM_TPCH_Q14_part_partkey_predicate_nestedLoop_join`  
uses a `JoinNestedLoop` operator to join tables `part` and `lineitem` with join predicate `l_partkey = p_partkey`.
- `BM_TPCH_Q14_part_partkey_predicate_groupKeyIndex_join`  
uses a `JoinIndex` operator to join tables `part` and `lineitem` with join predicate `l_partkey = p_partkey`.  
Only indices of type `GroupKeyIndex` exist for column `l_partkey` of table `lineitem` and for column `p_partkey` of table `part`.
- `BM_TPCH_Q14_part_partkey_predicate_bTreeIndex_join`  
uses a `JoinIndex` operator to join tables `part` and `lineitem` with join predicate `l_partkey = p_partkey`.  
Only indices of type `BTreeIndex` exist for column `l_partkey` of table `lineitem` and for column `p_partkey` of table `part`.

#### Used Flags
```
-DCMAKE_CXX_COMPILER_LAUNCHER=ccache
-DENABLE_JIT_SUPPORT=OFF
-DENABLE_NUMA_SUPPORT=OFF
-DCMAKE_BUILD_TYPE=Release
```

#### Benchmark Results
```
Running ./hyriseMicroBenchmarks
Run on (32 X 2500 MHz CPU s)
CPU Caches:
  L1 Data 32K (x32)
  L1 Instruction 32K (x32)
  L2 Unified 256K (x32)
  L3 Unified 25600K (x32)
Generating TPC-H data set with scale factor 1 and Dictionary encoding:
- Loading/Generating tables
- Loading/Generating tables done (18 s 266 ms)
- Encoding tables if necessary
-  Encoding 'nation' - encoding applied (1 ms 168 µs)
-  Encoding 'orders' - encoding applied (707 ms 695 µs)
-  Encoding 'region' - encoding applied (607 µs 13 ns)
-  Encoding 'part' - encoding applied (479 ms 641 µs)
-  Encoding 'lineitem' - encoding applied (1 s 728 ms)
-  Encoding 'partsupp' - encoding applied (422 ms 543 µs)
-  Encoding 'customer' - encoding applied (629 ms 358 µs)
-  Encoding 'supplier' - encoding applied (55 ms 146 µs)
- Encoding tables done (4 s 25 ms)
- Adding Tables to StorageManager and generating statistics
-  Adding 'nation' (321 µs 261 ns)
-  Adding 'orders' (2 s 561 ms)
-  Adding 'region' (32 µs 279 ns)
-  Adding 'part' (314 ms 227 µs)
-  Adding 'lineitem' (10 s 752 ms)
-  Adding 'partsupp' (1 s 378 ms)
-  Adding 'customer' (368 ms 812 µs)
-  Adding 'supplier' (18 ms 901 µs)
- Adding Tables to StorageManager and generating statistics done (15 s 395 ms)
------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                                             Time           CPU Iterations
------------------------------------------------------------------------------------------------------------------------------------
TPCHDataMicroBenchmarkFixture/BM_TPCH_Q6_lineitem_shipdate_less_predicate_table_scan           17921007 ns   17920684 ns         40
TPCHDataMicroBenchmarkFixture/BM_TPCH_Q6_lineitem_shipdate_less_predicate_groupKeyIndex_scan    8967938 ns    8967846 ns         81
TPCHDataMicroBenchmarkFixture/BM_TPCH_Q6_lineitem_shipdate_less_predicate_bTreeIndex_scan       8873764 ns    8872874 ns         80
TPCHDataMicroBenchmarkFixture/BM_TPCH_Q3_orders_orderdate_predicate_table_scan                  7567452 ns    7567191 ns         92
TPCHDataMicroBenchmarkFixture/BM_TPCH_Q3_orders_orderdate_predicate_groupKeyIndex_scan          2117514 ns    2117486 ns        358
TPCHDataMicroBenchmarkFixture/BM_TPCH_Q3_orders_orderdate_predicate_bTreeIndex_scan             1970570 ns    1970386 ns        355
TPCHDataMicroBenchmarkFixture/BM_TPCH_Q14_part_partkey_predicate_hash_join                    386531472 ns  386519878 ns          2
[PERF] Nested Loop Join used at src/lib/operators/join_nested_loop.cpp:82
        Performance can be affected. This warning is only shown once.

TPCHDataMicroBenchmarkFixture/BM_TPCH_Q14_part_partkey_predicate_nestedLoop_join             2161427643538 ns 2161403285121 ns          1
TPCHDataMicroBenchmarkFixture/BM_TPCH_Q14_part_partkey_predicate_groupKeyIndex_join          1627717495 ns 1627690885 ns          1
TPCHDataMicroBenchmarkFixture/BM_TPCH_Q14_part_partkey_predicate_bTreeIndex_join             1732551098 ns 1732541397 ns          1

```
