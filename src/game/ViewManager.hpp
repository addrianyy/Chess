#pragma once
#include "View.hpp"

#include <memory>
#include <vector>

class ViewManager {
  std::vector<std::unique_ptr<View>> views;
  View* current_view = nullptr;

public:
  ViewManager() = default;

  ViewManager(const ViewManager&) = delete;
  ViewManager& operator=(const ViewManager&) = delete;

  template <typename T> T* add_view(std::unique_ptr<T> view) {
    const auto ptr = view.get();
    views.push_back(std::move(view));
    return ptr;
  }

  void set_view(View* view);

  View* get_view() { return current_view; }
  const std::vector<std::unique_ptr<View>>& get_all_views() { return views; }
};