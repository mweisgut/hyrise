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

```
2019-04-05 12:13:22
Running ./hyriseMicroBenchmarks
Run on (32 X 2500 MHz CPU s)
CPU Caches:
  L1 Data 32K (x32)
  L1 Instruction 32K (x32)
  L2 Unified 256K (x32)
  L3 Unified 25600K (x32)
Generating TPC-H data set with scale factor 1 and Dictionary encoding:
- Loading/Generating tables
- Loading/Generating tables done (16 s 653 ms)
- Encoding tables if necessary
-  Encoding 'nation' - encoding applied (1 ms 140 µs)
-  Encoding 'orders' - encoding applied (509 ms 749 µs)
-  Encoding 'region' - encoding applied (506 µs 328 ns)
-  Encoding 'part' - encoding applied (447 ms 337 µs)
-  Encoding 'lineitem' - encoding applied (1 s 470 ms)
-  Encoding 'partsupp' - encoding applied (384 ms 550 µs)
-  Encoding 'customer' - encoding applied (536 ms 807 µs)
-  Encoding 'supplier' - encoding applied (41 ms 357 µs)
- Encoding tables done (3 s 392 ms)
- Adding Tables to StorageManager and generating statistics
-  Adding 'nation' (176 µs 350 ns)
-  Adding 'orders' (2 s 306 ms)
-  Adding 'region' (88 µs 533 ns)
-  Adding 'part' (302 ms 375 µs)
-  Adding 'lineitem' (10 s 751 ms)
-  Adding 'partsupp' (1 s 331 ms)
-  Adding 'customer' (426 ms 387 µs)
-  Adding 'supplier' (16 ms 842 µs)
- Adding Tables to StorageManager and generating statistics done (15 s 135 ms)
------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                                             Time           CPU Iterations
------------------------------------------------------------------------------------------------------------------------------------
TPCHDataMicroBenchmarkFixture/BM_TPCH_Q6_lineitem_shipdate_less_predicate_table_scan           18117844 ns   18117423 ns         39
TPCHDataMicroBenchmarkFixture/BM_TPCH_Q6_lineitem_shipdate_less_predicate_groupKeyIndex_scan    8617421 ns    8617201 ns         84
TPCHDataMicroBenchmarkFixture/BM_TPCH_Q6_lineitem_shipdate_less_predicate_bTreeIndex_scan       8512328 ns    8512135 ns         82
TPCHDataMicroBenchmarkFixture/BM_TPCH_Q3_orders_orderdate_predicate_table_scan                  7536569 ns    7536306 ns         94
TPCHDataMicroBenchmarkFixture/BM_TPCH_Q3_orders_orderdate_predicate_groupKeyIndex_scan          1957065 ns    1956986 ns        355
TPCHDataMicroBenchmarkFixture/BM_TPCH_Q3_orders_orderdate_predicate_bTreeIndex_scan             1961092 ns    1960984 ns        357
lineitem row count: 6001215
part row count: 200000
lineitem row count: 6001215
part row count: 200000
TPCHDataMicroBenchmarkFixture/BM_TPCH_Q14_part_partkey_predicate_hash_join                    388650537 ns  388651633 ns          2
lineitem row count: 6001215
part row count: 200000
TPCHDataMicroBenchmarkFixture/BM_TPCH_Q14_part_partkey_predicate_groupKeyIndex_join          1640895367 ns 1640861942 ns          1
lineitem row count: 6001215
part row count: 200000
TPCHDataMicroBenchmarkFixture/BM_TPCH_Q14_part_partkey_predicate_bTreeIndex_join             1720819950 ns 1720719245 ns          1
```


##### Join: Q14

- left input: filtered lineitem table (`l_shipdate >= '1995-09-01' AND l_shipdate < '1995-10-01'`; 75'983 rows)
- right input: part table (200'000 rows)
- result table: 75'983 rows (each lineitem of the filtered lineitem table qualifies)  

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
- Loading/Generating tables done (17 s 764 ms)
- Encoding tables if necessary
-  Encoding 'nation' - encoding applied (777 µs 462 ns)
-  Encoding 'orders' - encoding applied (615 ms 522 µs)
-  Encoding 'region' - encoding applied (689 µs 457 ns)
-  Encoding 'part' - encoding applied (444 ms 589 µs)
-  Encoding 'lineitem' - encoding applied (1 s 568 ms)
-  Encoding 'partsupp' - encoding applied (426 ms 240 µs)
-  Encoding 'customer' - encoding applied (575 ms 63 µs)
-  Encoding 'supplier' - encoding applied (47 ms 235 µs)
- Encoding tables done (3 s 680 ms)
- Adding Tables to StorageManager and generating statistics
-  Adding 'nation' (137 µs 293 ns)
-  Adding 'orders' (2 s 244 ms)
-  Adding 'region' (47 µs 345 ns)
-  Adding 'part' (273 ms 300 µs)
-  Adding 'lineitem' (10 s 269 ms)
-  Adding 'partsupp' (1 s 173 ms)
-  Adding 'customer' (296 ms 538 µs)
-  Adding 'supplier' (15 ms 881 µs)
- Adding Tables to StorageManager and generating statistics done (14 s 272 ms)
intermediate table 1: 2817779 rows
intermediate table 2: 75983 rows
intermediate table 1: 2817779 rows
intermediate table 2: 75983 rows
intermediate table 1: 2817779 rows
intermediate table 2: 75983 rows
--------------------------------------------------------------------------------------------
Benchmark                                                     Time           CPU Iterations
--------------------------------------------------------------------------------------------
TPCHDataMicroBenchmarkFixture/BM_TPCH_Q14_hash_join    13966819 ns   13966645 ns         48
TPCHDataMicroBenchmarkFixture/BM_TPCH_Q14_index_join   59771037 ns   59771111 ns         10
  ```

#### Join: reduced PART with LINEITEM
In this benchmark, first table PART is reduced by a scan. This leads to an intermediate result table with only 3 rows (filter predicate: `p_partkey <= 3`). Let this intermediate table be `REDUCED_PART`, `REDUCED_PART` is then joined with table `LINEITEM` using the join predicate `p_partkey = l_partkey`.

The result below shows that the JoinIndex is a factor of **954,13** faster than the JoinHash.

- reduced part table (left join input): 3 rows
- lineitem table (right join input): 6'001'215 rows
- join result table: 99 rows

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
- Loading/Generating tables done (17 s 483 ms)
- Encoding tables if necessary
-  Encoding 'nation' - encoding applied (1 ms 345 µs)
-  Encoding 'orders' - encoding applied (603 ms 890 µs)
-  Encoding 'region' - encoding applied (742 µs 394 ns)
-  Encoding 'part' - encoding applied (472 ms 91 µs)
-  Encoding 'lineitem' - encoding applied (1 s 550 ms)
-  Encoding 'partsupp' - encoding applied (509 ms 703 µs)
-  Encoding 'customer' - encoding applied (565 ms 801 µs)
-  Encoding 'supplier' - encoding applied (37 ms 627 µs)
- Encoding tables done (3 s 742 ms)
- Adding Tables to StorageManager and generating statistics
-  Adding 'nation' (403 µs 63 ns)
-  Adding 'orders' (2 s 222 ms)
-  Adding 'region' (32 µs 468 ns)
-  Adding 'part' (258 ms 747 µs)
-  Adding 'lineitem' (9 s 756 ms)
-  Adding 'partsupp' (1 s 430 ms)
-  Adding 'customer' (309 ms 674 µs)
-  Adding 'supplier' (19 ms 227 µs)
- Adding Tables to StorageManager and generating statistics done (13 s 997 ms)
--------------------------------------------------------------------------------------------------------------------
Benchmark                                                                             Time           CPU Iterations
--------------------------------------------------------------------------------------------------------------------
TPCHDataMicroBenchmarkFixture/BM_TPCH_filtered_part_with_lineitem_hash_join   117486954 ns  117485828 ns          6
TPCHDataMicroBenchmarkFixture/BM_TPCH_filtered_part_with_lineitem_index_join     123135 ns     123134 ns       5895
```

#### Join: reduced PART with reduced LINEITEM
In this benchmark, first table PART is reduced by a scan.
This leads to an intermediate result table with only 3 rows (filter predicate: `p_partkey <= 3`).
Let this intermediate table be `REDUCED_PART`.
Additionally, let `REDUCED_LINEITEM` be a reduced LINEITEM table.
It only contains tuples of the original table where `l_shipdate >= '1995-09-01' AND l_shipdate < '1995-10-01'`.
`REDUCED_PART` is then joined with table `REDUCED_LINEITEM` using the join predicate `p_partkey = l_partkey`.
`REDUCED_LINEITEM` has indices for column `l_shipdate`.

The result below shows that the JoinIndex is a factor of **95,738** faster than the JoinHash.

- reduced part table (left join input): 3 rows
- reduced lineitem table (right join input): 75'983 rows
- join result table: 1 row

```
--------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                                   Time           CPU Iterations
--------------------------------------------------------------------------------------------------------------------------
TPCHDataMicroBenchmarkFixture/BM_TPCH_reduced_part_and_reduced_lineitem_hash_join     1009670 ns    1009656 ns        680
TPCHDataMicroBenchmarkFixture/BM_TPCH_reduced_part_and_reduced_lineitem_index_join      10546 ns      10546 ns      67742
```

#### Scan: `LINEITEM` | `l_orderkey = 1`
- lineitem table: 6'001'215 rows
- result table: 6 rows
-  (table scan CPU time) / (group index scan CPU time) = **0,5596**

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
- Loading/Generating tables done (15 s 339 ms)
- Encoding tables if necessary
-  Encoding 'nation' - encoding applied (757 µs 352 ns)
-  Encoding 'orders' - encoding applied (442 ms 625 µs)
-  Encoding 'region' - encoding applied (545 µs 130 ns)
-  Encoding 'part' - encoding applied (492 ms 386 µs)
-  Encoding 'lineitem' - encoding applied (1 s 404 ms)
-  Encoding 'partsupp' - encoding applied (384 ms 389 µs)
-  Encoding 'customer' - encoding applied (485 ms 491 µs)
-  Encoding 'supplier' - encoding applied (45 ms 64 µs)
- Encoding tables done (3 s 256 ms)
- Adding Tables to StorageManager and generating statistics
-  Adding 'nation' (134 µs 536 ns)
-  Adding 'orders' (1 s 936 ms)
-  Adding 'region' (28 µs 34 ns)
-  Adding 'part' (257 ms 231 µs)
-  Adding 'lineitem' (9 s 283 ms)
-  Adding 'partsupp' (1 s 246 ms)
-  Adding 'customer' (301 ms 171 µs)
-  Adding 'supplier' (18 ms 462 µs)
- Adding Tables to StorageManager and generating statistics done (13 s 42 ms)
-------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                                              Time           CPU Iterations
-------------------------------------------------------------------------------------------------------------------------------------
TPCHDataMicroBenchmarkFixture/BM_TPCH_lineitem_filter_by_specific_orderkey_table_scan             100416 ns     100410 ns       7031
TPCHDataMicroBenchmarkFixture/BM_TPCH_lineitem_filter_by_specific_orderkey_groupKeyIndex_scan     179435 ns     179418 ns       3877
TPCHDataMicroBenchmarkFixture/BM_TPCH_lineitem_filter_by_specific_orderkey_bTreeIndex_scan        180185 ns     180175 ns       4020
```

#### Scan: `LINEITEM` | `l_shipdate = '1995-06-10'`
- lineitem table: 6'001'215 rows
- result table: 2384 rows
- (table scan CPU time) / (group index scan CPU time) = **12,3798**

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
- Loading/Generating tables done (15 s 344 ms)
- Encoding tables if necessary
-  Encoding 'nation' - encoding applied (1 ms 141 µs)
-  Encoding 'orders' - encoding applied (455 ms 439 µs)
-  Encoding 'region' - encoding applied (387 µs 206 ns)
-  Encoding 'part' - encoding applied (426 ms 425 µs)
-  Encoding 'lineitem' - encoding applied (1 s 440 ms)
-  Encoding 'partsupp' - encoding applied (387 ms 45 µs)
-  Encoding 'customer' - encoding applied (567 ms 904 µs)
-  Encoding 'supplier' - encoding applied (38 ms 386 µs)
- Encoding tables done (3 s 318 ms)
- Adding Tables to StorageManager and generating statistics
-  Adding 'nation' (215 µs 911 ns)
-  Adding 'orders' (2 s 22 ms)
-  Adding 'region' (31 µs 304 ns)
-  Adding 'part' (250 ms 877 µs)
-  Adding 'lineitem' (9 s 946 ms)
-  Adding 'partsupp' (1 s 176 ms)
-  Adding 'customer' (390 ms 606 µs)
-  Adding 'supplier' (17 ms 753 µs)
- Adding Tables to StorageManager and generating statistics done (13 s 804 ms)
-------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                                              Time           CPU Iterations
-------------------------------------------------------------------------------------------------------------------------------------
TPCHDataMicroBenchmarkFixture/BM_TPCH_lineitem_filter_by_specific_shipdate_table_scan            2978850 ns    2978813 ns        236
TPCHDataMicroBenchmarkFixture/BM_TPCH_lineitem_filter_by_specific_shipdate_groupKeyIndex_scan     240625 ns     240619 ns       2915
TPCHDataMicroBenchmarkFixture/BM_TPCH_lineitem_filter_by_specific_shipdate_bTreeIndex_scan        241255 ns     241233 ns       2815
```

#### Scan `LINEITEM` | `l_quantity = 42.0`
- lineitem table: 6'001'215 rows
- result table: 120'372 rows
- (table scan CPU time) / (group index scan CPU time) = **7,587546036**

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
- Loading/Generating tables done (15 s 277 ms)
- Encoding tables if necessary
-  Encoding 'nation' - encoding applied (728 µs 735 ns)
-  Encoding 'orders' - encoding applied (450 ms 828 µs)
-  Encoding 'region' - encoding applied (481 µs 561 ns)
-  Encoding 'part' - encoding applied (432 ms 833 µs)
-  Encoding 'lineitem' - encoding applied (1 s 345 ms)
-  Encoding 'partsupp' - encoding applied (404 ms 286 µs)
-  Encoding 'customer' - encoding applied (559 ms 645 µs)
-  Encoding 'supplier' - encoding applied (44 ms 711 µs)
- Encoding tables done (3 s 240 ms)
- Adding Tables to StorageManager and generating statistics
-  Adding 'nation' (161 µs 820 ns)
-  Adding 'orders' (2 s 58 ms)
-  Adding 'region' (36 µs 261 ns)
-  Adding 'part' (241 ms 731 µs)
-  Adding 'lineitem' (9 s 688 ms)
-  Adding 'partsupp' (1 s 341 ms)
-  Adding 'customer' (340 ms 289 µs)
-  Adding 'supplier' (15 ms 953 µs)
- Adding Tables to StorageManager and generating statistics done (13 s 686 ms)
-------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                                              Time           CPU Iterations
-------------------------------------------------------------------------------------------------------------------------------------
TPCHDataMicroBenchmarkFixture/BM_TPCH_lineitem_filter_by_specific_quantity_table_scan            3982486 ns    3982407 ns        165
TPCHDataMicroBenchmarkFixture/BM_TPCH_lineitem_filter_by_specific_quantity_groupKeyIndex_scan     524861 ns     524861 ns       1286
TPCHDataMicroBenchmarkFixture/BM_TPCH_lineitem_filter_by_specific_quantity_bTreeIndex_scan        527743 ns     527726 ns       1165
```

#### Scan `LINEITEM` | `l_extendedprice = 9999.0`
- lineitem table: 6'001'215 rows
- result table: 4 rows
- (table scan CPU time) / (group index scan CPU time) = **2,0118**

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
- Loading/Generating tables done (17 s 288 ms)
- Encoding tables if necessary
-  Encoding 'nation' - encoding applied (1 ms 306 µs)
-  Encoding 'orders' - encoding applied (631 ms 982 µs)
-  Encoding 'region' - encoding applied (560 µs 714 ns)
-  Encoding 'part' - encoding applied (446 ms 570 µs)
-  Encoding 'lineitem' - encoding applied (1 s 571 ms)
-  Encoding 'partsupp' - encoding applied (389 ms 832 µs)
-  Encoding 'customer' - encoding applied (553 ms 137 µs)
-  Encoding 'supplier' - encoding applied (37 ms 873 µs)
- Encoding tables done (3 s 633 ms)
- Adding Tables to StorageManager and generating statistics
-  Adding 'nation' (162 µs 791 ns)
-  Adding 'orders' (2 s 145 ms)
-  Adding 'region' (29 µs 815 ns)
-  Adding 'part' (247 ms 583 µs)
-  Adding 'lineitem' (9 s 338 ms)
-  Adding 'partsupp' (1 s 334 ms)
-  Adding 'customer' (311 ms 949 µs)
-  Adding 'supplier' (19 ms 568 µs)
- Adding Tables to StorageManager and generating statistics done (13 s 397 ms)
------------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                                                   Time           CPU Iterations
------------------------------------------------------------------------------------------------------------------------------------------
TPCHDataMicroBenchmarkFixture/BM_TPCH_lineitem_filter_by_specific_extendedprice_table_scan             382405 ns     382405 ns       1812
TPCHDataMicroBenchmarkFixture/BM_TPCH_lineitem_filter_by_specific_extendedprice_groupKeyIndex_scan     190092 ns     190084 ns       3807
TPCHDataMicroBenchmarkFixture/BM_TPCH_lineitem_filter_by_specific_extendedprice_bTreeIndex_scan        181137 ns     181134 ns       3894
```
