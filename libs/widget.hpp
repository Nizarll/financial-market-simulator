#pragma once
#include <concepts>
#include <QBoxLayout>
#include <QPushButton>
#include <QWidget>
#include <QString>
#include <qboxlayout.h>
#include <qwidget.h>
#include <string>
#include <tuple>
#include <type_traits>

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

private:
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
    if (m_spacing) layout->setSpacing(m_spacing);
    if (m_stretched) layout->addStretch();

    ::add_widgets_to(parent, layout, m_widgets);
    parent_layout->addLayout(layout);
    return parent;
  }

  auto fitTo(QWidget* parent)
  {
    auto layout = new QHBoxLayout();
    auto widget = create_widget(parent, layout);
    parent->setLayout(layout);
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
  uint m_spacing{};
  LayoutDirection m_direction;
  bool m_stretched{false};
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

