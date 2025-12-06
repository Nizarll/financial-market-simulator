#include "stochastic_process.hpp"
#include <cmath>
#include <random>

GeometricBrownianMotion::GeometricBrownianMotion(
  f64 initial_value,
  f64 drift_rate,
  f64 volatility,
  u64 sample_size,
  f64 step
) :
  StochasticProcess(sample_size),
  m_initial_val(initial_value),
  m_drift_rate(drift_rate),
  m_volatility(volatility),
  m_step(step),
  m_samples(sample_size),
  m_normal(.0, std::sqrt(step)),
  m_last_computed(initial_value)
{
  std::random_device rd{"hw"};
  m_gen = std::mt19937(rd());
  for (auto it = m_samples.begin(); it != m_samples.end(); it++)
  {
    *it = generate_sample();
  }
}

f64 GeometricBrownianMotion::generate_sample()
{
  auto brownian_factor = m_normal(m_gen);
  auto exponent = std::exp(
    (m_drift_rate -  .5 * m_volatility * m_volatility) * m_step
    + m_volatility * brownian_factor
  );
  m_last_computed = m_last_computed * exponent;
  return m_last_computed;
}

f64 GeometricBrownianMotion::operator()()
{
  static int index = 0;
  return m_samples.at(index++);
}

void GeometricBrownianMotion::set_initial_val(f64 initial_val) { m_initial_val = initial_val; }
void GeometricBrownianMotion::set_drift_rate(f64 drift_rate){ m_drift_rate = drift_rate; }
void GeometricBrownianMotion::set_volatility(f64 volatility){ m_volatility = volatility; }
void GeometricBrownianMotion::set_sample_size(u64 sample_size){ m_sample_size = sample_size; }
void GeometricBrownianMotion::set_step(f64 step){ m_step = step; }

CubicSplineInterpolatedGBM::CubicSplineInterpolatedGBM(
  f64 initial_value,
  f64 drift_rate,
  f64 volatility,
  u64 samples,
  f64 step
) : m_gbm(initial_value, drift_rate, volatility, samples, step),
    m_step(step),
    m_sample_size(samples),
    m_current_t(0.0),
    m_interpolation_step(step / 10.0)
{
  compute_spline();
}

f64 CubicSplineInterpolatedGBM::operator()() {
  f64 max_t = (m_sample_size - 1) * m_step;
  
  if (m_current_t >= max_t) {
    u64 last = m_sample_size - 2;
    f64 dx = m_step;
    return m_coeffs[last].a + 
           m_coeffs[last].b * dx + 
           m_coeffs[last].c * dx * dx + 
           m_coeffs[last].d * dx * dx * dx;
  }
  
  u64 i = static_cast<u64>(m_current_t / m_step);
  if (i >= m_sample_size - 1) i = m_sample_size - 2;
  
  f64 dx = m_current_t - (i * m_step);
  
  f64 dx2 = dx * dx;
  f64 dx3 = dx2 * dx;
  
  f64 result = m_coeffs[i].a + 
               m_coeffs[i].b * dx + 
               m_coeffs[i].c * dx2 + 
               m_coeffs[i].d * dx3;
  
  m_current_t += m_interpolation_step;

  return result;
}

void CubicSplineInterpolatedGBM::compute_spline() {
  std::vector<f64> y_values;
  y_values.reserve(m_sample_size);
  
  for (u64 i = 0; i < m_sample_size; ++i) {
    y_values.push_back(m_gbm());
  }
  
  u64 n = m_sample_size - 1;
  f64 h = m_step;
  
  std::vector<f64> alpha(n);
  for (u64 i = 1; i < n; ++i) {
    alpha[i] = (3.0 / h) * (y_values[i + 1] - y_values[i]) -
               (3.0 / h) * (y_values[i] - y_values[i - 1]);
  }
  
  std::vector<f64> l(m_sample_size), mu(m_sample_size), z(m_sample_size);
  std::vector<f64> M(m_sample_size); // Second derivatives
  
  l[0] = 1.0;
  mu[0] = 0.0;
  z[0] = 0.0;
  
  for (u64 i = 1; i < n; ++i) {
    l[i] = 2.0 * (2.0 * h) - h * mu[i - 1];  // Simplified: 2*(x[i+1]-x[i-1]) = 4h
    mu[i] = h / l[i];
    z[i] = (alpha[i] - h * z[i - 1]) / l[i];
  }
  
  l[n] = 1.0;
  z[n] = 0.0;
  M[n] = 0.0;
  
  for (i64 j = n - 1; j >= 0; --j) {
    M[j] = z[j] - mu[j] * M[j + 1];
  }
  
  m_coeffs.resize(n);
  for (u64 i = 0; i < n; ++i) {
    m_coeffs[i].a = y_values[i];
    m_coeffs[i].b = (y_values[i + 1] - y_values[i]) / h - 
                    h * (M[i + 1] + 2.0 * M[i]) / 3.0;
    m_coeffs[i].c = M[i];
    m_coeffs[i].d = (M[i + 1] - M[i]) / (3.0 * h);
  }
}

GeometricBrownianMotion& CubicSplineInterpolatedGBM::get_gbm() { return m_gbm; }
