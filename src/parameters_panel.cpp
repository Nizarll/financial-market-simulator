#include "parameters_panel.hpp"
#include <algorithm>
#include <concepts>
#include <functional>
#include <optional>

#include <qchar.h>
#include <qinputdialog.h>
#include <qlineedit.h>
#include <qnamespace.h>
#include <qspinbox.h>
#include <string>
#include <utility>
#include <QInputDialog>
#include <QBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <variant>
#include <iostream>

ParametersPanel::ParametersPanel(std::initializer_list<InputField> fields) : m_fields(fields)
{
  auto *layout = new QVBoxLayout(this);
  layout->setSpacing(0);

  for (auto &field : m_fields) {

    QWidget *inputWidget = nullptr;
    std::visit([&](auto &&arg) {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (std::same_as<T, int>) {
        auto *spin = new QSpinBox(this);
        spin->setRange(INT_MIN, INT_MAX);
        inputWidget = spin;
        connect(spin, &QSpinBox::textChanged, [this, &field](QString val){
          field.value = std::stoi(val.toStdString());
          for (auto listener : m_listeners.at(field.name)) std::invoke(listener, field);
        });
      } else if constexpr (std::same_as<T, f64>) {
        auto *dspin = new QDoubleSpinBox(this);
        dspin->setRange(-1e12, 1e12);
        dspin->setDecimals(6);
        inputWidget = dspin;
        connect(dspin, &QDoubleSpinBox::textChanged, [this, &field](QString val){
          field.value = std::stod(val.toStdString());
          for (auto listener : m_listeners.at(field.name)) std::invoke(listener, field);
        });
      } else {
        auto *edit = new QLineEdit(this);
        inputWidget = edit;
        connect(edit, &QLineEdit::textChanged, [this, &field](QString val){
          field.value = val;
          for (auto listener : m_listeners.at(field.name)) std::invoke(listener, field);
        });
      }
    }, field.value);

    auto *container = new QWidget(this);
    auto *vbox = new QVBoxLayout(container);
    vbox->addWidget(new QLabel(QString::fromStdString(field.name), container));
    vbox->addWidget(inputWidget);
    vbox->setAlignment(Qt::AlignTop);
    layout->addWidget(container);
  }
}

std::optional<ParametersPanel::InputField> ParametersPanel::retrieve_value(const std::string& name)
{
  auto it = std::find_if(m_fields.begin(), m_fields.end(), [&name](const InputField& f){
    return f.name == name;
  });
  if (it == m_fields.end()) return std::nullopt;

  return std::optional<InputField>{std::in_place_t{}, *it };
}

void ParametersPanel::value_changed(const std::string& name, std::function<void(InputField)> listener)
{
  auto& vector = m_listeners[name];
  vector.emplace_back(listener);
}
