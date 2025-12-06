#include <iostream>
#include <memory>
#include <qapplication.h>
#include <qboxlayout.h>
#include <qevent.h>
#include <QDir>
#include <QPushButton>
#include <qobject.h>
#include <qpushbutton.h>
#include <string>
#include "header.hpp"
#include "stochastic_process.hpp"
#include "graph.hpp"
#include "shadcn_theme.hpp"
#include "parameters_panel.hpp"

int main(int argc, char **argv) {
  QApplication app(argc, argv);

  auto parameters_panel = ParametersPanel{
    {"Stock Initial Price", f64(10) },
    {"Drift Rate", f64(0.05) },
    {"Volatility", f64(0.2) },
    {"Sample Size", int(500) },
    {"Step", f64(.1) }
  };

  auto gbm = std::make_shared<CubicSplineInterpolatedGBM>(
    parameters_panel.retrieve_value("Stock Initial Price")->as<f64>(),
    parameters_panel.retrieve_value("Drift Rate")->as<f64>(),
    parameters_panel.retrieve_value("Volatility")->as<f64>(),
    static_cast<u64>(parameters_panel.retrieve_value("Sample Size")->as<int>()),
    parameters_panel.retrieve_value("Step")->as<f64>()
  );

  parameters_panel.value_changed("Stock Initial Price", [gbm](ParametersPanel::InputField field){ gbm->get_gbm().set_initial_val(field.as<f64>()); });
  parameters_panel.value_changed("Drift Rate",[gbm](ParametersPanel::InputField field){ gbm->get_gbm().set_drift_rate(field.as<f64>()); });
  parameters_panel.value_changed("Volatility",[gbm](ParametersPanel::InputField field){ gbm->get_gbm().set_volatility(field.as<f64>()); });
  //parameters_panel.value_changed("Step", [gbm](ParametersPanel::InputField field){ gbm->get_gbm().set_step(static_cast<u64>(field.as<int>())); });
  //parameters_panel.value_changed("Sample Size",[gbm](parameters_panel::InputField field){ gbm->get_gbm().set_sample_size(field.as<f64>()); });

  auto graph_container = GraphContainer(gbm);
  std::vector<std::string> parameters{"Stock Initial Price", "Drift Rate", "Volatility", "Sample Size", "Step"};

  for (const auto& parameter: parameters)
    parameters_panel.value_changed(parameter, [&graph_container](ParametersPanel::InputField field){ graph_container.refresh(); });

  QWidget window;
  Header header;

  auto* widget = new QWidget();

  auto layout = new QHBoxLayout();
  layout->addWidget(&parameters_panel, 2);
  layout->addWidget(&graph_container, 8);
  widget->setLayout(layout);

  auto win_layout = new QVBoxLayout();
  win_layout->addWidget(&header, 1);
  win_layout->addWidget(widget, 9);
  window.setLayout(win_layout);

  QString stylesPath = QDir::currentPath() + "/assets/styles";
  ShadcnTheme theme(stylesPath);

  theme.installOnWindow(&window, ShadcnTheme::Dark, &app);
  header.theme_toggled([&theme, &window, &app](bool checked) {
    theme.setTheme(checked ? ShadcnTheme::Dark : ShadcnTheme::Light, &app);
  });

  window.show();
  return app.exec();
}
