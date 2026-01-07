#pragma once
#include "utils.hpp"
#include <generator>
#include <random>

struct GBMContext {
  f64 initial_value;
  f64 drift_rate;
  f64 volatility;
  f64 step;
};

static std::generator<f64> gbm_log_returns(GBMContext context) {
  std::mt19937 rng{std::random_device{}()};
  std::normal_distribution<double> normal(0.0, 1.0);
  while (true) {
    double z = normal(rng);
    co_yield (context.drift_rate - 0.5 * context.volatility * context.volatility) * context.step
           + context.volatility * std::sqrt(context.step) * z;
  }
}

static std::generator<f64> gbm_prices(GBMContext context) {
  std::mt19937 rng{std::random_device{}()};
  std::normal_distribution<double> normal(0.0, 1.0);
  f64 current_price = context.initial_value;
 
  f64 i = 0;
  f64 t = 0;
  while (true) {
    co_yield current_price;
    double Wt = std::sqrt(context.step) * normal(rng);
    double log_return = (context.drift_rate - 0.5 * context.volatility * context.volatility) * context.step
                      + context.volatility * Wt;
   current_price = current_price * std::exp(log_return); // use discrete time formula
  }
}
