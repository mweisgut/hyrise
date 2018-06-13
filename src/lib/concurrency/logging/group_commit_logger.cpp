/*
 *     Commit Entries:
 *       - log entry type ('t') : sizeof(char)
 *       - transaction_id       : sizeof(TransactionID)
 * 
 *     Value Entries:
 *       - log entry type ('v') : sizeof(char)
 *       - transaction_id       : sizeof(transaction_id_t)
 *       - table_name.size()    : sizeof(size_t)             --> what is max table_name size?
 *       - table_name           : table_name.size()
 *       - row_id               : sizeof(ChunkID) + sizeof(ChunkOffset)
 *       - length(value)        : sizeof(size_t)
 *       - value                : length(value)
 *       - { length(value) + value } *
 *       - end indicator        : sizeof(int)
 *         (-> length(value) = 0)
 * 
 *     Invalidation Entries:
 *       - log entry type ('i') : sizeof(char)
 *       - transaction_id       : sizeof(TransactionID)
 *       - table_name.size()    : sizeof(size_t)             --> what is max table_name size?
 *       - table_name           : table_name.size()
 *       - row_id               : sizeof(ChunkID) + sizeof(ChunkOffset)     
 *
 */


#include "group_commit_logger.hpp"
#include "logger.hpp"
#include "binary_recovery.hpp"
#include "types.hpp"

#include <chrono>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sstream>
#include <thread>
#include <fstream>
#include <future>
#include <string>

#include <iostream>

namespace opossum {

constexpr size_t LOG_BUFFER_CAPACITY = 16384;

constexpr auto LOG_INTERVAL = std::chrono::seconds(5);

void GroupCommitLogger::commit(const TransactionID transaction_id, std::function<void(TransactionID)> callback){
  constexpr auto entry_length = sizeof(char) + sizeof(TransactionID);
  auto entry = (char*) malloc(entry_length);
  *entry = 't';
  *(TransactionID*) (entry + sizeof(char)) = transaction_id;

  _commit_callbacks.emplace_back(std::make_pair(callback, transaction_id));

  _write_to_buffer(entry, entry_length);

  free(entry);
}

char* GroupCommitLogger::_put_into_entry(char* entry, const TransactionID &transaction_id, const std::string &table_name, const RowID &row_id) {
  *(TransactionID*) entry = transaction_id;
  entry += sizeof(TransactionID);

  *(size_t*) entry = table_name.size();
  entry += sizeof(size_t);

  table_name.copy(entry, table_name.size());
  entry += table_name.size();

  *(ChunkID*) entry = row_id.chunk_id;
  entry += sizeof(ChunkID);

  *(ChunkOffset*) entry = row_id.chunk_offset;
  entry += sizeof(ChunkOffset);

  return entry;
}

 /*     Value Entries:
 *       - log entry type ('v') : sizeof(char)
 *       - transaction_id       : sizeof(transaction_id_t)
 *       - table_name.size()    : sizeof(size_t)             --> what is max table_name size?
 *       - table_name           : table_name.size()
 *       - row_id               : sizeof(ChunkID) + sizeof(ChunkOffset)
 *       - length(value)        : sizeof(int)
 *       - value                : length(value)
 *       - { length(value) + value } *
 *       - end indicator        : sizeof(int)
 *         (-> length(value) = 0)
 */
void GroupCommitLogger::value(const TransactionID transaction_id, const std::string table_name, const RowID row_id, const std::vector<AllTypeVariant> values){
  auto entry_length = sizeof(char) + sizeof(TransactionID) + sizeof(size_t) + table_name.size() + sizeof(ChunkID) + sizeof(ChunkOffset) + sizeof(size_t);
  // TODO: use mmap ?

  std::vector<std::string> value_strings;
  for (auto &value : values) {
    std::stringstream value_ss;
    value_ss << value;

    value_strings.push_back(value_ss.str());

    entry_length += sizeof(size_t) + value_ss.str().length();
  }

  auto entry = (char*) malloc(entry_length);

  *entry = 'v';
  auto current_pos = entry + sizeof(char);

  current_pos = _put_into_entry(current_pos, transaction_id, table_name, row_id);

  for (auto &value_str : value_strings){
    *(size_t*)(current_pos) = value_str.length();
    current_pos += sizeof(size_t);

    *current_pos = *value_str.c_str();
    current_pos += value_str.length();
  }

  // end indicator: length of next value = 0
  *(size_t*) current_pos = 0;

  _write_to_buffer(entry, entry_length);
  
  free(entry);
}

void GroupCommitLogger::invalidate(const TransactionID transaction_id, const std::string table_name, const RowID row_id){
  const auto entry_length = sizeof(char) + sizeof(TransactionID) + sizeof(size_t) + table_name.size() + sizeof(ChunkID) + sizeof(ChunkOffset);
  auto entry = (char*) malloc(entry_length);
  
  *entry = 'i';
  auto current_pos = entry + sizeof(char);

  current_pos = _put_into_entry(current_pos, transaction_id, table_name, row_id);

  _write_to_buffer(entry, entry_length);

  free(entry);
}

void GroupCommitLogger::_write_to_buffer(char* entry, size_t length) {
  _buffer_mutex.lock();

  memcpy(_buffer + _buffer_position, entry, length);
  _buffer_position += length;
  _has_unflushed_buffer = true;

  _buffer_mutex.unlock();

  if (_buffer_position > _buffer_capacity / 2) {
    flush();
  }
}

void GroupCommitLogger::_write_buffer_to_logfile(){
  //TODO: perhaps second buffer, then buffer has not to be locked
  _file_mutex.lock();
  _buffer_mutex.lock();

  _log_file.write(_buffer, _buffer_position);
  _log_file.sync();

  _file_mutex.unlock();

  _buffer_position = 0u;
  _has_unflushed_buffer = false;

  for (auto &callback_tuple : _commit_callbacks) {
    callback_tuple.first(callback_tuple.second);
  }
  _commit_callbacks.clear();

  _buffer_mutex.unlock();
}

void GroupCommitLogger::_flush_to_disk_after_timeout(){
  // TODO: while loop should be exited on program termination
  while (true) {
    std::this_thread::sleep_for(LOG_INTERVAL);
    flush();
  }
}

void GroupCommitLogger::flush(){
  if (_has_unflushed_buffer) {
    _write_buffer_to_logfile();
  }
}

void GroupCommitLogger::recover() {
  BinaryRecovery::getInstance().recover();
}


GroupCommitLogger::GroupCommitLogger()
: AbstractLogger()
, _buffer_capacity(LOG_BUFFER_CAPACITY)
, _buffer_position(0u)
, _has_unflushed_buffer(false) {
  _buffer = (char*)malloc(_buffer_capacity);
  memset(_buffer, 0, _buffer_capacity);

  std::fstream last_log_number_file(Logger::directory + Logger::last_log_filename, std::ios::in);
  uint log_number;
  last_log_number_file >> log_number;
  last_log_number_file.close();
  ++log_number;

  _log_file.open(Logger::directory + Logger::filename + std::to_string(log_number), std::ios::out | std::ios::binary);

  last_log_number_file.open(Logger::directory + Logger::last_log_filename, std::ios::out | std::ofstream::trunc);
  last_log_number_file << std::to_string(log_number);
  last_log_number_file.close();

  // TODO: thread should be joined at the end of the program
  std::thread t1(&GroupCommitLogger::_flush_to_disk_after_timeout, this);
  t1.detach();
};

GroupCommitLogger::~GroupCommitLogger() {
  _log_file.close();
  free(_buffer);
}

}  // namespace opossum
