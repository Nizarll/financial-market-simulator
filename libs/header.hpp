#pragma once

#include <functional>
#include <QWidget>
#include <string>
#include "utils.hpp"

class Header : public QWidget {
  Q_OBJECT;

  std::vector<std::function<void(bool)>> m_theme_liseners;
  std::vector<std::function<void(const std::string&)>> m_window_listeners;
public:
  explicit Header(QWidget* parent = nullptr);
  void theme_toggled(std::function<void(bool)> listener);
  void window_switched(std::function<void(const std::string& str)> listener);
};
