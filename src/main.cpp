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
#include <tuple>
#include "header.hpp"
#include "stochastic_process.hpp"

int main(int argc, char **argv) {
  QApplication app(argc, argv);

  CEVContext cev_context {
    .elasticity = 1.1,
    .initial_value = 50,
    .drift_rate = 0.02,
    .volatility = 0.15,
    .step = .08,
  };

  GBMContext context {
    .initial_value = 50,
    .drift_rate = 0.02,
    .volatility = 0.15,
    .step = 1,
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

  auto compute_cev = [&] {
    auto simulations = std::views::iota(0, 4) |
      std::views::transform(generate_cev) |
      std::ranges::to<std::vector>();

    auto all_prices = simulations
      | std::views::join
      | std::views::transform([](const auto& pair){ return pair.second; })
      | std::ranges::to<std::vector<f64>>();

    auto min_price = std::ranges::min(all_prices);
    auto max_price = std::ranges::max(all_prices);

    return std::make_tuple(simulations, min_price, max_price);
  };
 
  auto cev = make_reactive(compute_cev());
  auto simulations_state = make_reactive([](auto cev){
    auto [prices, min, max] = cev;
    return prices;
  }, cev);

  auto x_axis_state = make_reactive(LineSeries::Axis{"Time", {0, 365}});

  auto y_axis_state = make_reactive([](auto cev) {
    auto [_, min, max] = cev;
    return LineSeries::Axis{ "Stock Price", {  min, max } };
  }, cev);


  auto recompute_cev = [&] { cev.set(compute_cev()); };

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
        VLayout(Text( "Initial Price" ), Input ( Input::ValueType::F64).valueChanged([&](f64 val){ context.initial_value = val; recompute_cev(); })),
        VLayout(Text( "Volatiltiy Price" ), Input ( Input::ValueType::F64 ).valueChanged([&](f64 val){ context.volatility = val; recompute_cev(); })),
        VLayout(Text( "Drift Rate" ), Input ( Input::ValueType::F64 ).valueChanged([&](f64 val){ context.drift_rate = val; recompute_cev(); })),
        VLayout(Text( "Step" ), Input ( Input::ValueType::F64 ).valueChanged([&](f64 val){ context.step = val; recompute_cev(); }))
      ).addStretch(),
      LineSeries()
      .addManySeries(simulations_state)
      .withXAxis(x_axis_state).withYAxis(y_axis_state)
    )
  );
  layout.fitTo(&window);
  window.show();
  return app.exec();
}
