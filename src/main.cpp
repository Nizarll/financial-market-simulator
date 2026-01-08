#include <algorithm>
#include <iostream>


#include "utils.hpp"
#include "widget.hpp"
#include <QBoxLayout>
#include <QDir>
#include <qapplication.h>
#include <qnamespace.h>
#include <qwidget.h>
#include <ranges>
#include <string>
#include "header.hpp"
#include "stochastic_process.hpp"

int main(int argc, char **argv) {
  QApplication app(argc, argv);

  CEVContext cev_context {
    .elasticity = 1.5,
    .initial_value = 50,
    .drift_rate = 0.02,
    .volatility = 0.15,
    .step = 0.2,
  };

  GBMContext context {
    .initial_value = 50,
    .drift_rate = 0.02,
    .volatility = 0.15,
    .step = 0.2,
  };
  
  auto generate_cev = [&](auto){
    f64 idx = 0;
    return constant_elasticity_of_variance(cev_context) | std::views::take(static_cast<size_t>(365 / context.step))  
    | std::views::transform([idx_counter = 0.0](f64 val) mutable {
      return std::pair<f64, f64>{idx_counter++, val};
    }) 
    | std::ranges::to<std::vector>();
  };

  auto generate_gbm = [&](auto){
    f64 idx = 0;
    return geometric_brownian_motion(context) | std::views::take(static_cast<size_t>(365 / context.step))  
    | std::views::transform([idx_counter = 0.0](f64 val) mutable {
      return std::pair<f64, f64>{idx_counter++, val};
    }) 
    | std::ranges::to<std::vector>();
  };

  QWidget window;

  auto style = read_asset("styles/app.css");
  Theme theme(app, style);

  HeaderProps props {
    .toggle_theme = [&theme](){
      auto state = theme.getState();
      theme.setState(state == Theme::Dark ? Theme::Light : Theme::Dark);
    }
  };

  auto simulations = std::views::iota(0, 2) |
    std::views::transform(generate_cev) |
    std::ranges::to<std::vector>();

  auto all_prices = simulations 
    | std::views::join
    | std::views::transform([](const auto& pair){ return pair.second; })
    | std::ranges::to<std::vector<f64>>();

  for (auto price : all_prices) std::cout << price << std::endl;
  
  auto min_price = std::ranges::min(all_prices);
  auto max_price = std::ranges::max(all_prices);

  std::cout << "Price range: " << min_price << " to " << max_price << std::endl;

  auto layout_two = VLayout(
    Text("Hello world")
  );

  auto layout = VLayout(
    Header(props).addStretch(false),
    VLayout (
      Text ( "Asset Pricing" ),
      ComboBox(
        "Stock Pricing"
      ).withOptions(std::vector<std::string>{
        "Stock Pricing",
        "Option Pricing"
      })
    ),
    HLayout(
      VLayout(
        VLayout(Text( "Initial Price" ), Input ( Input::ValueType::F64).valueChanged( [](f64 val){ (void)val; })),
        VLayout(Text( "Volatiltiy Price" ), Input ( Input::ValueType::F64 )),
        VLayout(Text( "Drift Rate" ), Input ( Input::ValueType::F64 )),
        VLayout(Text( "Step" ), Input ( Input::ValueType::F64 ))
      ).addStretch(),
      LineSeries()
      .addSeriesVector(simulations)
      .withXAxis({
        .name = "Time",
        .range = {0, 365}
      }).withYAxis({
        .name = "Stock Price",
        .range = {
          0, max_price
        }
      })
    )
  );
  layout.fitTo(&window);
  window.show();
  return app.exec();
}
