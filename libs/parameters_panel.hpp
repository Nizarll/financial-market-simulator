#pragma once

#include "utils.hpp"
#include <QWidget>
#include <QString>
#include <functional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>
#include <optional>


class ParametersPanel : public QWidget
{
  Q_OBJECT;
public:
  struct InputField {
    std::string name;
    std::variant<f64, int, QString> value;

    template <typename T>
    T as()
    {
      if (not std::holds_alternative<T>(value))
        throw std::invalid_argument("variant does not hold type queried");

      return std::get<T>(value);
    }
  };
  ParametersPanel() = default;
  ParametersPanel(std::initializer_list<InputField> fields);
  std::optional<InputField> retrieve_value(const std::string& name);
  void value_changed(const std::string& name, std::function<void(InputField)> listener);
private:
  std::vector<InputField> m_fields;
  std::unordered_map<
    std::string,
    std::vector<std::function<void(InputField)>>
  > m_listeners; // using std::functions is slower because of runtime polymorphism used by std::function it
};
