#pragma once

#include "utils.hpp"
#include <random>
#include <vector>

class StochasticProcess {
protected:
  u64 m_sample_size;
public:
  StochasticProcess() = default;
  StochasticProcess(u64 sample_size) : m_sample_size(sample_size) {}
  virtual ~StochasticProcess() = default;
  virtual f64 operator()() = 0;
  virtual u64 get_sample_size() const { return m_sample_size; }
};

class GeometricBrownianMotion final : public virtual StochasticProcess {
  std::normal_distribution<f64> m_normal;
  std::vector<f64> m_samples;
  f64 m_initial_val;
  f64 m_drift_rate;
  f64 m_volatility;
  f64 m_step;
  f64 m_last_computed;
  std::mt19937 m_gen;
public:
  explicit GeometricBrownianMotion(
    f64 initial_value,
    f64 drift_rate,
    f64 volatility,
    u64 sample_size = 1000,
    f64 step = .1
  );

  virtual f64 operator()() override;

  void set_initial_val(f64 initial_val);
  void set_drift_rate(f64 drift_rate);
  void set_volatility(f64 volatility);
  void set_step(f64 step);
  void set_sample_size(u64 sample_size);

  friend class CubicSplineInterpolatedGBM;
private:
  f64 generate_sample();
};

struct SplineCoefficient {
  f64 a, b, c, d;
};


class CubicSplineInterpolatedGBM final : public virtual StochasticProcess {
  GeometricBrownianMotion m_gbm;
  std::vector<SplineCoefficient> m_coeffs;
  f64 m_step;
  f64 m_current_t;
  f64 m_interpolation_step;
  u64 m_sample_size;
public:
  explicit CubicSplineInterpolatedGBM(
    f64 initial_value,
    f64 drift_rate,
    f64 volatility,
    u64 sample_size = 100,
    f64 step = .1
  );
  virtual f64 operator()() override;
public:
  GeometricBrownianMotion& get_gbm();
  virtual u64 get_sample_size() const override { return m_gbm.m_sample_size; }
private:
  void compute_spline();
};
