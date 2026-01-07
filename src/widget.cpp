#include "widget.hpp"
#include <qapplication.h>
#include <ranges>


//TODO:  Use System's builtin theme
Theme::Theme(QApplication& app, const std::string& style) : m_app(app)
{
  QString qt_style = QString::fromStdString(style);
  auto get_theme = [&qt_style](QString theme) {
    return qt_style.split("\n") | std::views::filter([theme](const QString& line){
      return line.contains(theme);
    }) | std::views::join_with(QString("\n")) |std::ranges::to<QString>();
  };

  m_light_theme = get_theme("Light");
  m_dark_theme = get_theme("Dark");

  setState(Dark);
}

Theme::ThemeState Theme::getState() const { return m_theme; }

void Theme::setState(Theme::ThemeState state)
{
  m_app.get().setStyleSheet(
    state == Theme::Dark ? m_dark_theme : m_light_theme
  );
  m_theme = state;
}
