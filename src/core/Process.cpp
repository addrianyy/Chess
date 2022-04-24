#include "Process.hpp"

#include <optional>
#include <span>

#define NOMINMAX
#include <Windows.h>

static void verify(bool boolean, const std::string& problem) {
  if (!boolean) {
    MessageBoxA(nullptr, "Error", problem.c_str(), MB_ICONERROR | MB_OK);
    std::exit(1);
  }
}

static std::optional<size_t> find_null_or_newline(std::span<uint8_t> data) {
  for (size_t i = 0; i < data.size(); ++i) {
    if (data[i] == '\n' || data[i] == '\0') {
      return i;
    }
  }

  return std::nullopt;
}

Process::Process(const std::string& path) {
  std::vector<char> command_line(path.size() + 1);
  std::memcpy(command_line.data(), path.c_str(), path.size() + 1);

  SECURITY_ATTRIBUTES attribs{};
  attribs.nLength = sizeof(SECURITY_ATTRIBUTES);
  attribs.bInheritHandle = true;
  attribs.lpSecurityDescriptor = nullptr;

  HANDLE stdin_read;
  HANDLE stdout_write;

  verify(CreatePipe(&stdin_read, &stdin_write, &attribs, 0), "Failed to create stdin pipe.");
  verify(CreatePipe(&stdout_read, &stdout_write, &attribs, 0), "Failed to create stdout pipe.");

  verify(SetHandleInformation(stdin_write, HANDLE_FLAG_INHERIT, 0),
         "Failed to set inherit information for stdin handles.");
  verify(SetHandleInformation(stdout_read, HANDLE_FLAG_INHERIT, 0),
         "Failed to set inherit information for stdout handles.");

  STARTUPINFOA startup_info{};
  PROCESS_INFORMATION process_info{};

  startup_info.cb = sizeof(STARTUPINFOA);
  startup_info.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
  startup_info.hStdError = nullptr;
  startup_info.hStdOutput = stdout_write;
  startup_info.hStdInput = stdin_read;
  startup_info.wShowWindow = SW_HIDE;

  verify(CreateProcessA(nullptr, command_line.data(), nullptr, nullptr, true, 0, nullptr, nullptr,
                        &startup_info, &process_info),
         "Failed to start the process.");

  CloseHandle(stdin_read);
  CloseHandle(stdout_write);
  CloseHandle(process_info.hThread);

  process_handle = process_info.hProcess;
}

Process::~Process() {
  wait_for_exit();

  CloseHandle(process_handle);
  CloseHandle(stdout_read);
  CloseHandle(stdin_write);
}

bool Process::read_stdout_internal(uint8_t* output, size_t output_size, size_t& bytes_read) {
  bytes_read = 0;

  if (output_size == 0) {
    return false;
  }

  if (stdout_buffer.empty()) {
    DWORD bytes_read_;
    if (!ReadFile(stdout_read, output, output_size, &bytes_read_, nullptr)) {
      return false;
    }

    bytes_read = bytes_read_;

    return true;
  }

  const auto read_size = std::min(output_size, stdout_buffer.size());
  std::memcpy(output, stdout_buffer.data(), read_size);

  bytes_read = read_size;

  if (read_size == stdout_buffer.size()) {
    stdout_buffer.clear();
  } else {
    const size_t size_left = stdout_buffer.size() - read_size;
    std::memmove(stdout_buffer.data(), stdout_buffer.data() + read_size, size_left);
    stdout_buffer.resize(size_left);
  }

  return true;
}

std::string Process::read_line() {
  uint8_t buffer[4096];
  size_t bytes_read;
  verify(read_stdout_internal(buffer, sizeof(buffer), bytes_read),
         "Failed to read from process stdout.");

  std::vector<uint8_t> big_buffer;
  const auto copy_to_big_buffer = [&]() {
    const auto before_size = big_buffer.size();
    big_buffer.resize(big_buffer.size() + bytes_read);
    std::memcpy(big_buffer.data() + before_size, buffer, bytes_read);
  };

  auto read_data = std::span<uint8_t>(buffer, bytes_read);

  while (true) {
    if (const auto null_newline = find_null_or_newline(read_data)) {
      auto line_data = read_data.subspan(0, *null_newline);

      // Trim.
      while (!line_data.empty() && isspace(line_data[line_data.size() - 1])) {
        line_data = line_data.subspan(0, line_data.size() - 1);
      }

      if (*null_newline != read_data.size() - 1) {
        const auto rest_data = read_data.subspan(*null_newline + 1);
        if (!rest_data.empty()) {
          const auto before_size = stdout_buffer.size();
          stdout_buffer.resize(stdout_buffer.size() + rest_data.size());
          std::memcpy(stdout_buffer.data() + before_size, rest_data.data(), rest_data.size());
        }
      }

      const auto string_ptr = (const char*)line_data.data();
      return std::string(string_ptr, string_ptr + line_data.size());
    }

    if (big_buffer.empty()) {
      copy_to_big_buffer();
    }

    verify(read_stdout_internal(buffer, sizeof(buffer), bytes_read),
           "Failed to read from process stdout.");
    copy_to_big_buffer();

    read_data = big_buffer;
  }
}

void Process::write_line(const std::string& string) {
  const std::string with_newline = string + '\n';
  std::span<const uint8_t> write_data((const uint8_t*)with_newline.data(), with_newline.length());

  while (!write_data.empty()) {
    DWORD bytes_written;
    verify(WriteFile(stdin_write, write_data.data(), write_data.size(), &bytes_written, nullptr),
           "Failed to write to process stdin.");

    write_data = write_data.subspan(size_t(bytes_written));
  }
}

void Process::wait_for_exit() {
  if (!exited) {
    WaitForSingleObject(process_handle, INFINITE);
    exited = true;
  }
}