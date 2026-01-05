#pragma once

#include <concepts>
#include <QObject>
#include <QBoxLayout>
#include <QEvent>
#include <QPushButton>
#include <QWidget>
#include <QString>
#include <QBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QWidget>

#include <qcombobox.h>
#include <qlist.h>
#include <qnamespace.h>
#include <qspinbox.h>
#include <ranges>
#include <string>
#include <tuple>
#include <variant>

#include "utils.hpp"

template <typename T>
concept CreatableWidget = requires (T widget, QWidget* parent, QBoxLayout* layout) {
{ widget.create_widget(parent, layout) } -> std::convertible_to<QWidget*>;
};

template <CreatableWidget ...Widgets>
auto add_widgets_to(QWidget* parent, QBoxLayout* layout, std::tuple<Widgets...> widgets)
{
  std::apply([parent, layout](auto&&... members) {
    (([&]() {
      members.create_widget(parent, layout);
    }()), ...);
  }, widgets);
  layout->addStretch();
}

template <typename T>
class Widget {
public:

  auto withAlignment(this auto&& self, Qt::Alignment alignment)
  {
    self.m_alignment = alignment;
    return self;
  }
  virtual auto create_widget(QWidget* parent, QBoxLayout* layout) -> QWidget* = 0;

protected:
  Qt::Alignment m_alignment{Qt::AlignCenter};
  uint m_stretch{};
};

enum class LayoutDirection {
  Vertical,
  Horizontal
};

template <CreatableWidget... Widgets>
struct Layout : public Widget<Layout<Widgets...>> {
  explicit Layout(LayoutDirection dir, Widgets&&... widgets)
  : m_widgets(std::forward<Widgets>(widgets)...), m_direction(dir) {}

  explicit Layout(Widgets&&... widgets) 
    : m_widgets(std::forward<Widgets>(widgets)...),
    m_direction(LayoutDirection::Horizontal) {}

  auto create_widget(QWidget* parent, QBoxLayout* parent_layout) -> QWidget*
  {
    QBoxLayout* layout = nullptr;
    if (m_direction == LayoutDirection::Vertical) {
      layout = new QVBoxLayout();
    } else {
      layout = new QHBoxLayout();
    }
    m_layout_ptr = layout;
    if (m_spacing) layout->setSpacing(m_spacing.value());
    if (m_stretched) layout->addStretch();

    ::add_widgets_to(parent, layout, m_widgets);
    if (parent_layout)
      parent_layout->addLayout(layout);
    return parent;
  }

  auto fitTo(QWidget* parent)
  {
    auto layout = new QHBoxLayout();
    auto widget = create_widget(parent, nullptr);
    parent->setLayout(m_layout_ptr);
  }

  auto withSpacing(this auto&& self, uint spacing)
  {
    self.m_spacing = spacing;
    return self;
  }

  auto addStretch(this auto&& self, bool stretch = true)
  {
    self.m_stretched = stretch;
    return self;
  }

private:
  std::tuple<Widgets...> m_widgets;
  std::optional<bool> m_stretched{};
  std::optional<uint> m_spacing{};
  QBoxLayout* m_layout_ptr;
  LayoutDirection m_direction;
};

template <typename... Widgets>
struct HLayout : public Layout<Widgets...> {
  explicit HLayout(Widgets&&... widgets) :
    Layout<Widgets...>(LayoutDirection::Horizontal, std::forward<Widgets>(widgets)...)
  {}
};

template <typename... Widgets>
struct VLayout : public Layout<Widgets...> {
  explicit VLayout(Widgets&&... widgets) :
    Layout<Widgets...>(LayoutDirection::Vertical, std::forward<Widgets>(widgets)...)
  {}
};

struct Button : public Widget<Button> {
  Button() = default;
  Button(std::string&& text) : m_text(std::move(text)) {}
  Button(const std::string& text) : m_text(text) {}

  auto create_widget(QWidget* parent, QBoxLayout* layout) -> QWidget*
  {
    auto* widget = new QPushButton(QString::fromStdString(m_text), parent);
    layout->addWidget(widget);
    return widget;
  }

private:
  std::string m_text;
};

struct Text : public Widget<Text> {
  Text() = default;
  Text(std::string&& text) : m_text(std::move(text)) {}
  Text(const std::string& text) : m_text(text) {}

  auto create_widget(QWidget* parent, QBoxLayout* layout) -> QWidget*
  {
    auto* widget = new QLabel(QString::fromStdString(m_text), parent);
    layout->addWidget(widget);
    return widget;
  }
private:
  std::string m_text;
};

struct Input: public Widget<Input> {
  enum class ValueType {
    U64,
    F64,
    String
  };
  Input(
    ValueType type = ValueType::U64,
    std::optional<std::string> placeholder = {},
    std::optional<std::variant<f64, u64, std::string>> default_value = {}
  ): m_placeholder(placeholder),
    m_type(type),
    m_default_value(default_value)
  {}


  auto valueChanged(this auto&& self, auto listener)
  {
    self.m_value_changed = listener;
    return self;
  }
  auto create_widget(QWidget* parent, QBoxLayout* layout) -> QWidget*
  {
    QWidget* widget;
    QSpinBox* uint_widget;
    QDoubleSpinBox* float_widget;
    QLineEdit* string_widget;
    switch (m_type) {
    case ValueType::U64:
      uint_widget = new QSpinBox(parent);
      uint_widget->setValue(std::get<f64>(m_default_value.value_or(u64{})));
      widget = uint_widget;
      ::QObject::connect(uint_widget, &QSpinBox::textChanged, [this](QString val){
        m_value = std::stoul(val.toStdString());
        m_value_changed.value_or([](decltype(m_value)){ })(m_value);
      });
      break;
    case ValueType::F64:
      float_widget = new QDoubleSpinBox(parent);
      float_widget->setValue(std::get<f64>(m_default_value.value_or(f64{})));
      widget = float_widget;
      ::QObject::connect(float_widget, &QDoubleSpinBox::textChanged, [this](QString val){
        m_value = std::stod(val.toStdString());
        m_value_changed.value_or([](decltype(m_value)){ })(m_value);
      });
      break;
    case ValueType::String:
      string_widget = new QLineEdit(parent);
      string_widget->setPlaceholderText(QString::fromStdString(m_placeholder.value_or("")));
      widget = string_widget;
      ::QObject::connect(string_widget, &QLineEdit::textChanged, [this](QString val){
        m_value = std::stod(val.toStdString());
        m_value_changed.value_or([](decltype(m_value)){ })(m_value);
      });
      break;
    }
    layout->addWidget(widget);
    return widget;
  }
private:
  std::variant<f64, u64, std::string> m_value;
  std::optional<std::string> m_placeholder;
  std::optional<std::variant<f64, u64, std::string>> m_default_value;
  std::optional<std::function<void(std::variant<f64, u64, std::string>)>> m_value_changed;
  ValueType m_type{ValueType::U64};
};

struct ComboBox : public Widget<Button> {
  ComboBox() = default;
  ComboBox(
    std::optional<std::string> base_value = {},
    std::optional<std::vector<std::string>> options = {}
  ) : m_base_value(base_value), m_options(options) {}


  auto create_widget(QWidget* parent, QBoxLayout* layout) -> QWidget*
  {
    auto* widget = new QComboBox(parent);
    widget->setPlaceholderText(QString::fromStdString(m_base_value.value_or("")));

    auto items = m_options.value_or({})                           |
                              std::views::transform(&QString::fromStdString) |
                              std::ranges::to<QStringList>();
    widget->addItems(items);
    layout->addWidget(widget);
    return widget;
  }

private:
  std::optional<std::string> m_base_value;
  std::optional<std::vector<std::string>> m_options;
  std::optional<std::function<void(std::string)>> m_value_changed;
};
