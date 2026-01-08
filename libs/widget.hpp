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
#include <QValueAxis>

#include <cstddef>
#include <functional>
#include <limits>
#include <new>
#include <qalgorithms.h>
#include <qchar.h>
#include <qchart.h>
#include <qchartview.h>
#include <qcombobox.h>
#include <qglobal.h>
#include <qlineseries.h>
#include <qlist.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qspinbox.h>
#include <ranges>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

#include "utils.hpp"

template <typename... Overloads>
class FunctionOverload {
  struct _FunctionOverloadStorage {
    static constexpr std::size_t size = std::max({sizeof(Overloads)...});
    static constexpr std::size_t alignment = std::max({alignof(Overloads)...});
    alignas(alignment) std::byte data[size];
  };
  _FunctionOverloadStorage storage;
  usz index;

  template <std::size_t I = 0>
  void copy_from(const FunctionOverload& other) {
    if constexpr (I < sizeof...(Overloads)) {
      if (other.index == I) {
        using T = std::tuple_element_t<I, std::tuple<Overloads...>>;
        new (storage.data) T(*std::launder(reinterpret_cast<const T*>(other.storage.data)));
      } else {
        copy_from<I + 1>(other);
      }
    }
  }

  template <std::size_t I = 0>
  void move_from(FunctionOverload& other) {
    if constexpr (I < sizeof...(Overloads)) {
      if (other.index == I) {
        using T = std::tuple_element_t<I, std::tuple<Overloads...>>;
        new (storage.data) T(std::move(*std::launder(reinterpret_cast<T*>(other.storage.data))));
      } else {
        move_from<I + 1>(other);
      }
    }
  }

  template <typename T, usz i, typename First, typename... Rest>
  static constexpr auto _index_of_impl() -> usz
  {
    if constexpr (std::same_as<T, First>) return i;
    else return _index_of_impl<T, i + 1, Rest...>();
  }

  template <typename T, usz i>
  static constexpr auto _index_of_impl() -> usz { return sizeof...(Overloads); }

  template <typename T, usz i = 0>
  static constexpr auto _index_of() -> usz
  {
    static_assert(_index_of_impl<T, i, Overloads...>() < sizeof...(Overloads), "Type is not a valid overload");
    return _index_of_impl<T, i, Overloads...>();
  }

public:
  FunctionOverload(const FunctionOverload& other) : index(other.index) { copy_from(other); }
  FunctionOverload(FunctionOverload&& other) noexcept : index(other.index) { move_from(other); }

  FunctionOverload& operator=(const FunctionOverload& other) {
    if (this != &other) {
      this->~FunctionOverload();
      index = other.index;
      copy_from(other);
    }
    return *this;
  }

  FunctionOverload& operator=(FunctionOverload&& other) noexcept {
    if (this != &other) {
      this->~FunctionOverload();
      index = other.index;
      move_from(other);
    }
    return *this;
  }

  template <typename T> requires (!std::same_as<std::decay_t<T>, FunctionOverload>)
  FunctionOverload(T&& t) {
    constexpr auto idx = []<std::size_t... Is>(std::index_sequence<Is...>) {
      std::size_t result = sizeof...(Overloads);
      ((std::convertible_to<std::decay_t<T>, Overloads> ? (result = std::min(result, Is), true) : false) || ...);
      return result;
    }(std::index_sequence_for<Overloads...>{});
    
    static_assert(idx < sizeof...(Overloads), "Type is not convertible to any overload");
    
    using TargetType = std::tuple_element_t<idx, std::tuple<Overloads...>>;
    index = idx;
    new (storage.data) TargetType(std::forward<T>(t));
  }

  ~FunctionOverload() {
    [this]<std::size_t... Is>(std::index_sequence<Is...>) {
      auto destroy = [this]<std::size_t I>() {
        if (index == I) {
          using T = std::tuple_element_t<I, std::tuple<Overloads...>>;
          reinterpret_cast<T*>(storage.data)->~T();
        }
      };
      (destroy.template operator()<Is>(), ...);
    }(std::index_sequence_for<Overloads...>{});
  }

  usz which() const { return index; }

  template <typename T>
  auto get(this auto&& self) {
    assert(self.index == _index_of<T>() && "Type passed to get<T> is invalid");
    return *std::launder(reinterpret_cast<T*>(const_cast<std::byte*>(self.storage.data)));
  }
};

struct Theme {
  Theme(QApplication& app, const std::string& style);
  Theme() = delete;

  enum ThemeState : uint {
    Light,
    Dark
  };

  ThemeState getState() const;
  void setState(ThemeState state);
private:
  std::reference_wrapper<QApplication> m_app;
  QString m_light_theme;
  QString m_dark_theme;
  ThemeState m_theme;
};

template <typename T>
concept CreatableWidget = requires (T widget, QWidget* parent, QBoxLayout* layout) {
{ widget.create_widget(parent, layout) } -> std::convertible_to<QWidget*>;
};

template <CreatableWidget ...Widgets>
auto add_widgets_to(QWidget* parent, QBoxLayout* layout, std::tuple<Widgets...> widgets, auto lambda = [](){})
{
  std::apply([parent, layout, lambda](auto&&... members) mutable {
    (([&]() {
      lambda();
      members.create_widget(parent, layout);
    }()), ...);
  }, widgets);
  layout->setSizeConstraint(QLayout::SetMinimumSize);
}

template <typename T>
struct Widget {

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

    if(m_justify_between) {
      ::add_widgets_to(parent, layout, m_widgets, [layout](){
        layout->addStretch();
      });
    }
    else {
      ::add_widgets_to(parent, layout, m_widgets, [layout](){
        layout->addStretch();
      });
    }


    layout->setSizeConstraint(QLayout::SetMinimumSize);
    m_layout_ptr = layout;
    if (m_spacing) layout->setSpacing(m_spacing.value());
    if (m_stretched) layout->addStretch();
    if (!parent_layout) layout->addStretch();
    else parent_layout->addLayout(layout);
    return parent;
  }

  auto fitTo(this auto&& self, QWidget* parent)
  {
    if (parent->layout()) qDeleteAll(parent->layout()->children());
    self.create_widget(parent, nullptr);
    parent->setLayout(self.m_layout_ptr);
    return self;
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

  auto justifyBetween(this auto&& self, bool justify = true)
  {
    self.m_justify_between = justify;
    return self;
  }

private:
  std::tuple<Widgets...> m_widgets;
  std::optional<uint> m_spacing{};
  QBoxLayout* m_layout_ptr;
  LayoutDirection m_direction;
  bool m_stretched{};
  bool m_justify_between{};
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
    ::QObject::connect(widget, &QPushButton::clicked, m_clicked.value_or([](bool){}));
    ::QObject::connect(widget, &QPushButton::pressed, m_pressed.value_or([](){}));
    return widget;
  }

  auto pressed(this auto&& self, std::function<void()> listener)
  {
    self.m_pressed = listener;
    return self;
  }
  auto clicked(this auto&& self, std::function<void(bool)> listener)
  {
    self.m_clicked = listener;
    return self;
  }

private:
  std::string m_text;
  std::optional<std::function<void(bool)>> m_clicked;
  std::optional<std::function<void()>> m_pressed;
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

  using OptionalListenerOverloads = std::optional<FunctionOverload<
  std::function<void(f64)>,
  std::function<void(u64)>,
  std::function<void(std::string)>
  >>;

  enum class ValueType {
    U64,
    F64,
    String
  };
  constexpr Input(
    ValueType type = ValueType::U64,
    std::optional<std::string> placeholder = {},
    std::optional<std::variant<f64, u64, std::string>> default_value = {}
  ): m_placeholder(placeholder),
    m_type(type),
    m_default_value(default_value)
  {}


  template <typename Func>
  constexpr auto valueChanged(this auto&& self, Func listener)
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
        uint_widget = new QSpinBox(parent); //TODO: find a fix for int -> u64 range conversion
        uint_widget->setValue(std::get<u64>(m_default_value.value_or(u64{})));
        widget = uint_widget;
        ::QObject::connect(uint_widget,
                           qOverload<int>(&QSpinBox::valueChanged),
                           [value_changed = m_value_changed](u64 val){
                           if (not value_changed.has_value()) return;
                           auto listener = value_changed.value().get<std::function<void(u64)>>();
                           listener(val);
                           });
        break;
      case ValueType::F64:
        float_widget = new QDoubleSpinBox(parent);
        float_widget->setValue(std::get<f64>(m_default_value.value_or(f64{})));
        widget = float_widget;
        ::QObject::connect(float_widget,
                           qOverload<double>(&QDoubleSpinBox::valueChanged),
                           [value_changed = m_value_changed](u64 val){
                           if (not value_changed.has_value()) return;
                           auto listener = value_changed.value().get<std::function<void(f64)>>();
                           listener(val);
                           });
        break;
      case ValueType::String:
        string_widget = new QLineEdit(parent);
        string_widget->setPlaceholderText(QString::fromStdString(m_placeholder.value_or("")));
        widget = string_widget;
        ::QObject::connect(string_widget,
                           &QLineEdit::textChanged,
                           [value_changed = m_value_changed](QString val){
                           if (not value_changed.has_value()) return;
                           auto listener = value_changed.value().get<std::function<void(std::string)>>();
                           listener(val.toStdString());
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
  OptionalListenerOverloads m_value_changed;
  std::optional<std::function<void(std::string)>> m_value_invalid;
  ValueType m_type{ValueType::U64};
};

struct ComboBox : public Widget<ComboBox> {
  ComboBox(
    std::optional<std::string> base_value = {}
  ) : m_base_value(base_value) {}
  auto create_widget(QWidget* parent, QBoxLayout* layout) -> QWidget*
  {
    auto* widget = new QComboBox(parent);
    widget->setPlaceholderText(QString::fromStdString(m_base_value.value_or("Hello")));
    layout->addWidget(widget);
    auto items = m_options.value_or({})   |
      std::views::transform(&QString::fromStdString) |
      std::ranges::to<QStringList>();
    widget->addItems(items);
    ::QObject::connect(
      widget,
      &QComboBox::currentTextChanged,
      [value_changed = m_value_changed](const QString& str) {
        value_changed.value_or([](auto){})(str.toStdString());
      });
    return widget;
  }

  auto withOptions(this auto&& self, const std::vector<std::string>& options)
  {
    self.m_options = options;
    return self;
  }

  auto valueChanged(this auto&& self, std::function<void(std::string)> func)
  {
    self.m_value_changed = func;
    return self;
  }

private:
  std::optional<std::string> m_base_value;
  std::optional<std::vector<std::string>> m_options;
  std::optional<std::function<void(std::string)>> m_value_changed;
};

struct LineSeries : public Widget<LineSeries> {

  struct Axis {
    std::string name;
    std::pair<f64, f64> range;
    std::optional<std::string> format;
  };

  LineSeries() {}
  auto create_widget(QWidget* parent, QBoxLayout* layout) -> QWidget*
  {
    auto* widget = new QtCharts::QChartView();
    auto* chart = new QtCharts::QChart();

    auto series = m_series | std::views::transform([](const std::vector<std::pair<f64, f64>>& vec) {
      return vec | std::views::transform([](std::pair<f64, f64> point) { return QPointF{point.first, point.second}; }) |
      std::ranges::to<QList<QPointF>>();
    }) | std::ranges::to<std::vector<QList<QPointF>>>();

    QPen pen(QColor("#3b82f6")); // TODO: abstract it away
    pen.setWidth(2);

    for (auto serie : series) {
      auto* line_series = new QtCharts::QLineSeries();
      line_series->setUseOpenGL();
      line_series->setPen(pen);
      line_series->append(serie);
      chart->addSeries(line_series);
    }

    if(m_x_axis.has_value()) {
      auto x_axis = new QtCharts::QValueAxis();
      x_axis->setTitleText(QString::fromStdString(m_x_axis->name));
      x_axis->setRange(static_cast<qreal>(m_x_axis->range.first), 
                       static_cast<qreal>(m_x_axis->range.second));
      chart->addAxis(x_axis, Qt::AlignBottom);
      std::cout << "Range is : " << m_x_axis->range.first << " :: " << m_x_axis->range.second << std::endl;
    }

    if(m_y_axis.has_value()) {
      auto y_axis = new QtCharts::QValueAxis();
      y_axis->setTitleText(QString::fromStdString(m_y_axis->name));
      y_axis->setRange(static_cast<qreal>(m_y_axis->range.first), 
                       static_cast<qreal>(m_y_axis->range.second));
      chart->addAxis(y_axis, Qt::AlignLeft);
      std::cout << "Range is : " << m_y_axis->range.first << " :: " << m_y_axis->range.second << std::endl;
    }

    widget->setChart(chart);
    layout->addWidget(widget);
    return widget;
  }

  auto withYAxis(this LineSeries&& self, Axis axis)
  {
    self.m_y_axis = axis;
    return self;
  }

  auto withXAxis(this LineSeries&& self, Axis axis)
  {
    self.m_x_axis = axis;
    return self;
  }

  auto addSeries(this auto&& self, const std::vector<std::pair<f64, f64>>& items)
  {
    self.m_series.emplace_back(items);
    return self;
  }

  auto addSeriesVector(this LineSeries&& self, const std::vector<std::vector<std::pair<f64, f64>>>& items)
  {
    for (auto& serie : items) self.m_series.emplace_back(serie);
    return self;
  }

private:
  std::vector<std::vector<std::pair<f64, f64>>> m_series;
  std::optional<std::string> m_base_value;
  std::optional<Axis> m_x_axis;
  std::optional<Axis> m_y_axis;
};
