#pragma once

#include <QWidget>
#include "stochastic_process.hpp"
#include <memory>

#include <QtCharts/QChart>
#include <qlineseries.h>

class GraphContainer : public QWidget
{
  Q_OBJECT;

  QtCharts::QLineSeries* m_series;
public:
  explicit GraphContainer(std::shared_ptr<StochasticProcess> process, QWidget* parent = nullptr);
  
  void refresh();
private:
  void init();
  std::shared_ptr<StochasticProcess> m_st_process;
};
