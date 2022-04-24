#pragma once
#include <string>
#include <vector>

class Process {
  void* process_handle = nullptr;

  void* stdin_write = nullptr;
  void* stdout_read = nullptr;

  bool exited = false;

  std::vector<uint8_t> stdout_buffer;

  bool read_stdout_internal(uint8_t* output, size_t output_size, size_t& bytes_read);

public:
  Process(Process&&) = delete;
  Process(const Process&) = delete;

  Process& operator=(const Process&) = delete;
  Process& operator=(Process&&) = delete;

  explicit Process(const std::string& path);
  ~Process();

  std::string read_line();
  void write_line(const std::string& string);

  void wait_for_exit();
};