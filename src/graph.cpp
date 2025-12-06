#include <QPointer>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <iostream>
#include <qboxlayout.h>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <qchart.h>
#include <qchartview.h>
#include <stdexcept>

#include "graph.hpp"

GraphContainer::GraphContainer(
  std::shared_ptr<StochasticProcess> process,
  QWidget* parent
) : m_st_process(std::move(process)), QWidget(parent) { init(); }

void GraphContainer::init()
{
  using namespace QtCharts;

  if(not m_st_process)
    throw std::runtime_error(
      "Attempted to create a stochastic process graph visualizer"
      " without a stochastic process"
    );

  auto *layout = new QHBoxLayout(this);
  m_series = new QLineSeries();

  std::cout << m_st_process->get_sample_size() << std::endl;
  for (int i = 0; i < m_st_process->get_sample_size(); i++) {
    m_series->append(i, (*m_st_process)());
  }

  auto *chart = new QChart();
  chart->addSeries(m_series);
  chart->createDefaultAxes();
  chart->legend()->hide();

  auto *view = new QChartView(chart);
  view->setRenderHint(QPainter::Antialiasing);
  view->setInteractive(true);
  view->setRubberBand(QChartView::HorizontalRubberBand);
  view->setDragMode(QChartView::NoDrag);
  view->chart()->setAnimationOptions(QChart::SeriesAnimations);
  view->setMinimumSize(400, 250);
  layout->addWidget(view);
}

void GraphContainer::refresh()
{
  m_series->clear();
  for (int i = 0; i < m_st_process->get_sample_size(); i++) m_series->append(i, (*m_st_process)());
}
