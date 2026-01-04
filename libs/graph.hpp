#pragma once

#include <QWidget>
#include <QChartView>
#include <QtCharts/QChart>
#include <QLineSeries>
#include <QMouseEvent>
#include <QWheelEvent>
#include <memory>
#include <qevent.h>
#include <qglobal.h>
#include "stochastic_process.hpp"

class GraphContainer : public QtCharts::QChartView
{
  Q_OBJECT;
  QtCharts::QLineSeries* m_series;
public:
  explicit GraphContainer(std::shared_ptr<StochasticProcess> process, QWidget* parent = nullptr);
  void refresh();
protected:
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;
private:
  void init();
  std::shared_ptr<StochasticProcess> m_st_process;
  QPoint m_mouse_pos;
  bool m_is_dragging = false;
  qreal m_last_zoom;
};
