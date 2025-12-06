#include "header.hpp"

#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QFont>
#include <qcombobox.h>

#include <string>
#include <array>

Header::Header(QWidget* parent): QWidget(parent)
{
  auto* layout = new QHBoxLayout();
  auto* theme_toggle = new QPushButton(this);
  auto* window_list = new QComboBox(this);

  QFont nerd_font("Hack Nerd Font", 12, QFont::Normal);

  
  auto options = std::array<std::string, 3>{
    "Geometric Brownian Motion",
    "Black Scholes",
    "Monte Carlo"
  };

  for(const auto& option : options)
    window_list->addItem(QString::fromStdString(option));

  theme_toggle->setCheckable(true);
  theme_toggle->setChecked(true);
  theme_toggle->setFont(nerd_font);
  theme_toggle->setText("");
  theme_toggle->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  theme_toggle->adjustSize();
  theme_toggle->setStyleSheet("QPushButton{ border:none; outline:none; }");

  connect(theme_toggle, &QPushButton::toggled, [this, theme_toggle](bool value) {
    for (auto listener : m_theme_liseners) listener(value);
    theme_toggle->setText(value ? "" : "");
  });

  layout->addWidget(theme_toggle);
  layout->addWidget(window_list);
  layout->addStretch();
}

void Header::theme_toggled(std::function<void(bool)> listener) { m_theme_liseners.push_back(listener); }
void Header::window_switched(std::function<void(const std::string& str)> listener) {}
