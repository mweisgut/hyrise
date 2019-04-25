### Table Scan and Index Scan

In these benchmarks, first table `LINEITEM` is scanned using a TableScan with the predicate `l_shipdate >= '1995-09-01'` to get a reference table.
After that the resulting table is scanned with the predicate `l_shipdate < '1995-10-01'`.
Only the execution of the second scan is measured.

Scan input table: 2817779 rows
Scan output table: 75983 rows

The IndexScan is **0,017540869** times faster than the table scan.

```
2019-04-18 06:09:16
Running ./hyriseMicroBenchmarks
Run on (32 X 2500 MHz CPU s)
CPU Caches:
  L1 Data 32K (x32)
  L1 Instruction 32K (x32)
  L2 Unified 256K (x32)
  L3 Unified 25600K (x32)
Generating TPC-H data set with scale factor 1 and Dictionary encoding:
- Loading/Generating tables
- Loading/Generating tables done (20 s 193 ms)
- Encoding tables if necessary
-  Encoding 'nation' - encoding applied (717 µs 924 ns)
-  Encoding 'orders' - encoding applied (476 ms 636 µs)
-  Encoding 'region' - encoding applied (458 µs 507 ns)
-  Encoding 'part' - encoding applied (481 ms 541 µs)
-  Encoding 'lineitem' - encoding applied (1 s 727 ms)
-  Encoding 'partsupp' - encoding applied (534 ms 971 µs)
-  Encoding 'customer' - encoding applied (609 ms 858 µs)
-  Encoding 'supplier' - encoding applied (45 ms 496 µs)
- Encoding tables done (3 s 878 ms)
- Adding Tables to StorageManager and generating statistics
-  Adding 'nation' (165 µs 237 ns)
-  Adding 'orders' (2 s 643 ms)
-  Adding 'region' (38 µs 28 ns)
-  Adding 'part' (321 ms 161 µs)
-  Adding 'lineitem' (12 s 803 ms)
-  Adding 'partsupp' (1 s 512 ms)
-  Adding 'customer' (417 ms 463 µs)
-  Adding 'supplier' (27 ms 446 µs)
- Adding Tables to StorageManager and generating statistics done (17 s 725 ms)
------------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                                                   Time           CPU Iterations
------------------------------------------------------------------------------------------------------------------------------------------
TPCHDataMicroBenchmarkFixture/BM_TPCH_lineitem_reference_table_table_scan                             6392396 ns    6391687 ns        115
TPCHDataMicroBenchmarkFixture/BM_TPCH_lineitem_reference_table_index_scan                           364399910 ns  364388269 ns          2
```

### Join: reduced PART with reduced LINEITEM

#### First Attempt

In these benchmarks, first table PART is reduced by a scan. This leads to an intermediate result table with only 3 rows (filter predicate: p_partkey <= 3). Let this intermediate table be REDUCED_PART. Additionally, let REDUCED_LINEITEM be a reduced LINEITEM table. It only contains tuples of the original table where l_shipdate >= '1995-09-01' AND l_shipdate < '1995-10-01'. REDUCED_PART is then joined with table REDUCED_LINEITEM using the join predicate p_partkey = l_partkey. REDUCED_LINEITEM has indices for column l_shipdate.

The `data_table`-benchmarks are using REDUCED_LINEITEM as data table (the result table of the second scan is stored as data table),
the `reference_table`-benchmarks are using the REDUCED_LINEITEM as reference table (the output table of the second scan).

The result below shows that the JoinIndex is a factor of **116,2488** faster than the JoinHash if REDUCED_LINEITEM is a data table.
If the REDUCED_LINEITEM is the output of the second scan and thus a reference table,
the JoinIndex is a factor of **1,0997** faster than the JoinHash.

reduced part table (left join input): 3 rows  
reduced lineitem table (right join input): 75'983 rows  
join result table: 1 row  
In this benchmark, first table  

**We should investigate the message** `[PERF] Only 0 of 61 chunks scanned using an index at src/lib/operators/join_index.cpp:173`.

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
- Loading/Generating tables done (20 s 246 ms)
- Encoding tables if necessary
-  Encoding 'nation' - encoding applied (1 ms 315 µs)
-  Encoding 'orders' - encoding applied (786 ms 808 µs)
-  Encoding 'region' - encoding applied (576 µs 357 ns)
-  Encoding 'part' - encoding applied (490 ms 186 µs)
-  Encoding 'lineitem' - encoding applied (1 s 718 ms)
-  Encoding 'partsupp' - encoding applied (437 ms 163 µs)
-  Encoding 'customer' - encoding applied (615 ms 865 µs)
-  Encoding 'supplier' - encoding applied (42 ms 725 µs)
- Encoding tables done (4 s 94 ms)
- Adding Tables to StorageManager and generating statistics
-  Adding 'nation' (339 µs 20 ns)
-  Adding 'orders' (2 s 509 ms)
-  Adding 'region' (33 µs 424 ns)
-  Adding 'part' (319 ms 975 µs)
-  Adding 'lineitem' (11 s 971 ms)
-  Adding 'partsupp' (1 s 441 ms)
-  Adding 'customer' (402 ms 856 µs)
-  Adding 'supplier' (19 ms 863 µs)
- Adding Tables to StorageManager and generating statistics done (16 s 664 ms)
[PERF] get_value() used at src/lib/storage/table.hpp:129
        Performance can be affected. This warning is only shown once.

[PERF] operator[] used at src/lib/storage/reference_segment.cpp:31
        Performance can be affected. This warning is only shown once.

[PERF] operator[] used at src/lib/storage/dictionary_segment.cpp:26
        Performance can be affected. This warning is only shown once.

[PERF] get_value() used at src/lib/storage/table.hpp:129
        Performance can be affected. This warning is only shown once.

[PERF] operator[] used at src/lib/storage/dictionary_segment.cpp:26
        Performance can be affected. This warning is only shown once.

[PERF] get_value() used at src/lib/storage/table.hpp:129
        Performance can be affected. This warning is only shown once.

[PERF] operator[] used at src/lib/storage/dictionary_segment.cpp:26
        Performance can be affected. This warning is only shown once.

------------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                                                   Time           CPU Iterations
------------------------------------------------------------------------------------------------------------------------------------------
TPCHDataMicroBenchmarkFixture/BM_TPCH_reduced_part_and_reduced_lineitem_data_table_hash_join           933131 ns     933129 ns        649
TPCHDataMicroBenchmarkFixture/BM_TPCH_reduced_part_and_reduced_lineitem_data_table_index_join            8027 ns       8027 ns      80806
TPCHDataMicroBenchmarkFixture/BM_TPCH_reduced_part_and_reduced_lineitem_reference_table_hash_join     2898362 ns    2898320 ns        235
[PERF] Only 0 of 61 chunks scanned using an index at src/lib/operators/join_index.cpp:173
        Performance can be affected. This warning is only shown once.

TPCHDataMicroBenchmarkFixture/BM_TPCH_reduced_part_and_reduced_lineitem_reference_table_index_join    2635585 ns    2635556 ns        259

```

#### Second Attempt

commit id: `67b7dae1450a66a85a804bb4dbebdddf9589b0e8`
Scale Factor 1.0
The IndexScan is **0,253471597** times faster than the table scan.

```
2019-04-25 12:01:09
Running ./hyriseMicroBenchmarks
Run on (80 X 2395 MHz CPU s)
CPU Caches:
  L1 Data 32K (x40)
  L1 Instruction 32K (x40)
  L2 Unified 256K (x40)
  L3 Unified 30720K (x4)
***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
Generating TPC-H data set with scale factor 1 and Dictionary encoding:
- Loading/Generating tables 
- Loading/Generating tables done (1 min 37 s)
- Encoding tables if necessary
-  Encoding 'nation' - encoding applied (1 ms 499 µs)
-  Encoding 'orders' - encoding applied (3 s 3 ms)
-  Encoding 'region' - encoding applied (1 ms 49 µs)
-  Encoding 'part' - encoding applied (2 s 971 ms)
-  Encoding 'lineitem' - encoding applied (6 s 323 ms)
-  Encoding 'partsupp' - encoding applied (1 s 467 ms)
-  Encoding 'customer' - encoding applied (3 s 58 ms)
-  Encoding 'supplier' - encoding applied (234 ms 844 µs)
- Encoding tables done (17 s 60 ms)
- Adding Tables to StorageManager and generating statistics 
-  Adding 'nation' (571 µs 77 ns)
-  Adding 'orders' (13 s 624 ms)
-  Adding 'region' (231 µs 903 ns)
-  Adding 'part' (1 s 847 ms)
-  Adding 'lineitem' (1 min 9 s)
-  Adding 'partsupp' (4 s 84 ms)
-  Adding 'customer' (1 s 698 ms)
-  Adding 'supplier' (99 ms 984 µs)
- Adding Tables to StorageManager and generating statistics done (1 min 31 s)
[PERF] Using type-erased accessor as the ReferenceSegmentIterable is type-erased itself at src/lib/storage/reference_segment/reference_segment_iterable.hpp:57
        Performance can be affected. This warning is only shown once.

------------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                                                   Time           CPU Iterations
------------------------------------------------------------------------------------------------------------------------------------------
TPCHDataMicroBenchmarkFixture/BM_TPCH_reduced_part_and_reduced_lineitem_reference_table_hash_join    48209320 ns   48187412 ns         11
TPCHDataMicroBenchmarkFixture/BM_TPCH_reduced_part_and_reduced_lineitem_reference_table_index_join  190173864 ns  190109711 ns          4
```

#### Third Attempt

commit id: `87ee079d5df8934701d0d82ab6d892148fe28882`
Scale Factor 10.0
The IndexScan is **4,074700484** times faster than the table scan.

```
Running ./hyriseMicroBenchmarks
Run on (80 X 2395 MHz CPU s)
CPU Caches:
  L1 Data 32K (x40)
  L1 Instruction 32K (x40)
  L2 Unified 256K (x40)
  L3 Unified 30720K (x4)
***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
Generating TPC-H data set with scale factor 10 and Dictionary encoding:
- Loading/Generating tables 
- Loading/Generating tables done (3 min 6 s)
- Encoding tables if necessary
-  Encoding 'nation' - encoding applied (1 ms 29 µs)
-  Encoding 'orders' - encoding applied (2 s 849 ms)
-  Encoding 'region' - encoding applied (5 ms 3 µs)
-  Encoding 'part' - encoding applied (923 ms 311 µs)
-  Encoding 'lineitem' - encoding applied (13 s 524 ms)
-  Encoding 'partsupp' - encoding applied (2 s 138 ms)
-  Encoding 'customer' - encoding applied (1 s 279 ms)
-  Encoding 'supplier' - encoding applied (879 ms 904 µs)
- Encoding tables done (21 s 599 ms)
- Adding Tables to StorageManager and generating statistics 
-  Adding 'nation' (259 µs 239 ns)
-  Adding 'orders' (42 s 286 ms)
-  Adding 'region' (116 µs 375 ns)
-  Adding 'part' (4 s 796 ms)
-  Adding 'lineitem' (2 min 32 s)
-  Adding 'partsupp' (17 s 943 ms)
-  Adding 'customer' (7 s 106 ms)
-  Adding 'supplier' (300 ms 995 µs)
- Adding Tables to StorageManager and generating statistics done (3 min 45 s)
------------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                                                   Time           CPU Iterations
------------------------------------------------------------------------------------------------------------------------------------------
TPCHDataMicroBenchmarkFixture/BM_TPCH_reduced_part_and_reduced_lineitem_reference_table_hash_join   136170268 ns  136105460 ns          4
TPCHDataMicroBenchmarkFixture/BM_TPCH_reduced_part_and_reduced_lineitem_reference_table_index_join   33417554 ns   33402568 ns         21
```