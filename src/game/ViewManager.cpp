#include "ViewManager.hpp"

void ViewManager::set_view(View* view) {
  if (current_view != view) {
    const auto previous_view = current_view;

    current_view = view;

    view->on_switched_view(previous_view);
  }
}